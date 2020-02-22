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
#include <stdlib.h>

#include "udp.h"
#include "network_utilities.h"
#include "dhcp.h"
#include "arp.h"



/******************************************************************************/
/*                                                                            */
/*                      Data Structures and Defines                           */
/*                                                                            */
/******************************************************************************/


#pragma pack(1)

#define DHCP_FRAME_SIZE         240
#define DHCP_DISCOVER_OPTS_SIZE 18
#define DHCP_REQUEST_OPTS_SIZE  36



/* DHCP UDP port */
typedef enum _network_ports
{
    DHCP_DESTINATION_PORT = 67,
    DHCP_SOURCE_PORT      = 68,

}network_ports_t;



/* DHCP FRAME (240 bytes), options variable data */
typedef struct _net_dhcp
{
    uint8_t  op_code;                 /*!< Operation code or message type */
    uint8_t  hw_type;                 /*!< Hardware type                  */
    uint8_t  hw_length;               /*!< Hardware length                */
    uint8_t  hops;                    /*!< Number of Hops                 */
    uint32_t transaction_id;          /*!< */
    uint16_t seconds;                 /*!< */
    uint16_t flags;                   /*!< */
    uint8_t  client_ip[4];            /*!< */
    uint8_t  your_ip[4];              /*!< */
    uint8_t  server_ip[4];            /*!< */
    uint8_t  gateway_ip[4];           /*!< */
    uint8_t  client_hw_addr[6];       /*!< */
    uint8_t  client_hw_addr_pad[10];  /*!< */
    uint8_t  server_name[64];         /*!< */
    uint8_t  boot_filename[128];      /*!< */
    uint8_t  magic_cookie[4];         /*!< */
    uint8_t  options;

}net_dhcp_t ;




/************* DHCP Discover options structures **************/

/* DHCP Option message type */
typedef struct _opts_53
{
    uint8_t option_number;
    uint8_t length;
    uint8_t dhcp;

}dhcp_option_53_t;


/* DHCP Option parameter request list  */
typedef struct _opts_55
{
    uint8_t option_number;
    uint8_t length;
    uint8_t req_item[3];

}dhcp_option_55_t;


/* DHCP Option client identifier */
typedef struct _opts_61
{
    uint8_t option_number;
    uint8_t length;
    uint8_t hw_type;
    uint8_t client_mac[ETHER_MAC_SIZE];

}dhcp_option_61_t;



/* DHCP Discover options (18 bytes) */
typedef struct _dhcp_discover_options
{
    dhcp_option_53_t message_type;
    dhcp_option_55_t param_request_list;
    dhcp_option_61_t client_identifier;
    uint8_t          options_end;

}dhcp_discover_opts_t;



/************* DHCP Offer options structures **************/

/* */
typedef struct _opts_54
{
    uint8_t option_number;
    uint8_t length;
    uint8_t server_ip[ETHER_IPV4_SIZE];

}dhcp_option_54_t;

/* */
typedef struct _opts_51
{
    uint8_t option_number;
    uint8_t length;
    uint32_t lease_time;

}dhcp_option_51_t;

/* */
typedef struct _opts_1
{
    uint8_t option_number;
    uint8_t length;
    uint8_t subnet_mask[ETHER_IPV4_SIZE];

}dhcp_option_1_t;

/* */
typedef struct _opts_3
{
    uint8_t option_number;
    uint8_t length;
    uint8_t router[ETHER_IPV4_SIZE];

}dhcp_option_3_t;



/* DHCP Offer options */
typedef struct _dhcp_offer_options
{
    dhcp_option_53_t message_type;
    dhcp_option_54_t server_identifier;
    dhcp_option_51_t lease_time;
    dhcp_option_1_t  subnet_mask;
    dhcp_option_3_t  router;
    uint8_t          options_end;

}dhcp_offer_opts_t;


/************* DHCP Offer Request structures **************/


