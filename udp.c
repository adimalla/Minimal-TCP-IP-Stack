/**
 ******************************************************************************
 * @file    udp.c
 * @author  Aditya Mall,
 * @brief   ethernet layer header file
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
#include "udp.h"

#include "network_utilities.h"




/******************************************************************************/
/*                                                                            */
/*                      Data Structures and Defines                           */
/*                                                                            */
/******************************************************************************/


#define UDP_FRAME_SIZE 8


typedef struct _net_udp // 8 bytes
{
    uint16_t source_port;
    uint16_t destination_port;
    uint16_t length;
    uint16_t checksum;
    uint8_t  data;

}net_udp_t;




/******************************************************************************/
/*                                                                            */
/*                               UDP Functions                                */
/*                                                                            */
/******************************************************************************/



/***************************************************************
 * @brief  Function to get UDP data
 * @param  *ethernet        : Reference to the Ethernet handle
 * @param  *data            : UDP data
 * @param  data_length      : Length of UDP data
 * @retval int8_t           : Error = 0, Success = 1
 ***************************************************************/
uint8_t ether_get_udp_data(ethernet_handle_t *ethernet, uint8_t *data, uint8_t data_length)
{
    uint8_t func_retval = 0;

    net_ip_t  *ip;
    net_udp_t *udp;

    uint16_t  pseudo_protocol = 0;
    uint32_t  sum             = 0;
    uint8_t   validate        = 0;

    uint16_t  udp_packet_length = 0;
    uint16_t  udp_data_length   = 0;

    ip  = (void*)&ethernet->ether_obj->data;

    udp = (void*)( (uint8_t*)ip + IP_HEADER_SIZE );


    if(ethernet->ether_obj == NULL || data == NULL)
    {
        func_retval = 0;
    }
    else
    {
        udp_packet_length = ntohs(udp->length);
        udp_data_length   = udp_packet_length - UDP_FRAME_SIZE;

        /* Check and Truncate UDP data length */
        if(udp_data_length > data_length)
            udp_data_length = data_length;

        /* validate UDP checksum */
        sum = 0;

        ether_sum_words(&sum, &ip->source_ip, 8);

        pseudo_protocol = ip->protocol;

        sum += ( (pseudo_protocol & 0xFF) << 8 );

        ether_sum_words(&sum, &udp->length, 2);

        ether_sum_words(&sum, udp, ntohs(udp->length));

        validate = (ether_get_checksum(sum) == 0);

        /* Get UDP data */
        if(validate)
        {
            memcpy((char*)data, (char*)&udp->data, udp_data_length);
        }

        func_retval = validate;
    }


    return func_retval;
}




/*******************************************************************
 * @brief  Function to send UPD packets
 * @param  *ethernet        : Reference to the Ethernet handle
 * @param  *source_addr     : Reference to source address structure
 * @param  *destination_ip  : Destination IP address
 * @param  *destination_mac : Destination MAC address
 * @param  destination_port : UDP destination port
 * @param  *data            : UDP data
 * @param  data_length      : Length of UDP data
 * @retval int8_t           : Error = 0, Success = 0
 *******************************************************************/
uint8_t ether_send_upd(ethernet_handle_t *ethernet, ether_source_t *source_addr, uint8_t *destination_ip,
                       uint8_t *destination_mac, uint16_t destination_port, uint8_t *data, uint8_t data_length)
{

    net_ip_t  *ip;
    net_udp_t *udp;

    uint8_t  index = 0;

    uint8_t  *data_copy;
    uint16_t pseudo_protocol = 0;
    uint32_t sum             = 0;
    uint16_t udp_packet_size = 0;

    ip  = (void*)&ethernet->ether_obj->data;

    udp = (void*)( (uint8_t*)ip + IP_HEADER_SIZE );


    /* Fill UDP frame */
    udp->source_port      = htons(source_addr->source_port);
    udp->destination_port = htons(destination_port);

    udp->length = htons(UDP_FRAME_SIZE + data_length);


    /* Add UDP data */
    data_copy = &udp->data;

    for(index = 0; index < data_length; index++)
    {
        data_copy[index] = data[index];
    }


    /* Fill IP frame */
    udp_packet_size = UDP_FRAME_SIZE + data_length;

    fill_ip_frame(ip, destination_ip, source_addr->source_ip, IP_UDP, udp_packet_size);


    /************ Checksum Calculations Begin *****************/

    /* UDP Pseudo Header checksum calculation */
    sum = 0;

    ether_sum_words(&sum, ip->source_ip, 8);

    pseudo_protocol = ip->protocol;

    sum += ( (pseudo_protocol & 0xFF) << 8 );

    ether_sum_words(&sum, &udp->length, 2);

    /* UDP Fixed header checksum calculation, excluding checksum field */
    ether_sum_words(&sum, udp, UDP_FRAME_SIZE - 2);

    ether_sum_words(&sum, &udp->data, data_length);

    udp->checksum = ether_get_checksum(sum);

    /************ Checksum Calculations End *****************/


    /* Fill Ethernet frame */
    fill_ether_frame(ethernet, destination_mac, source_addr->source_mac, ETHER_IPV4);


    /* Send UPD data */
    ether_send_data(ethernet,(uint8_t*)ethernet->ether_obj, ETHER_FRAME_SIZE + htons(ip->total_length));


    return 0;
}




