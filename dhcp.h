/**
 ******************************************************************************
 * @file    dhcp.h
 * @author  Aditya Mall,
 * @brief   DHCP protocol header file
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

#ifndef DHCP_H_
#define DHCP_H_



/*
 * Standard header and api header files
 */
#include <stdint.h>

#include "ethernet.h"



/******************************************************************************/
/*                                                                            */
/*                      Data Structures and Defines                           */
/*                                                                            */
/******************************************************************************/


#define DHCP_FRAME_SIZE 240

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
    uint8_t lease_time[4];

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
    dhcp_option_53_t message_type;
    dhcp_option_55_t param_request_list;
    dhcp_option_61_t client_identifier;
    dhcp_option_50_t requested_ip;
    dhcp_option_54_t server_identifier;
    dhcp_option_51_t lease_time;

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
/*                      DHCP Functions Prototypes                             */
/*                                                                            */
/******************************************************************************/



/***************************************************************
 * @brief   Function Send DHCP Discover
 * @param   *ethernet       : reference to the Ethernet handle
 * @param   transaction_id  : random transaction ID
 * @param   seconds_elapsed : number of seconds elapsed
 * @retval  uint8_t         : Error = -1, Success = 0
 ***************************************************************/
int8_t ether_dhcp_send_discover(ethernet_handle_t *ethernet, uint32_t transaction_id, uint16_t seconds_elapsed);




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
                             uint8_t *subnet_mask, uint8_t *lease_time);




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
                               uint8_t *server_ip, uint8_t *requested_ip, uint8_t *lease_time);



#endif /* DHCP_H_ */