/* */
typedef struct _opts_50
{
    uint8_t option_number;
    uint8_t length;
    uint8_t requested_ip[ETHER_IPV4_SIZE];

}dhcp_option_50_t;



/* DHCP Request options */
typedef struct _dhcp_request_options
{
    dhcp_option_51_t lease_time;
    dhcp_option_53_t message_type;
    dhcp_option_55_t param_request_list;
    dhcp_option_61_t client_identifier;
    dhcp_option_50_t requested_ip;
    dhcp_option_54_t server_identifier;
    uint8_t          options_end;

}dhcp_request_opts_t;



/* */
typedef enum _dhcp_boot_message
{
    DHCP_BOOT_REQ   = 1,
    DHCP_BOOT_REPLY = 2,
    DHCP_DISCOVER   = 1,
    DHCP_OFFER      = 2,
    DHCP_REQUEST    = 3,
    DHCP_ACK        = 5,

}dhcp_boot_msg_t;



/* */
typedef enum _dhcp_option_types
{
    DHCP_SUBNET_MASK       = 1,
    DHCP_ROUTER            = 3,
    DHCP_REQUESTED_IP      = 50,
    DHCP_ADDR_LEASE_TIME   = 51,
    DHCP_MESSAGE_TYPE      = 53,
    DHCP_SERVER_IDENTIFIER = 54,
    DHCP_PARAM_REQ_LIST    = 55,
    DHCP_CLIENT_IDENTIFIER = 61,
    DHCP_OPTION_END        = 255,

}dhcp_options_types_t;



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

    char data[258] = {0};

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
        dhcp_discover->transaction_id = htonl(transaction_id);
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
        ether_send_udp_raw(ethernet, &dhcp_client, destination_ip, destination_mac, DHCP_DESTINATION_PORT,
                           (uint8_t*)dhcp_discover, (DHCP_FRAME_SIZE + DHCP_DISCOVER_OPTS_SIZE));
    }

    return func_retval;
}




/************************************************************
 * @brief   Function read DHCP offer
 * @param   *ethernet     : reference to the Ethernet handle
 * @param   *network_data : network_data from PHY
 * @param   *your_ip      : 'your IP' address
 *
 * @param   *dhcp_options : DHCP options data
 * @retval  uint8_t       : Error = 0, Success = DHCP type
 ************************************************************/
int8_t ether_dhcp_read(ethernet_handle_t *ethernet, uint8_t *network_data, uint8_t *your_ip, uint32_t client_transac_id, uint8_t *dhcp_options)
{
    int8_t   func_retval        = 0;
    uint16_t udp_message_length = 0;

    net_dhcp_t *dhcp_reply;
    dhcp_offer_opts_t *dhcp_opts;

    uint16_t udp_src_port  = 0;
    uint16_t udp_dest_port = 0;

    char dhcp_data[APP_BUFF_SIZE] = {0};

    if(ethernet == NULL || network_data == NULL)
    {
        func_retval = 0;
    }
    else
    {
        /* read UDP packet */
        udp_message_length = ether_read_udp_raw(ethernet, network_data, ETHER_MTU_SIZE, &udp_src_port, &udp_dest_port, dhcp_data, APP_BUFF_SIZE);


        /* Check if UDP source ports = DHCP destination port */
        if(udp_src_port == DHCP_DESTINATION_PORT && udp_dest_port == DHCP_SOURCE_PORT && udp_message_length)
        {
            dhcp_reply = (void*)dhcp_data;

            if(client_transac_id == ntohl(dhcp_reply->transaction_id))
            {

                /* Get your_ip from DHCP standard header*/
                strncpy((char*)your_ip, (char*)dhcp_reply->your_ip, ETHER_IPV4_SIZE);

                /* Get server_ip, SUBNET mask, lease time from DHCP offer options */
                dhcp_opts = (void*)&dhcp_reply->options;

                func_retval = dhcp_opts->message_type.dhcp;

                memcpy(dhcp_options, (uint8_t*)dhcp_opts, (udp_message_length - DHCP_FRAME_SIZE));
            }

        }

    }

    return func_retval;
}






