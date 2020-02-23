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

#include "icmp.h"


/******************************************************************************/
/*                                                                            */
/*                      Data Structures and Defines                           */
/*                                                                            */
/******************************************************************************/







/******************************************************************************/
/*                                                                            */
/*                              Private Functions                             */
/*                                                                            */
/******************************************************************************/




/*******************************************************
 * @brief  Static function to calculate TCP checksum
 * @param  *ip     : Reference to IP frame structure
 * @param  *udp    : Reference to TCP frame structure
 * @retval uint8_t : Error = 0, Success = TCP checksum
 ******************************************************/
static uint16_t get_tcp_checksum(net_ip_t *ip, net_tcp_t *tcp, uint16_t data_length)
{
    uint16_t func_retval     = 0;
    uint32_t sum             = 0;
    uint16_t pseudo_protocol = 0;
    uint16_t tcp_length      = 0;


    if(ip == NULL || tcp == NULL)
    {
        func_retval = 0;
    }
    else
    {
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
    }

    return func_retval;

}



/******************************************************
 * @brief  Static function to validate TCP checksum
 * @param  *ip     : Reference to IP frame structure
 * @param  *udp    : Reference to TCP frame structure
 * @retval uint8_t : Error = 0, Success = 1
 ******************************************************/
static uint8_t validate_tcp_checksum(net_ip_t *ip, net_tcp_t *tcp)
{
    uint8_t func_retval      = 0;
    uint32_t sum             = 0;
    uint16_t pseudo_protocol = 0;
    uint16_t tcp_length      = 0;

    if(ip == NULL || tcp == NULL)
    {
        func_retval = 0;
    }
    else
    {
        /* validate TCP checksum */
        sum = 0;

        ether_sum_words(&sum, &ip->source_ip, 8);

        pseudo_protocol = ip->protocol;

        sum += ( (pseudo_protocol & 0xFF) << 8 );

        tcp_length = ( ntohs(ip->total_length) - IP_HEADER_SIZE );

        sum += htons(tcp_length);

        ether_sum_words(&sum, tcp, tcp_length);

        func_retval = (ether_get_checksum(sum) == 0);
    }

    return func_retval;
}




/******************************************************************************/
/*                                                                            */
/*                               TCP Functions                                */
/*                                                                            */
/******************************************************************************/



/**********************************************************
 * @brief  Static function for sending TCP SYN packet
 * @param  *ethernet        : Reference to Ethernet handle
 * @param  source_port      : TCP source port
 * @param  destination_port : TCP destination port
 * @param  sequence_number  : TCP sequence number
 * @param  ack_number       : TCP acknowledgment number
 * @param  *destination_ip  : Destination server IP
 * @retval uint8_t          : Error = 0, Success = 1
 **********************************************************/
static int8_t ether_send_tcp_syn(ethernet_handle_t *ethernet, uint16_t source_port, uint16_t destination_port,
                                 uint32_t sequence_number, uint32_t ack_number, uint8_t *destination_ip)
{

    int8_t func_retval = 0;

    net_ip_t  *ip;
    net_tcp_t *tcp;
    tcp_syn_opts_t *syn_option;

    /* Ethernet Frame related variables */
    uint8_t  destination_mac[ETHER_MAC_SIZE] = {0};

    if(ethernet->ether_obj == NULL || destination_ip == NULL)
    {
        func_retval = 0;
    }
    else
    {

        ip  = (void*)&ethernet->ether_obj->data;

        tcp = (void*)( (uint8_t*)ip + IP_HEADER_SIZE );

        /* Fill TCP frame */
        tcp->source_port      = htons(source_port);
        tcp->destination_port = htons(destination_port);

        tcp->sequence_number  = htonl(sequence_number);
        tcp->ack_number       = htonl(ack_number);

        /* Shift data offset to Big-Endian MSB (4 bits) */
        tcp->data_offset      = ((TCP_FRAME_SIZE + 12) >> 2) << 4;
        tcp->control_bits     = TCP_SYN;

        tcp->window           = ntohs(1);
        tcp->urgent_pointer   = 0;

        /* Configure TCP options */
        syn_option = (void*)&tcp->data;

        syn_option->mss.option_kind = TCP_MAX_SEGMENT_SIZE;
        syn_option->mss.length      = 4;
        syn_option->mss.value       = ntohs(ETHER_MTU_SIZE);

        syn_option->sack.option_kind = TCP_SACK_PERMITTED;
        syn_option->sack.length      = 2;

        syn_option->nop.option_kind  = TCP_NO_OPERATION;
        syn_option->nop1.option_kind = TCP_NO_OPERATION;
        syn_option->nop2.option_kind = TCP_NO_OPERATION;

        syn_option->window_scale.option_kind = TCP_WINDOW_SCALING;
        syn_option->window_scale.length      = 3;
        syn_option->window_scale.value       = 7;


        /* fill IP frame before TCP checksum calculation */
        fill_ip_frame(ip, &ethernet->ip_identifier, destination_ip, ethernet->host_ip, IP_TCP, TCP_FRAME_SIZE + TCP_SYN_OPTS_SIZE);

        /*Get TCP checksum */
        tcp->checksum = get_tcp_checksum(ip, tcp, 12);

        /* Get MAC address from ARP table */
        ether_arp_resolve_address(ethernet, destination_mac, destination_ip);

        /* Fill Ethernet frame */
        fill_ether_frame(ethernet, destination_mac, ethernet->host_mac, ETHER_IPV4);

        /*Send TCP data */
        ether_send_data(ethernet,(uint8_t*)ethernet->ether_obj, ETHER_FRAME_SIZE + htons(ip->total_length));
    }

    return func_retval;
}




