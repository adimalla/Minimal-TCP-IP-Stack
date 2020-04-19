/**
 ******************************************************************************
 * @file    ipv4.h
 * @author  Aditya Mall,
 * @brief   IP version 4 protocol header file
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

#ifndef IPV4_H_
#define IPV4_H_


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


#define IP_VERSION        4       /*!< IP protocol version                      */
#define IP_HEADER_LENGTH  5       /*!< IP header length value (not header size) */
#define IP_DF_SET         0x4000  /*!< Don't Fragment set value                 */
#define IP_TTL_VALUE      64      /*!< Time to Live value                       */
#define IP_HEADER_SIZE    20      /*!< IP header size                           */

/* IP version and header length fields */
typedef struct _ip_ver_size
{
    uint8_t header_length : 4;  /*!< Header length value */
    uint8_t version       : 4;  /*!< IP version value    */

}ip_ver_size_t;


/* IP Frame structure (20 Bytes in current implementation) */
typedef struct _net_ip
{
    ip_ver_size_t   version_length;     /*!< IP version and header length      */
    uint8_t         service_type;       /*!< IP quality of service type        */
    uint16_t        total_length;       /*!< IP packet total length            */
    uint16_t        id;                 /*!< IP packet ID                      */
    uint16_t        flags_offset;       /*!< IP fragmentation flags and offset */
    uint8_t         ttl;                /*!< Time to Live value                */
    uint8_t         protocol;           /*!< IP protocol type                  */
    uint16_t        header_checksum;    /*!< IP header checksum                */
    uint8_t         source_ip[4];       /*!< Source IP address                 */
    uint8_t         destination_ip[4];  /*!< Destination IP address            */

}net_ip_t;


/* IP protocol type values */
typedef enum _ip_protocol_type
{
    IP_ICMP = 0x01,
    IP_TCP  = 0x06,
    IP_UDP  = 0x11,

}ip_protocol_type_t;




/******************************************************************************/
/*                                                                            */
/*                     IPV4 Function Prototypes                               */
/*                                                                            */
/******************************************************************************/



/**************************************************************
 * @brief  Function to get IP data for current host device
 *         validates IP Checksum, (Only handles UNICAST)
 * @param  *ethernet  : reference to the Ethernet handle
 * @retval int16_t    : Error   = -4, -5,
 *                      Success = 1 (UNICAST),
 *                              = 2 (BROADCAST)
 **************************************************************/
int16_t get_ip_communication_type(ethernet_handle_t *ethernet);




/****************************************************************
 * @brief  Function to get IP protocol type
 * @param  *ethernet  : reference to the Ethernet handle
 * @retval int16_t    : Error = -4, -5, Success = protocol type
 ***************************************************************/
ip_protocol_type_t get_ip_protocol_type(ethernet_handle_t *ethernet);




/**********************************************************
 * @brief  Function to fill the IP frame
 * @param  *ip             : reference to the IP structure
 * @param  *id             : reference to IP identifier
 * @param  *destination_ip : destination IP address
 * @param  *source_ip      : source IP address
 * @param  protocol        : IP protocol type
 * @param  data_size       : size of payload
 * @retval int8_t          : Error = NULL
 **********************************************************/
int8_t fill_ip_frame(net_ip_t *ip, uint16_t *id, uint8_t *destination_ip, uint8_t *source_ip, ip_protocol_type_t protocol, uint16_t data_size);




#endif /* IPV4_H_ */