/************************************************************
 * @brief   Function read DHCP offer (Depreciated)
 * @param   *ethernet     : reference to the Ethernet handle
 * @param   *network_data : network_data from PHY
 * @param   *your_ip      : 'your IP' address
 * @param   *server_ip    : server IP address
 * @param   *subnet_mask  : SUBNET mask
 * @param   *lease_time   : IP address lease time
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
        api_retval = ether_read_udp_raw(ethernet, network_data, 350, &udp_src_port, &udp_dest_port, dhcp_data, 300);


        /* Check if UDP source ports = DHCP destination port */
        if(udp_src_port == DHCP_DESTINATION_PORT && udp_dest_port == DHCP_SOURCE_PORT && api_retval)
        {
            dhcp_offer = (void*)dhcp_data;

            /* Get your_ip from DHCP standard header*/
            memcpy((char*)your_ip, (char*)dhcp_offer->your_ip, ETHER_IPV4_SIZE);


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





/*************************************************************
 * @brief   Function Send DHCP Request
 * @param   *ethernet       : reference to the Ethernet handle
 * @param   transaction_id  : random transaction ID
 * @param   seconds_elapsed : number of seconds elapsed
 * @param   *server_ip      : DHCP server IP
 * @param   *requested_ip   : IP request to the server
 * @param   *lease_time     : Lease time
 * @retval  uint8_t         : Error = -1, Success = 0
 *************************************************************/
int8_t ether_dhcp_send_request(ethernet_handle_t *ethernet, uint32_t transaction_id, uint16_t seconds_elapsed,
                               uint8_t *server_ip, uint8_t *requested_ip, uint32_t lease_time)
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
        dhcp_request->transaction_id = htonl(transaction_id);
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


        /* options (51) */
        request_opts->lease_time.option_number = DHCP_ADDR_LEASE_TIME;
        request_opts->lease_time.length        = 4;
        //memcpy((char*)request_opts->lease_time.lease_time, (char*)lease_time, 4);
        request_opts->lease_time.lease_time  = htonl(lease_time);


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
        memcpy((char*)request_opts->client_identifier.client_mac, (char*)ethernet->host_mac, ETHER_MAC_SIZE);


        /* options (50) */
        request_opts->requested_ip.option_number = DHCP_REQUESTED_IP;
        request_opts->requested_ip.length        = 4;
        memcpy((char*)request_opts->requested_ip.requested_ip, (char*)requested_ip, ETHER_IPV4_SIZE);


        /* options (54) */
        request_opts->server_identifier.option_number = DHCP_SERVER_IDENTIFIER;
        request_opts->server_identifier.length        = 4;
        memcpy((char*)request_opts->server_identifier.server_ip, (char*)server_ip, ETHER_IPV4_SIZE);


        /* option end */
        request_opts->options_end = DHCP_OPTION_END;


        /* Configure sources */
        memset(&dhcp_client, 0, sizeof(ether_source_t));

        dhcp_client.identifier  = 1;
        dhcp_client.source_port = DHCP_SOURCE_PORT;

        memcpy((char*)dhcp_client.source_mac, (char*)ethernet->host_mac, ETHER_MAC_SIZE);

        /* Configure destination address */
        memcpy((char*)destination_ip, (char*)ethernet->broadcast_ip, ETHER_IPV4_SIZE);
        memcpy((char*)destination_mac, (char*)ethernet->broadcast_mac, ETHER_MAC_SIZE);


        /* Send DHCP packet as UPD message */
        ether_send_udp_raw(ethernet, &dhcp_client, destination_ip, destination_mac, DHCP_DESTINATION_PORT,
                           (uint8_t*)dhcp_request, (DHCP_FRAME_SIZE + DHCP_REQUEST_OPTS_SIZE));
    }

    return func_retval;
}





/*************************************************************
 * @brief   Function Enable DHCP mode
 * @param   *ethernet     : reference to the Ethernet handle
 * @param   *network_data : network data from PHY
 * @param   dhcp_state    : DHCP state machine states
 * @retval  uint8_t       : Error = NA, Success = NA
 *************************************************************/