/*****************************************************************
 * @brief  Function for getting TCP sever ACK packets
 * @param  *ethernet        : Reference to Ethernet handle
 * @param  *sequence_number : Reference to TCP sequence number
 * @param  *ack_number      : Reference to acknowledgment number
 * @param  server_src_port  : TCP destination /server port
 * @param  client_src_port  : TCP client source port
 * @param  *sever_ip        : Destination server IP
 * @retval uint8_t          : Error = 0, Success = TCP ACK number
 *****************************************************************/
tcp_ctl_flags_t ether_get_tcp_server_ack(ethernet_handle_t *ethernet,  uint32_t *sequence_number, uint32_t *ack_number,
                                         uint16_t server_src_port, uint16_t client_src_port, uint8_t *sever_ip)
{

    tcp_ctl_flags_t func_retval = (tcp_ctl_flags_t)0;

    net_ip_t  *ip;
    net_tcp_t *tcp;

    uint8_t validate = 0;

    uint16_t sender_src_port  = 0;
    uint16_t sender_dest_port = 0;

    if(ethernet->ether_obj == NULL || sever_ip == NULL)
    {
        func_retval = (tcp_ctl_flags_t)0;
    }
    else
    {
        ip  = (void*)&ethernet->ether_obj->data;

        tcp = (void*)( (uint8_t*)ip + IP_HEADER_SIZE );

        validate = validate_tcp_checksum(ip, tcp);

        if(validate && ( memcmp(sever_ip, ip->source_ip, 4) == 0 ) )
        {
            sender_src_port  = ntohs(tcp->source_port);
            sender_dest_port = ntohs(tcp->destination_port);

            if(server_src_port == sender_src_port && client_src_port == sender_dest_port)
            {
                *sequence_number = ntohl(tcp->sequence_number);
                *ack_number      = ntohl(tcp->ack_number);

                func_retval = (tcp_ctl_flags_t)tcp->control_bits;
            }

        }
    }

    return func_retval;
}





uint8_t ether_send_tcp_ack(ethernet_handle_t *ethernet, uint16_t source_port, uint16_t destination_port,
                           uint32_t sequence_number, uint32_t ack_number, uint8_t *destination_ip, tcp_ctl_flags_t ack_type)
{

    net_ip_t  *ip;
    net_tcp_t *tcp;

    /* Ethernet Frame related variables */
    uint8_t  destination_mac[ETHER_MAC_SIZE] = {0};

    ip  = (void*)&ethernet->ether_obj->data;

    tcp = (void*)( (uint8_t*)ip + IP_HEADER_SIZE );


    /* Fill TCP frame */
    tcp->source_port      = htons(source_port);
    tcp->destination_port = htons(destination_port);

    tcp->sequence_number  = htonl(sequence_number);
    tcp->ack_number       = htonl(ack_number);

    /* Shift data offset to Big-Endian MSB (4 bits) */
    tcp->data_offset      = ((TCP_FRAME_SIZE) >> 2) << 4;
    tcp->control_bits     = (uint8_t)ack_type;

    tcp->window           = ntohs(1);
    tcp->urgent_pointer   = 0;


    /* Fill IP frame before TCP checksum calculation */
    fill_ip_frame(ip, &ethernet->ip_identifier, destination_ip, ethernet->host_ip, IP_TCP, TCP_FRAME_SIZE);


    /*Get TCP checksum */
    tcp->checksum = get_tcp_checksum(ip, tcp, 0);


    /* Get MAC address from ARP table */
    ether_arp_resolve_address(ethernet, destination_mac, destination_ip);


    /* Fill Ethernet frame */
    fill_ether_frame(ethernet, destination_mac, ethernet->host_mac, ETHER_IPV4);


    /*Send TCP data */
    ether_send_data(ethernet,(uint8_t*)ethernet->ether_obj, ETHER_FRAME_SIZE + htons(ip->total_length));


    return 0;
}




