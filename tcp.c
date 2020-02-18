/**
 ******************************************************************************
 * @file    TCP.c
 * @author  Aditya Mall,
 * @brief   TCP protocol source file
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
#include "arp.h"

#include "network_utilities.h"

#include "tcp.h"



/******************************************************************************/
/*                                                                            */
/*                              Private Functions                             */
/*                                                                            */
/******************************************************************************/




static uint16_t get_tcp_checksum(net_ip_t *ip, net_tcp_t *tcp, uint16_t data_length)
{
    uint16_t func_retval = 0;

    uint32_t sum             = 0;
    uint16_t pseudo_protocol = 0;
    uint16_t tcp_length      = 0;

    /* TCP Pseudo Header checksum calculation */
    sum = 0;

    ether_sum_words(&sum, ip->source_ip, 8);

    pseudo_protocol = ip->protocol;

    /* create space for reserved bits */
    sum += ( (pseudo_protocol & 0xFF) << 8 );

    /* TCP data_length = options + data to checksum */
    tcp_length = htons(TCP_FRAME_SIZE + data_length);

    sum += tcp_length;

    /* TCP header checksum */
    ether_sum_words(&sum, tcp, 12);

    ether_sum_words(&sum, &tcp->data_offset, 4);

    /* TCP data_length = options + data to checksum */
    ether_sum_words(&sum, &tcp->data, data_length);

    func_retval = ether_get_checksum(sum);

    return func_retval;

}





/******************************************************************************/
/*                                                                            */
/*                               TCP Functions                                */
/*                                                                            */
/******************************************************************************/




int8_t ether_send_tcp_syn(ethernet_handle_t *ethernet, uint16_t source_port, uint16_t destination_port,
                              uint32_t sequence_number, uint32_t ack_number, uint8_t *destination_ip)
{

    net_ip_t  *ip;
    net_tcp_t *tcp;
    tcp_syn_opts_t *syn_option;

    uint16_t ip_identifier = 0;

    /* Ethernet Frame related variables */
    uint8_t  destination_mac[ETHER_MAC_SIZE] = {0};

    ip  = (void*)&ethernet->ether_obj->data;

    tcp = (void*)( (uint8_t*)ip + IP_HEADER_SIZE );


    /* Fill TCP frame */
    tcp->source_port      = htons(source_port);
    tcp->destination_port = htons(destination_port);

    tcp->sequence_number  = htons(sequence_number);
    tcp->ack_number       = htons(ack_number);

    /* Shift data offset to Big Endian MSB (4 bits) */
    tcp->data_offset      = ((TCP_FRAME_SIZE + 12) >> 2) << 4;
    tcp->control_bits     = 2;

    tcp->window           = ntohs(1);
    tcp->urgent_pointer   = 0;

    /* Configure TCP options */
    syn_option = (void*)&tcp->data;

    syn_option->mss.option_kind = TCP_MAX_SEGMENT_SIZE;
    syn_option->mss.length      = 4;
    syn_option->mss.value       = ntohs(ETHER_MTU_SIZE);

    syn_option->sack.option_kind = TCP_SACK_PERMITTED;
    syn_option->sack.length      = 2;

    syn_option->nop.option_kind = TCP_NO_OPERATION;
    syn_option->nop1.option_kind = TCP_NO_OPERATION;
    syn_option->nop2.option_kind = TCP_NO_OPERATION;

    syn_option->window_scale.option_kind = TCP_WINDOW_SCALING;
    syn_option->window_scale.length      = 3;
    syn_option->window_scale.value       = 7;


    /* fill IP frame before TCP checksum calculation */
    ip_identifier = get_unique_id(ethernet, 2000);

    fill_ip_frame(ip, &ip_identifier, destination_ip, ethernet->host_ip, IP_TCP, 32);

    /*Get TCP checksum */
    tcp->checksum = get_tcp_checksum(ip, tcp, 12);


    /* Get MAC address from ARP table */
    ether_arp_resolve_address(ethernet, destination_mac, destination_ip);


    /* Fill Ethernet frame */
    fill_ether_frame(ethernet, destination_mac, ethernet->host_mac, ETHER_IPV4);

    ether_send_data(ethernet,(uint8_t*)ethernet->ether_obj, ETHER_FRAME_SIZE + htons(ip->total_length));


    return 0;
}