int8_t ether_dhcp_enable(ethernet_handle_t *ethernet, uint8_t *network_data, dhcp_states dhcp_state)
{

    dhcp_offer_opts_t *offer_options;

    uint8_t your_ip[4]       = {0};
    uint8_t server_ip[4]     = {0};
    uint8_t subnet_mask[4]   = {0};
    uint8_t dhcp_options[60] = {0};

    int8_t   dhcp_type         = 0;
    uint8_t  dhcp_request_flag = 0;
    uint8_t  dhcp_loop         = 1;

    uint32_t dhcp_transac_id   = get_random_port_l(ethernet, 65535);

    dhcp_state = DHCP_INIT_STATE;

    uint32_t lease_time = 0;

    while(dhcp_loop)
    {
        switch(dhcp_state)
        {

        case DHCP_INIT_STATE:

            ethernet->status.mode_dhcp_init = 1;

            dhcp_state = DHCP_SELECTING_STATE;

            break;


        case DHCP_SELECTING_STATE:

            ether_dhcp_send_discover(ethernet, dhcp_transac_id, 0);

            dhcp_state = DHCP_READ_STATE;

            break;


        case DHCP_READ_STATE:

            dhcp_type = ether_dhcp_read(ethernet, (uint8_t*)network_data, your_ip, dhcp_transac_id, dhcp_options);

            /* Handle DHCP offer */
            if(dhcp_type == DHCP_OFFER && ethernet->status.mode_dhcp_init == 1)
            {
                /* Get server_ip, SUBNET mask, lease time from DHCP offer options */
                offer_options = (void*)dhcp_options;

                memcpy(server_ip, offer_options->server_identifier.server_ip, ETHER_IPV4_SIZE);
                memcpy(subnet_mask, offer_options->subnet_mask.subnet_mask, ETHER_IPV4_SIZE);

                lease_time = ntohl(offer_options->lease_time.lease_time);

                dhcp_state = DHCP_REQUESTING_STATE;
                break;

            }
            /* Handle DHCP ACK */
            else if(dhcp_type == DHCP_ACK &&  dhcp_request_flag == 1)
            {
                dhcp_state = DHCP_ACK_STATE;
                break;
            }
            /* Handle DHCP offer or request not received conditions */
            else if(dhcp_request_flag == 1)
            {
                dhcp_state = DHCP_REQUESTING_STATE;
                break;
            }
            else if(ethernet->status.mode_dhcp_init == 1)
            {
                dhcp_state = DHCP_SELECTING_STATE;
            }


            break;


        case DHCP_REQUESTING_STATE:

            ether_dhcp_send_request(ethernet, dhcp_transac_id, 1, server_ip, your_ip, lease_time);

            dhcp_request_flag = 1;

            dhcp_state = DHCP_READ_STATE;

            break;


        case DHCP_ACK_STATE:

            dhcp_request_flag = 0;

            memcpy((char*)ethernet->host_ip, (char*)your_ip, 4);

            memcpy((char*)ethernet->gateway_ip, (char*)server_ip, 4);

            memcpy((char*)ethernet->subnet_mask, (char*)subnet_mask, 4);

            ethernet->lease_time = lease_time;

            dhcp_state = DHCP_BOUND_STATE;

            break;


        case DHCP_BOUND_STATE:

            ethernet->status.mode_dhcp_init  = 0;
            ethernet->status.mode_dhcp_bound = 1;
            ethernet->status.mode_dynamic    = 1;

            dhcp_loop =  0;

            ether_send_arp_req(ethernet, ethernet->host_ip, ethernet->gateway_ip);

            if(ether_is_arp(ethernet, (uint8_t*)network_data, 128))
            {

                ether_handle_arp_resp_req(ethernet);

            }


            break;


        default:

            dhcp_loop =  0;

            break;

        }

    }


    return 0;
}