uint16_t ether_get_tcp_psh_ack(ethernet_handle_t *ethernet, char *tcp_data, uint16_t data_buffer_length)
{
    uint16_t func_retval = 0;


    net_ip_t  *ip;
    net_tcp_t *tcp;

    uint16_t tcp_packet_length = 0;
    uint16_t tcp_data_length   = 0;

    ip  = (void*)&ethernet->ether_obj->data;

    tcp = (void*)( (uint8_t*)ip + IP_HEADER_SIZE );

    tcp_packet_length = ( ntohs(ip->total_length) - IP_HEADER_SIZE );

    tcp_data_length = abs(tcp_packet_length - TCP_FRAME_SIZE);

    if(data_buffer_length > tcp_data_length)
        data_buffer_length = tcp_data_length;


    memcpy(tcp_data, &tcp->data, data_buffer_length);


    func_retval = tcp_data_length;

    return func_retval;
}






int8_t ether_send_tcp_psh_ack(ethernet_handle_t *ethernet, uint16_t source_port, uint16_t destination_port,
                              uint32_t sequence_number, uint32_t ack_number, uint8_t *destination_ip, char *tcp_data, uint16_t data_length)
{
    int8_t func_retval = 0;

    net_ip_t  *ip;
    net_tcp_t *tcp;

    uint8_t *data_copy;

    uint16_t index = 0;

    /* Ethernet Frame related variables */
    uint8_t  destination_mac[ETHER_MAC_SIZE] = {0};

    ip  = (void*)&ethernet->ether_obj->data;

    tcp = (void*)( (uint8_t*)ip + IP_HEADER_SIZE );


    /* Fill TCP frame */
    tcp->source_port      = htons(source_port);
    tcp->destination_port = htons(destination_port);

    tcp->sequence_number  = htonl(sequence_number);
    tcp->ack_number       = htonl(ack_number);

    /* Shift data offset to Big-Endian MSB (4 bits) */
    tcp->data_offset      = ((TCP_FRAME_SIZE) >> 2) << 4;
    tcp->control_bits     = (uint8_t)(TCP_PSH_ACK);

    tcp->window           = ntohs(1);
    tcp->urgent_pointer   = 0;

    /* Copy TCP data */
    data_copy = &tcp->data;


    for(index = 0; index < data_length; index++)
    {
        data_copy[index] = (uint8_t)(tcp_data[index]);
    }


    /* fill IP frame before TCP checksum calculation */
    fill_ip_frame(ip, &ethernet->ip_identifier, destination_ip, ethernet->host_ip, IP_TCP, TCP_FRAME_SIZE + data_length);

    /*Get TCP checksum */
    tcp->checksum = get_tcp_checksum(ip, tcp, data_length);


    /* Get MAC address from ARP table */
    ether_arp_resolve_address(ethernet, destination_mac, destination_ip);


    /* Fill Ethernet frame */
    fill_ether_frame(ethernet, destination_mac, ethernet->host_mac, ETHER_IPV4);


    /*Send TCP data */
    ether_send_data(ethernet,(uint8_t*)ethernet->ether_obj, ETHER_FRAME_SIZE + htons(ip->total_length));


    return func_retval;
}



