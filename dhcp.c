/**
 ******************************************************************************
 * @file    dhcp.c
 * @author  Aditya Mall,
 * @brief   DHCP protocol source file
 *
 *  Info
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2019 Aditya Mall, MIT License </center></h2>
 *
 * MIT License
 *
 * Copyright (c) 2019 Aditya Mall
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */




/*
 * Standard header and api header files
 */

#include <string.h>

#include "udp.h"
#include "network_utilities.h"
#include "dhcp.h"





/******************************************************************************/
/*                                                                            */
/*                              DHCP Functions                                */
/*                                                                            */
/******************************************************************************/




/***************************************************************
 * @brief   Function Send DHCP Discover
 * @param   *ethernet       : reference to the Ethernet handle
 * @param   transaction_id  : random transaction ID
 * @param   seconds_elapsed : number of seconds elapsed
 * @retval  uint8_t         : Error = -1, Success = 0
 ***************************************************************/
int8_t ether_dhcp_send_discover(ethernet_handle_t *ethernet, uint32_t transaction_id, uint16_t seconds_elapsed)
{

    int8_t func_retval = 0;

    net_dhcp_t           *dhcp_discover;
    dhcp_discover_opts_t *discover_opts;
    ether_source_t        dhcp_client;

    uint8_t destination_ip[4]  = {0};
    uint8_t destination_mac[6] = {0};

    char data[260] = {0};

    if(ethernet == NULL)
    {
        func_retval = -1;
    }
    else
    {
        /* Configure DHCP fields */
        dhcp_discover = (net_dhcp_t*)data;

        dhcp_discover->op_code        = DHCP_BOOT_REQ;
        dhcp_discover->hw_type        = 1;
        dhcp_discover->hw_length      = ETHER_MAC_SIZE;
        dhcp_discover->hops           = 0;
        dhcp_discover->transaction_id = transaction_id;
        dhcp_discover->seconds        = htons(seconds_elapsed);
        dhcp_discover->flags          = htons(0x8000);

        /* client  IP address  = 0  */
        /* your    IP address  = 0  */
        /* server  IP address  = 0  */
        /* gateway IP address  = 0  */

        /* client hardware address */
        strncpy((char*)dhcp_discover->client_hw_addr, (char*)ethernet->host_mac, ETHER_MAC_SIZE);

        /* Client hardware address padding = 0 */
        /* Client Server host name         = 0 */
        /* Client file name                = 0 */

        /* Configure magic cookie value */
        dhcp_discover->magic_cookie[0] = 0x63;
        dhcp_discover->magic_cookie[1] = 0x82;
        dhcp_discover->magic_cookie[2] = 0x53;
        dhcp_discover->magic_cookie[3] = 0x63;


        /* Configure DHCP options */
        discover_opts = (void*)&dhcp_discover->options;

        /* option (53) */
        discover_opts->message_type.option_number = DHCP_MESSAGE_TYPE;
        discover_opts->message_type.length        = 1;
        discover_opts->message_type.dhcp          = DHCP_DISCOVER;

        /* option (55) */
        discover_opts->param_request_list.option_number = DHCP_PARAM_REQ_LIST;
        discover_opts->param_request_list.length        = 3;
        discover_opts->param_request_list.req_item[0]   = DHCP_SUBNET_MASK;
        discover_opts->param_request_list.req_item[1]   = DHCP_ROUTER;
        discover_opts->param_request_list.req_item[2]   = DHCP_ADDR_LEASE_TIME;

        /* option (61) */
        discover_opts->client_identifier.option_number = DHCP_CLIENT_IDENTIFIER;
        discover_opts->client_identifier.length        = 7;
        discover_opts->client_identifier.hw_type       = 1;

        strncpy((char*)discover_opts->client_identifier.client_mac, (char*)ethernet->host_mac, ETHER_MAC_SIZE);

        /* option end */
        discover_opts->options_end = DHCP_OPTION_END;


        /* Configure sources */
        memset(&dhcp_client, 0, sizeof(ether_source_t));

        dhcp_client.identifier  = 1;
        dhcp_client.source_port = DHCP_SOURCE_PORT;

        strncpy((char*)dhcp_client.source_mac, (char*)ethernet->host_mac, ETHER_MAC_SIZE);

        /* Configure destination address */
        strncpy((char*)destination_ip, (char*)ethernet->broadcast_ip, ETHER_IPV4_SIZE);
        strncpy((char*)destination_mac, (char*)ethernet->broadcast_mac, ETHER_MAC_SIZE);


        /* Send DHCP packet as UPD message */
        ether_send_udp_raw(ethernet, &dhcp_client, destination_ip, destination_mac, DHCP_DESTINATION_PORT, (uint8_t*)dhcp_discover, (DHCP_FRAME_SIZE + 18));
    }

    return func_retval;
}





