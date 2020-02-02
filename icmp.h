/**
 ******************************************************************************
 * @file    icmp.h
 * @author  Aditya Mall,
 * @brief   ICMP (for IPV4) header file
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


#ifndef ICMP_H_
#define ICMP_H_




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


/* ICMP message types */
typedef enum _icmp_type
{
    ICMP_ECHOREPLY   = 0,  /*!< ICMP Echo reply code              */
    ICMP_UNREACHABLE = 3,  /*!< ICMP destination unreachable code */
    ICMP_ECHOREQUEST = 8,  /*!< ICMP Echo request code            */
    ICMP_TRACEROUTE  = 30, /*!< ICMP Trace route code             */

}icmp_type_t;




/******************************************************************************/
/*                                                                            */
/*                       ICMP Function Prototypes                             */
/*                                                                            */
/******************************************************************************/




/******************************************************************
 * @brief  Function to send ICMP reply packet
 * @param  *ethernet  : reference to the Ethernet handle
 * @retval int16_t    : Error = -6, -7 = reply ignore, Success = 0
 ******************************************************************/
int8_t ether_send_icmp_reply(ethernet_handle_t *ethernet);



/*****************************************************************
 * @brief  Function to send ICMP request
 * @param  *ethernet        : Reference to the Ethernet structure
 * @param  icmp_type        : ICMP request type
 * @param  *destination_ip  : Destination IP address
 * @param  sequence_no      : ICMP packet sequence Number
 * @param  *destination_mac : Destination MAC address
 * @param  *source_mac      : Source MAC address
 * @retval int8_t           : Error = -8, Success = 0.
 ****************************************************************/
int8_t ether_send_icmp_req(ethernet_handle_t *ethernet, icmp_type_t icmp_type, uint8_t *destination_ip,
                           uint8_t *sequence_no, uint8_t* destination_mac, uint8_t *source_mac);


#endif /* ICMP_H_ */
