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

#include "icmp.h"
#include "ipv4.h"
#include "network_utilities.h"



/******************************************************************************/
/*                                                                            */
/*                      Data Structures and Defines                           */
/*                                                                            */
/******************************************************************************/


/* ICMP Frame */
typedef struct _net_icmp
{
    uint8_t  type;         /*!< */
    uint8_t  code;         /*!< */
    uint16_t checksum;     /*!< */
    uint16_t id;           /*!< */
    uint16_t sequence_no;  /*!< */
    uint8_t  data;         /*!< */

} net_icmp_t;


/* ICMP message types */
typedef enum _icmp_type
{
    ICMP_UNREACHABLE = 3,  /*!< */
    ICMP_ECHOREPLY   = 0,  /*!< */
    ICMP_ECHOREQUEST = 8,  /*!< */
    ICMP_TRACEROUTE  = 30, /*!< */

}icmp_type_t;





/******************************************************************************/
/*                                                                            */
/*                               ICMP Functions                               */
/*                                                                            */
/******************************************************************************/




int8_t ether_send_icmp_reply(ethernet_handle_t *ethernet)
{
    int8_t func_retval = 0;

    net_ip_t   *ip;
    net_icmp_t *icmp;

    uint32_t sum = 0;
    uint16_t ip_packet_length    = 0;
    uint16_t icmp_remaining_size = 0;

    if(ethernet->ether_obj == NULL)
    {
        func_retval = NET_ICMP_RESP_ERROR;
    }
    else
    {
        ip = (void*)&ethernet->ether_obj->data;

        icmp = (void*)( (uint8_t*)ip + ((ip->version_length.header_length) * 4) );

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
            icmp_remaining_size = ip_packet_length - (IP_HEADER_SIZE + 4);

            ether_sum_words(&sum, &icmp->id, icmp_remaining_size);

            /* Get checksum */
            icmp->checksum = ether_get_checksum(sum);

            /* Send ICMP response packet */
            ethernet->ether_commands->ether_send_packet((uint8_t*)ethernet->ether_obj, ip_packet_length + ETHER_FRAME_SIZE);

        }
        else
        {
            func_retval = NET_ICMP_RESP_IGNORE;
        }

    }

    return func_retval;
}

