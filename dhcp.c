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






int8_t ether_dhcp_send_discover(ethernet_handle_t *ethernet, uint32_t transaction_id, uint16_t seconds_elapsed)
{

    char data[260] = {0};

    net_dhcp_t           *dhcp_discover;
    dhcp_discover_opts_t *discover_opts;
    ether_source_t        dhcp_client;

    uint8_t destination_ip[4]  = {0};
    uint8_t destination_mac[6] = {0};

    /* Configure DHCP fields */
    dhcp_discover = (net_dhcp_t*)data;

    dhcp_discover->op_code        = DHCP_BOOT_REQ;
    dhcp_discover->hw_type        = 1;
    dhcp_discover->hw_length      = ETHER_MAC_SIZE;
    dhcp_discover->hops           = 0;
    dhcp_discover->transaction_id = transaction_id;
    dhcp_discover->seconds        = seconds_elapsed;
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
    discover_opts->message_type.dhcp          = 1;

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
    ether_send_udp_raw(ethernet, &dhcp_client, destination_ip, destination_mac, DHCP_DESTINATION_PORT, (uint8_t*)dhcp_discover, 258);


    return 0;
}