/************************************************************
 * @brief   Function read DHCP offer
 * @param   *ethernet     : reference to the Ethernet handle
 * @param   *network_data : network_data from PHY
 * @param   *your_ip      : 'your IP' address
 * @param   *server_ip    : server IP address
 * @param   *subnet_mask  : SUBNET mask
 *
 * @retval  uint8_t       : Error = 0, Success = 1
 ************************************************************/
int8_t ether_dhcp_read_offer(ethernet_handle_t *ethernet, uint8_t *network_data, uint8_t *your_ip, uint8_t *server_ip,
                             uint8_t *subnet_mask, uint8_t *lease_time)
{
    int8_t func_retval = 0;
    uint8_t api_retval = 0;

    net_dhcp_t *dhcp_offer;
    dhcp_offer_opts_t *offer_opts;

    uint16_t udp_src_port  = 0;
    uint16_t udp_dest_port = 0;

    char    dhcp_data[300] = {0};

    if(ethernet == NULL || network_data == NULL)
    {
        func_retval = 0;
    }
    else
    {
        /* read UDP packet */
        api_retval = ether_read_udp_raw(ethernet, network_data, ETHER_MTU_SIZE, &udp_src_port, &udp_dest_port, dhcp_data, 300);


        /* Check if UDP source ports = DHCP destination port */
        if(udp_src_port == DHCP_DESTINATION_PORT && udp_dest_port == DHCP_SOURCE_PORT && api_retval)
        {
            dhcp_offer = (void*)dhcp_data;

            /* Get your_ip from DHCP standard header*/
            strncpy((char*)your_ip, (char*)dhcp_offer->your_ip, ETHER_IPV4_SIZE);


            /* Get server_ip, SUBNET mask, lease time from DHCP offer options */
            offer_opts = (void*)&dhcp_offer->options;

            memcpy((char*)server_ip, (char*)offer_opts->server_identifier.server_ip, ETHER_IPV4_SIZE);

            memcpy((char*)lease_time, (char*)offer_opts->lease_time.lease_time, ETHER_IPV4_SIZE);

            memcpy((char*)subnet_mask, (char*)offer_opts->subnet_mask.subnet_mask, ETHER_IPV4_SIZE);


            func_retval = 1;
        }

    }

    return func_retval;
}







/***************************************************************
 * @brief   Function Send DHCP Request
 * @param   *ethernet       : reference to the Ethernet handle
 * @param   transaction_id  : random transaction ID
 * @param   seconds_elapsed : number of seconds elapsed
 *
 *
 *
 * @retval  uint8_t         : Error = -1, Success = 0
 ***************************************************************/
