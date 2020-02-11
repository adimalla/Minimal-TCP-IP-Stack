/**
 ******************************************************************************
 * @file    arp.h
 * @author  Aditya Mall,
 * @brief   arp protocol header file
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

#ifndef ARP_H_
#define ARP_H_


/*
 * Standard header and api header files
 */
#include <stdint.h>

#include "ethernet.h"



/******************************************************************************/
/*                                                                            */
/*                              Defines                                       */
/*                                                                            */
/******************************************************************************/

/* Used by other protocols to resolve address */
#define ARP_FRAME_SIZE 28




/******************************************************************************/
/*                                                                            */
/*                     ARP Function Prototypes                                */
/*                                                                            */
/******************************************************************************/


/********************************************************
 * @brief  Function to send arp request
 * @param  *ethernet  : reference to the Ethernet handle
 * @param  *sender_ip : sender ip address
 * @param  *target_ip : target ip address
 * @retval int16_t    : Error = -1, Success = 0
 ********************************************************/
int16_t ether_send_arp_req(ethernet_handle_t *ethernet, uint8_t *sender_ip, uint8_t *target_ip);



/**********************************************************
 * @brief  Function to independently read ARP data
 *         (Blocking Call)
 * @param  *ethernet    : reference to the Ethernet handle
 * @param  *data        : network_data
 * @param  *data_length : length of data to be read
 * @retval uint8_t      : Error = 0, Success = 1
 **********************************************************/
uint8_t ether_is_arp(ethernet_handle_t *ethernet, uint8_t *data, uint16_t data_length);



/******************************************************************
 * @brief  Function to handle ARP request and reply
 *         sends ARP reply if ARP request received
 * @param  *ethernet  : reference to the Ethernet handle
 * @retval int16_t    : Error = -2, -3 = reply ignore, Success = 0
 ******************************************************************/
int16_t ether_handle_arp_resp_req(ethernet_handle_t *ethernet);


int8_t search_arp_table(ethernet_handle_t *ethernet, uint8_t *destination_mac, uint8_t *destination_ip);




#endif /* ARP_H_ */
