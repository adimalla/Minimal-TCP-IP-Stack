/**
 ******************************************************************************
 * @file    icmp.c
 * @author  Aditya Mall,
 * @brief   ICMP (for IPV4) source file
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
#include <stdlib.h>

#include "icmp.h"
#include "ipv4.h"
#include "network_utilities.h"

#include <time.h>

/******************************************************************************/
/*                                                                            */
/*                      Data Structures and Defines                           */
/*                                                                            */
/******************************************************************************/


#define ICMP_FRAME_SIZE 8



/* ICMP Frame (8 bytes) */
typedef struct _net_icmp
{
    uint8_t  type;         /*!< ICMP message type              */
    uint8_t  code;         /*!< ICMP code for subtype messages */
    uint16_t checksum;     /*!< ICMP message checksum          */
    uint16_t id;           /*!< ICMP message ID                */
    uint16_t sequence_no;  /*!< ICMP message sequence number   */
    uint8_t  data;         /*!< ICMP message data              */

} net_icmp_t;




/******************************************************************************/
/*                                                                            */
/*                               ICMP Functions                               */
/*                                                                            */
/******************************************************************************/




/******************************************************************
 * @brief  Function to send ICMP reply packet
 * @param  *ethernet  : reference to the Ethernet handle
 * @retval int16_t    : Error = -6, -7 = reply ignore, Success = 0
 ******************************************************************/
int8_t ether_send_icmp_reply(ethernet_handle_t *ethernet)
{
    int8_t func_retval = 0;

    net_ip_t   *ip;
    net_icmp_t *icmp;

    uint32_t sum = 0;
    uint16_t ip_header_size      = 0;
    uint16_t ip_packet_length    = 0;
    uint16_t icmp_remaining_size = 0;

    if(ethernet->ether_obj == NULL)
    {
        func_retval = NET_ICMP_RESP_ERROR;
    }
    else
    {
        ip   = (void*)&ethernet->ether_obj->data;

        ip_header_size = ip->version_length.header_length * 4;

        icmp = (void*)( (uint8_t*)ip + ip_header_size);

        /* Send ECHO reply */
        if(icmp->type == ICMP_ECHOREQUEST)
        {
            /* Swap MAC address in Ethernet Frame */
            net_swap_address((char*)ethernet->ether_obj->destination_mac_addr, (char*)ethernet->ether_obj->source_mac_addr, 6);

            /* Swap IP address in IP Frame */
            net_swap_address((char*)ip->destination_ip, (char*)ip->source_ip, 4);

            /* Fill ICMP frame */
            icmp->type = ICMP_ECHOREPLY;

            /*** Calculate ICMP Checksum ***/

            ether_sum_words(&sum, &icmp->type, 2);

            /* get total length of IP packet */
            ip_packet_length = ntohs(ip->total_length);

            /* ICMP Remaining packet size = IP Length - (ICMP type+code+checksum size) */
            icmp_remaining_size = ip_packet_length - (ip_header_size + 4);

            ether_sum_words(&sum, &icmp->id, icmp_remaining_size);

            /* Get checksum */
            icmp->checksum = ether_get_checksum(sum);

            /* Send ICMP response packet(uses callback) */
            ether_send_data(ethernet, (uint8_t*)ethernet->ether_obj, ip_packet_length + ETHER_FRAME_SIZE);

        }
        else
        {
            func_retval = NET_ICMP_RESP_IGNORE;
        }

    }

    return func_retval;
}





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
                           uint8_t *sequence_no, uint8_t* destination_mac, uint8_t *source_mac)
{
    int8_t func_retval = 0;

    net_ip_t   *ip;
    net_icmp_t *icmp;

    uint32_t sum              = 0;
    uint16_t icmp_packet_size = 0;
    uint16_t ip_id            = 0;

    if(ethernet->ether_obj == NULL)
    {
        func_retval = NET_ICMP_REQ_ERROR;
    }
    else
    {

        /* Fill ICMP frame */
        ip  = (void*)&ethernet->ether_obj->data;

        icmp = (void*)( (uint8_t*)ip + IP_HEADER_SIZE );

        icmp->type = ICMP_ECHOREQUEST;

        icmp->code = 0;

        /* Currently fixed */
        icmp->id = 15625;

        icmp->sequence_no = htons(*sequence_no);

        (*sequence_no)++;

        /* Calculate ICMP checksum */
        sum = 0;
        ether_sum_words(&sum, &icmp->type, 2);

        ether_sum_words(&sum, &icmp->id, 5);

        icmp->checksum = ether_get_checksum(sum);

        icmp_packet_size = ICMP_FRAME_SIZE;


        /* Fill IP frame */
        fill_ip_frame(ip, &ip_id, destination_ip, ethernet->host_ip, IP_ICMP, icmp_packet_size);


        /* Fill Ethernet frame */
        fill_ether_frame(ethernet, destination_mac, source_mac, ETHER_IPV4);


        /* Send ICMP data */
        ether_send_data(ethernet, (uint8_t*)ethernet->ether_obj, ETHER_FRAME_SIZE + htons(ip->total_length));

    }


    return func_retval;
}