int8_t ether_dhcp_send_request(ethernet_handle_t *ethernet, uint32_t transaction_id, uint16_t seconds_elapsed,
                               uint8_t *server_ip, uint8_t *requested_ip, uint8_t *lease_time)
{

    int8_t func_retval = 0;

    net_dhcp_t          *dhcp_request;
    dhcp_request_opts_t *request_opts;
    ether_source_t        dhcp_client;

    uint8_t destination_ip[4]  = {0};
    uint8_t destination_mac[6] = {0};

    char data[280] = {0};

    if(ethernet == NULL)
    {
        func_retval = -1;
    }
    else
    {
        /* Configure DHCP fields */
        dhcp_request = (net_dhcp_t*)data;

        dhcp_request->op_code        = DHCP_BOOT_REQ;
        dhcp_request->hw_type        = 1;
        dhcp_request->hw_length      = ETHER_MAC_SIZE;
        dhcp_request->hops           = 0;
        dhcp_request->transaction_id = transaction_id;
        dhcp_request->seconds        = htons(seconds_elapsed);
        dhcp_request->flags          = htons(0x8000);

        /* client  IP address  = 0  */
        /* your    IP address  = 0  */
        /* server  IP address  = 0  */
        /* gateway IP address  = 0  */

        /* client hardware address */
        strncpy((char*)dhcp_request->client_hw_addr, (char*)ethernet->host_mac, ETHER_MAC_SIZE);

        /* Client hardware address padding = 0 */
        /* Client Server host name         = 0 */
        /* Client file name                = 0 */

        /* Configure magic cookie value */
        dhcp_request->magic_cookie[0] = 0x63;
        dhcp_request->magic_cookie[1] = 0x82;
        dhcp_request->magic_cookie[2] = 0x53;
        dhcp_request->magic_cookie[3] = 0x63;


        /* Configure DHCP options */
        request_opts = (void*)&dhcp_request->options;

        /* option (53) */
        request_opts->message_type.option_number = DHCP_MESSAGE_TYPE;
        request_opts->message_type.length        = 1;
        request_opts->message_type.dhcp          = DHCP_REQUEST;

        /* option (55) */
        request_opts->param_request_list.option_number = DHCP_PARAM_REQ_LIST;
        request_opts->param_request_list.length        = 3;
        request_opts->param_request_list.req_item[0]   = DHCP_SUBNET_MASK;
        request_opts->param_request_list.req_item[1]   = DHCP_ROUTER;
        request_opts->param_request_list.req_item[2]   = DHCP_ADDR_LEASE_TIME;

        /* option (61) */
        request_opts->client_identifier.option_number = DHCP_CLIENT_IDENTIFIER;
        request_opts->client_identifier.length        = 7;
        request_opts->client_identifier.hw_type       = 1;
        strncpy((char*)request_opts->client_identifier.client_mac, (char*)ethernet->host_mac, ETHER_MAC_SIZE);


        /* options (50) */
        request_opts->requested_ip.option_number = DHCP_REQUESTED_IP;
        request_opts->requested_ip.length        = 4;
        strncpy((char*)request_opts->requested_ip.requested_ip, (char*)requested_ip, ETHER_IPV4_SIZE);


        /* options (54) */
        request_opts->server_identifier.option_number = DHCP_SERVER_IDENTIFIER;
        request_opts->server_identifier.length        = 4;
        strncpy((char*)request_opts->server_identifier.server_ip, (char*)server_ip, ETHER_IPV4_SIZE);


        /* options (51) */
        request_opts->lease_time.option_number = DHCP_ADDR_LEASE_TIME;
        request_opts->lease_time.length        = 4;
        memcpy((char*)request_opts->lease_time.lease_time, (char*)lease_time, 4);


        /* option end */
        request_opts->options_end = DHCP_OPTION_END;


        /* Configure sources */
        memset(&dhcp_client, 0, sizeof(ether_source_t));

        dhcp_client.identifier  = 1;
        dhcp_client.source_port = DHCP_SOURCE_PORT;

        strncpy((char*)dhcp_client.source_mac, (char*)ethernet->host_mac, ETHER_MAC_SIZE);

        /* Configure destination address */
        strncpy((char*)destination_ip, (char*)ethernet->broadcast_ip, ETHER_IPV4_SIZE);
        strncpy((char*)destination_mac, (char*)ethernet->broadcast_mac, ETHER_MAC_SIZE);


        /* Send DHCP packet as UPD message */
        ether_send_udp_raw(ethernet, &dhcp_client, destination_ip, destination_mac, DHCP_DESTINATION_PORT, (uint8_t*)dhcp_request, (DHCP_FRAME_SIZE + 36));
    }

    return func_retval;
}

