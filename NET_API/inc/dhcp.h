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




/* DHCP states */
typedef enum _dhcp_state_values
{
    DHCP_INIT_STATE       = 1,
    DHCP_SELECTING_STATE  = 2,
    DHCP_READ_STATE       = 3,
    DHCP_REQUESTING_STATE = 4,
    DHCP_ACK_STATE        = 5,
    DHCP_BOUND_STATE      = 6,

}dhcp_states;




/******************************************************************************/
/*                                                                            */
/*                      DHCP Functions Prototypes                             */
/*                                                                            */
/******************************************************************************/



/**************************************************************
 * @brief   Function Send DHCP Discover
 * @param   *ethernet       : reference to the Ethernet handle
 * @param   transaction_id  : random transaction ID
 *
 * @param   seconds_elapsed : number of seconds elapsed
 * @retval  uint8_t         : Error = -1, Success = 0
 **************************************************************/
int8_t ether_dhcp_send_discover(ethernet_handle_t *ethernet, uint32_t transaction_id, uint8_t *mac_address,
                                uint16_t seconds_elapsed);




/************************************************************
 * @brief   Function read DHCP offer
 * @param   *ethernet     : reference to the Ethernet handle
 * @param   *network_data : network_data from PHY
 * @param   *your_ip      : 'your IP' address
 *
 * @param   *dhcp_options : DHCP options data
 * @retval  uint8_t       : Error = 0, Success = DHCP type
 ************************************************************/
int8_t ether_dhcp_read(ethernet_handle_t *ethernet, uint8_t *network_data, uint8_t *your_ip, uint32_t client_transac_id, uint8_t *dhcp_options);




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
                             uint8_t *subnet_mask, uint8_t *lease_time);




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
                               uint8_t *server_ip, uint8_t *requested_ip, uint32_t lease_time);




/*************************************************************
 * @brief   Function to get IP though DHCP state machine
 * @param   *ethernet     : reference to the Ethernet handle
 * @param   *network_data : network data from PHY
 *
 * @param   dhcp_state    : DHCP state machine states
 * @retval  uint8_t       : Error = NA, Success = NA
 *************************************************************/
int8_t ether_get_dhcp_ip(ethernet_handle_t *ethernet, uint8_t *network_data, uint8_t *mac_address,
                         dhcp_states dhcp_state);




#endif /* DHCP_H_ */
