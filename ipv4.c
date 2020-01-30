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



/************************************************************************
 * @brief  Function to get IP communication type for current host device
 *         (Only handles UNICAST)
 * @param  *ethernet  : reference to the Ethernet handle
 * @retval int16_t    : Error = -4, -5, Success = 1 (UNICAST)
 ************************************************************************/
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




ip_protocol_type_t get_ip_protocol_type(ethernet_handle_t *ethernet)
{
    ip_protocol_type_t protocol;

    net_ip_t *ip;

    ip = (void*)&ethernet->ether_obj->data;

    protocol = (ip_protocol_type_t)ip->protocol;

    return protocol;
}







