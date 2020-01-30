/**
 ******************************************************************************
 * @file    network_utilities.h
 * @author  Aditya Mall,
 * @brief   Network utility header file
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

#ifndef NETWORK_UTILITIES_H_
#define NETWORK_UTILITIES_H_




/*
 * Standard header and api header files
 */
#include <stdint.h>





/******************************************************************************/
/*                                                                            */
/*                      Data Structures and Defines                           */
/*                                                                            */
/******************************************************************************/


#define ntohs htons




/******************************************************************************/
/*                                                                            */
/*                  Network Utility Function Prototypes                       */
/*                                                                            */
/******************************************************************************/



/************************************************************************
 * @brief  Function to convert from host to network order and vice versa
 *         (For 16 bit data)
 * @retval uint16_t : host to network or network to host converted data
 ************************************************************************/
uint16_t htons(uint16_t value);



/********************************************************
 * @brief  Function to set mac address
 * @param  *device_mac  : device mac address (Hex)
 * @param  *mac_address : mac address (string)
 * @retval int8_t       : Error = -1, Success = 0
 ********************************************************/
int8_t set_mac_address(char *device_mac, char *mac_address);



/********************************************************
 * @brief  function to set ip address
 * @param  *host_ip    : host ip address (integer)
 * @param  *ip_address : ip address (string)
 * @retval int8_t      : Error = -1, Success = 0
 ********************************************************/
int8_t set_ip_address(uint8_t *host_ip, char *ip_address);




int8_t net_swap_address(char* l_address, char *r_address, uint8_t size);



#endif /* NETWORK_UTILITIES_H_ */