uint8_t ether_is_tcp(ethernet_handle_t *ethernet, uint8_t *network_data, uint16_t network_data_length)
{
    uint8_t func_retval = 0;
    uint8_t block_loop  = 0;

    int16_t comm_type = 0;

    if(ethernet->ether_obj == NULL || network_data == NULL || network_data_length == 0 || network_data_length > UINT16_MAX)
    {
        func_retval = 0;
    }
    else
    {

        /* Wait for data */
        block_loop = ethernet->status.mode_read_blocking;

        do
        {
            if(ether_get_data(ethernet, network_data, network_data_length))
            {
                /* Check if protocol is IPV4 */
                if(get_ether_protocol_type(ethernet) == ETHER_IPV4)
                {

                    /* Checks if UNICAST, validates checksum */
                    comm_type = get_ip_communication_type(ethernet);

                    /* UNICAST */
                    if(comm_type == 1)
                    {
                        /* Check if protocol is TCP */
                        if(get_ip_protocol_type(ethernet) == IP_TCP)
                        {
                            func_retval = 1;

                            break;
                        }

                    }

                }

            }

        }while(block_loop);

    }

    return func_retval;
}



uint8_t init_tcp_client(tcp_client_t *client, uint16_t source_port, uint16_t destination_port, uint8_t *server_ip)
{

    uint8_t func_retval = 0;

    client->source_port      = source_port;
    client->destination_port = destination_port;

    client->sequence_number        = 0;
    client->acknowledgement_number = 0;

    client->client_flags.client_blocking = 1;

    memcpy((char*)client->server_ip, (char*)server_ip, ETHER_IPV4_SIZE);

    return func_retval;
}


tcp_client_t* tcp_create_client(uint16_t source_port, uint16_t destination_port, uint8_t *server_ip)
{

    static tcp_client_t tcp_client;

    tcp_client.source_port      = source_port;
    tcp_client.destination_port = destination_port;

    tcp_client.sequence_number        = 0;
    tcp_client.acknowledgement_number = 0;

    tcp_client.client_flags.client_blocking = 1;

    memcpy((char*)tcp_client.server_ip, (char*)server_ip, ETHER_IPV4_SIZE);

    return &tcp_client;
}




int8_t ether_tcp_handshake(ethernet_handle_t *ethernet, uint8_t *network_data ,tcp_client_t *client)
{
    int8_t func_retval = 0;

    uint8_t api_retval  = 0;

    uint8_t tcp_read_loop = 1;

    tcp_ctl_flags_t ack_type;

    /* Send TCP SYN packet */
    ether_send_tcp_syn(ethernet, client->source_port, client->destination_port, client->sequence_number,
                       client->acknowledgement_number, client->server_ip);

    client->client_flags.connect_request = 1;

    /* Read response message from the TCP server */
    api_retval = ether_is_tcp(ethernet, network_data, ETHER_MTU_SIZE);

    if(api_retval)
    {
        do
        {
            ack_type = ether_get_tcp_server_ack(ethernet, &client->sequence_number, &client->acknowledgement_number,
                                                client->destination_port, client->source_port, client->server_ip);

            switch(ack_type)
            {

            case TCP_SYN_ACK:

                /* Increment the sequence number and pass it as acknowledgment number*/
                client->sequence_number += 1;

                ether_send_tcp_ack(ethernet, client->source_port, client->destination_port,
                                   client->acknowledgement_number, client->sequence_number, client->server_ip, TCP_ACK);

                /* Set flags */
                client->client_flags.connect_request     = 0;
                client->client_flags.connect_established = 1;

                tcp_read_loop = 0;

                break;


            case TCP_FIN_ACK:

                /* Increment the sequence number and pass it as acknowledgment number*/
                client->sequence_number += 1;

                ether_send_tcp_ack(ethernet, client->source_port, client->destination_port,
                                   client->acknowledgement_number, client->sequence_number, client->server_ip, TCP_FIN_ACK);

                tcp_read_loop = 0;

                break;


            case TCP_RST_ACK:

                tcp_read_loop = 0;

                /* Start retransmission timer */

                break;


            default:

                tcp_read_loop = 0;

                break;

            }

        }while(client->client_flags.client_blocking == 1 && tcp_read_loop);

    }

    return func_retval;
}




