/**
 ******************************************************************************
 * @file    udp.h
 * @author  Aditya Mall,
 * @brief   UDP protocol header file
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


#ifndef _UDP_H
#define _UDP_H



/*
 * Standard header and api header files
 */
#include <stdint.h>

#include "ethernet.h"




/******************************************************************************/
/*                                                                            */
/*                       UDP Functions Prototypes                             */
/*                                                                            */
/******************************************************************************/



/****************************************************************
 * @brief  Function to get UDP data inside network state machine
 * @param  *ethernet        : Reference to the Ethernet handle
 * @param  *data            : UDP data
 * @param  data_length      : Length of UDP data
 * @retval uint8_t          : Error = 0, Success = 1
 ****************************************************************/
uint8_t ether_get_udp_data(ethernet_handle_t *ethernet, uint8_t *data, uint16_t data_length);




/*******************************************************************
 * @brief  Raw Function to send UPD packets
 *         UDP data dependent upon total data allocated to
 *         ethernet object
 * @param  *ethernet        : Reference to the Ethernet handle
 * @param  *source_addr     : Reference to source address structure
 * @param  *destination_ip  : Destination IP address
 * @param  *destination_mac : Destination MAC address
 * @param  destination_port : UDP destination port
 * @param  *data            : UDP data
 * @param  data_length      : Length of UDP data
 * @retval int8_t           : Error = -9, Success = 0
 *******************************************************************/
int8_t ether_send_udp_raw(ethernet_handle_t *ethernet, ether_source_t *source_addr, uint8_t *destination_ip,
                      uint8_t *destination_mac, uint16_t destination_port, uint8_t *data, uint16_t data_length);




/****************************************************************
 * @brief  Function detect UDP packet, state machine independent
 * @param  *ethernet           : Reference to the Ethernet handle
 * @param  *network_data       : network data from PHY
 * @param  network_data_length : network data length to be read
 * @retval uint8_t             : Error   = 0,
 *                               Success = 1 (UNICAST)
 *                                       = 2 (BROADCAST)
 ****************************************************************/
uint8_t ether_is_udp(ethernet_handle_t *ethernet, uint8_t *network_data, uint16_t network_data_length);





/*****************************************************************
 * @brief  Function to read UPD packets
 * @param  *ethernet           : Reference to the Ethernet handle
 * @param  *network_data       : network data from PHY
 * @param  *application_data   : UDP data
 * @param  app_data_length     : Length of UDP data
 * @retval int8_t              : Error = 0, Success = 1
 *****************************************************************/
uint8_t ether_read_udp(ethernet_handle_t *ethernet, uint8_t *network_data, char *application_data, uint16_t app_data_length);



/**************************************************************
 * @brief  Function to send UPD packets
 *         UDP data dependent upon total data allocated to
 *         ethernet object
 * @param  *ethernet         : Reference to the Ethernet handle
 * @param  *destination_ip   : Destination IP address
 * @param  destination_port  : UDP destination port
 * @param  *application_data : UDP data
 * @param  data_length       : Length of UDP data
 * @retval int8_t            : Error = -10, Success = 0
 **************************************************************/
int8_t ether_send_udp(ethernet_handle_t *ethernet, uint8_t *destination_ip, uint16_t destination_port,
                      char *application_data, uint16_t data_length);





/**************************************************************
 * @brief  Function to read UDP packet
 * @param  *ethernet         : reference to the Ethernet handle
 * @param  *network_data     : network data from the ether PHY
 * @param  net_data_length   : network data length
 * @param  *source_port      : UDP source port
 * @param  *destination_port : UDP destination port
 * @param  *application_data : UDP data
 * @param  data_length       : Length of UDP data
 * @param app_data_length   :
 * @retval uint16_t          : Error = 0, Success = bytes read
 **************************************************************/
uint16_t ether_read_udp_raw(ethernet_handle_t *ethernet, uint8_t *network_data, uint16_t net_data_length,
                           uint16_t *source_port, uint16_t *destination_port, char *application_data, uint16_t app_data_length);




#endif /* UDP_H_ */
