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




/******************************************************************************/
/*                                                                            */
/*                      Data Structures and Defines                           */
/*                                                                            */
/******************************************************************************/


typedef struct _ip_ver_size
{
    uint8_t header_length : 4;
    uint8_t version       : 4;

}ip_ver_size_t;


typedef struct _ip_flags_offset
{
    uint16_t fragment_offset : 13;
    uint16_t flags           : 3;

}ip_flags_offset;


typedef struct _net_ip
{
    ip_ver_size_t   version_length;
    uint8_t         service_type;
    uint16_t        total_length;
    uint16_t        id;
    ip_flags_offset flags_offset;
    uint8_t         ttl;
    uint8_t         protocol;
    uint16_t        header_checksum;
    uint8_t         source_ip[4];
    uint8_t         destination_ip[4];

}net_ip_t;






/******************************************************************************/
/*                                                                            */
/*                           IPV4 Functions                                   */
/*                                                                            */
/******************************************************************************/



int16_t get_ether_ip_data(ethernet_handle_t *ethernet)
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