int8_t ether_send_tcp_data(ethernet_handle_t *ethernet, uint8_t *network_data, tcp_client_t *client, char *application_data,
                           uint16_t data_length)
{
    int8_t func_retval = 0;

    tcp_ctl_flags_t ack_type;

    uint8_t tcp_read_loop = 1;

    while(tcp_read_loop && ether_get_data(ethernet, network_data, ETHER_MTU_SIZE))
    {
        if(get_ether_protocol_type(ethernet) == ETHER_IPV4 && (get_ip_communication_type(ethernet) == 1) \
                && get_ip_protocol_type(ethernet) == IP_TCP)
        {

            /* Read ACK from the TCP server */
            ack_type = ether_get_tcp_server_ack(ethernet, &client->sequence_number, &client->acknowledgement_number,
                                                client->destination_port, client->source_port, client->server_ip);

            switch(ack_type)
            {
            case TCP_FIN_ACK:

                /* Increment the sequence number and pass it as acknowledgment number*/
                client->sequence_number += 1;

                ether_send_tcp_ack(ethernet, client->source_port, client->destination_port, client->acknowledgement_number,
                                   client->sequence_number, ethernet->gateway_ip, TCP_FIN_ACK);

                tcp_read_loop = 0;

                func_retval = -1;

                break;


            default:

                tcp_read_loop = 0;

                break;

            }

        }
        else if(get_ether_protocol_type(ethernet) == ETHER_IPV4 && (get_ip_communication_type(ethernet) == 1) \
                && get_ip_protocol_type(ethernet) == IP_ICMP)
        {
            ether_send_icmp_reply(ethernet);

            break;
        }

    }

    /* Don't send PSH ACK packet if server has terminated connection */
    if(func_retval >= 0)
    {
        /* Send PSH ACK packet to the server (SEQ and ACK numbers swapped) */
        ether_send_tcp_psh_ack(ethernet, client->source_port, client->destination_port,
                               client->acknowledgement_number, client->sequence_number, client->server_ip, application_data, data_length);
    }

    return func_retval;
}





int16_t ether_read_tcp_data(ethernet_handle_t *ethernet, uint8_t *network_data, tcp_client_t *client,
                            char *application_data, uint16_t data_length)
{
    int16_t func_retval = 0;

    uint8_t api_retval = 0;

    uint8_t tcp_read_loop = 1;

    tcp_ctl_flags_t ack_type;

    uint16_t tcp_data_length = 0;


    while(tcp_read_loop && ether_get_data(ethernet, network_data, ETHER_MTU_SIZE))
    {
        /* Handle ARP requests */
        if(get_ether_protocol_type(ethernet) == ETHER_ARP)
        {

            ether_handle_arp_resp_req(ethernet);

        }
        else if(get_ether_protocol_type(ethernet) == ETHER_IPV4)
        {
            /* Checks if UNICAST, validates checksum */
            api_retval = get_ip_communication_type(ethernet);

            /* Get transport layer protocol type */
            if(api_retval == 1)
            {
                /* Handle ICMP packets */
                if(get_ip_protocol_type(ethernet) == IP_ICMP)
                {

                    ether_send_icmp_reply(ethernet);

                }
                /* Handle TCP packets */
                else if(get_ip_protocol_type(ethernet) == IP_TCP)
                {

                    /* Read ACK from the TCP server */
                    ack_type = ether_get_tcp_server_ack(ethernet, &client->sequence_number, &client->acknowledgement_number,
                                                        client->destination_port, client->source_port, client->server_ip);

                    switch(ack_type)
                    {

                    case TCP_PSH_ACK:

                        tcp_data_length = ether_get_tcp_psh_ack(ethernet, application_data, data_length);

                        client->sequence_number += tcp_data_length;

                        ether_send_tcp_ack(ethernet, client->source_port, client->destination_port, client->acknowledgement_number,
                                           client->sequence_number, ethernet->gateway_ip, TCP_ACK);

                        tcp_read_loop = 0;

                        func_retval = tcp_data_length;

                        break;


                    case TCP_FIN_ACK:

                        /* Increment the sequence number and pass it as acknowledgment number*/
                        client->sequence_number += 1;

                        ether_send_tcp_ack(ethernet, client->source_port, client->destination_port, client->acknowledgement_number,
                                           client->sequence_number, ethernet->gateway_ip, TCP_FIN_ACK);

                        tcp_read_loop = 0;

                        func_retval = -1;

                        break;

                    case TCP_ACK:


                        break;


                    default:

                        /* NOP */

                        break;

                    }

                    break;

                } /* IP is TCP condition */

            } /* UNICAST Packets */

        } /* ETHER is IP packet condition */

    }/* while loop */

    memset(network_data, 0, sizeof(ETHER_MTU_SIZE));

    return func_retval;
}



