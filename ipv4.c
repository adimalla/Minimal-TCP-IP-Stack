/**
 ******************************************************************************
 * @file    ipv4.c
 * @author  Aditya Mall,
 * @brief   IP version 4 protocol source file
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

#include "ipv4.h"

#include "network_utilities.h"



/******************************************************************************/
/*                                                                            */
/*                      Data Structures and Defines                           */
/*                                                                            */
/******************************************************************************/





/******************************************************************************/
/*                                                                            */
/*                           IPV4 Functions                                   */
/*                                                                            */
/******************************************************************************/



/**************************************************************
 * @brief  Function to get IP data for current host device
 *         validates IP Checksum, (Only handles UNICAST)
 * @param  *ethernet  : reference to the Ethernet handle
 * @retval int16_t    : Error = -4, -5, Success = 1 (UNICAST)
 **************************************************************/
int16_t get_ip_communication_type(ethernet_handle_t *ethernet)
{
    int16_t func_retval = 0;

    net_ip_t *ip;

    uint32_t sum = 0;

    if(ethernet->ether_obj == NULL)
    {
        func_retval = NET_IP_GET_ERROR;
    }
    else
    {
        ip = (void*)&ethernet->ether_obj->data;

        /* Validate IP header checksum */
        ether_sum_words(&sum, ip, (ip->version_length.header_length) * 4);

        if( (ether_get_checksum(sum) == 0)  )
        {
            /* Check if UNICAST */
            if( strncmp((char*)ip->destination_ip, (char*)ethernet->host_ip, 4) == 0)
            {
                func_retval = 1;
            }

        }
        else
        {
            func_retval = NET_IP_CHECKSUM_ERROR;
        }

    }

    return func_retval;
}



/****************************************************************
 * @brief  Function to get IP protocol type
 * @param  *ethernet  : reference to the Ethernet handle
 * @retval int16_t    : Error = -4, -5, Success = protocol type
 ***************************************************************/
ip_protocol_type_t get_ip_protocol_type(ethernet_handle_t *ethernet)
{
    ip_protocol_type_t protocol;

    net_ip_t *ip;

    ip = (void*)&ethernet->ether_obj->data;

    protocol = (ip_protocol_type_t)ip->protocol;

    return protocol;
}



/**********************************************************
 * @brief  Function to fill the IP frame
 * @param  *ip             : reference to the IP structure
 * @param  *destination_ip : destination IP address
 * @param  *source_ip      : source IP address
 * @param  protocol        : IP protocol type
 * @param  data_size       : size of payload
 * @retval int8_t          : Error = NULL
 **********************************************************/
int8_t fill_ip_frame(net_ip_t *ip, uint8_t *destination_ip, uint8_t *source_ip, ip_protocol_type_t protocol, uint16_t data_size)
{
    int8_t func_retval = 0;

    uint8_t i = 0;

    uint32_t sum = 0;

    if(ip == NULL || destination_ip == NULL || source_ip == NULL || data_size == 0 || data_size > UINT16_MAX)
    {
        func_retval = -1;
    }
    else
    {

        ip->version_length.version       = IP_VERSION;
        ip->version_length.header_length = IP_HEADER_LENGTH;

        ip->service_type = 0;

        ip->id = 0;

        /* Don't Fragment set condition */
        ip->flags_offset = htons(IP_DF_SET);

        ip->ttl = IP_TTL_VALUE;

        ip->protocol = protocol;

        ip->total_length = htons(IP_HEADER_SIZE + data_size);

        for(i=0; i < 4; i++)
        {
            ip->destination_ip[i] = destination_ip[i];
            ip->source_ip[i]      = source_ip[i];
        }

        /* Calculate checksum */
        sum = 0;
        ether_sum_words(&sum, &ip->version_length, 10);

        ether_sum_words(&sum, ip->source_ip, 8);

        ip->header_checksum = ether_get_checksum(sum);
    }


    return func_retval;
}







