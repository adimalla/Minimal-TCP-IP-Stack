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


#define TCP_FRAME_SIZE 20
#define TCP_SYN_OPTS_SIZE 12

/**/
typedef struct _net_tcp
{
    uint16_t source_port;      /*!< TCP source port                 */
    uint16_t destination_port; /*!< TCP destination port            */
    uint32_t sequence_number;  /*!< TCP packet sequence number      */
    uint32_t ack_number;       /*!< TCP acknowledgment number       */
    uint8_t  data_offset;      /*!< TCP data offset (header length) */
    uint8_t  control_bits;     /*!< TCP control flags               */
    uint16_t window;           /*!< TCP window size                 */
    uint16_t checksum;         /*!< TCP checksum                    */
    uint16_t urgent_pointer;   /*!< TCP urgent pointer              */
    uint8_t  data;             /*!< TCP data, also contains options */

}net_tcp_t;


/* TCP Option value */
typedef enum _tcp_option_kinds
{
    TCP_NO_OPERATION     = 1,  /*!< No operation option   */
    TCP_MAX_SEGMENT_SIZE = 2,  /*!< MAX segment size      */
    TCP_WINDOW_SCALING   = 3,  /*!< Window scaling offset */
    TCP_SACK_PERMITTED   = 4,  /*!< */
    TCP_TIMESTAMPS       = 8,  /*!< TCP time stamp        */

}tcp_opts_kind;


/**/
typedef struct tcp_max_segment_size
{
    uint8_t  option_kind;  /*!< */
    uint8_t  length;       /*!< */
    uint16_t value;        /*!< */

}tcp_mss_t;


/**/
typedef struct tcp_sack_permitted_size
{
    uint8_t option_kind;  /*!< */
    uint8_t length;       /*!< */

}tcp_sack_t;


/**/
typedef struct tcp_no_operation
{
    uint8_t option_kind;  /*!< */

}tcp_nop_t;


/**/
typedef struct tcp_window_scaling
{
    uint8_t option_kind;  /*!< */
    uint8_t length;       /*!< */
    uint8_t value;        /*!< */

}tcp_win_scale_t;


/**/
typedef struct _tcp_syn_options
{
    tcp_mss_t       mss;           /*!< */
    tcp_sack_t      sack;          /*!< */
    tcp_nop_t       nop;           /*!< */
    tcp_nop_t       nop1;          /*!< */
    tcp_nop_t       nop2;          /*!< */
    tcp_win_scale_t window_scale;  /*!< */

}tcp_syn_opts_t;




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



/***************************************************************
 * @brief  Static function to check if Packet is TCP (UNICAST)
 * @param  *ethernet           : Reference to Ethernet handle
 * @param  *network_data       : Network data
 * @param  network_data_length : network data length
 * @retval int8_t              : Error = 0, Success = 1
 ***************************************************************/
static uint8_t ether_is_tcp(ethernet_handle_t *ethernet, uint8_t *network_data, uint16_t network_data_length)
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
            /* Get network data and check if protocol is IPV4 */
            if(ether_get_data(ethernet, network_data, network_data_length) && (get_ether_protocol_type(ethernet) == ETHER_IPV4))
            {
                /* Checks if UNICAST, validates checksum */
                comm_type = get_ip_communication_type(ethernet);

                /* UNICAST and if protocol is TCP */
                if(comm_type == 1 && (get_ip_protocol_type(ethernet) == IP_TCP))
                {

                    func_retval = 1;

                    break;

                }

            }

        }while(block_loop);

    }

    return func_retval;
}



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

        func_retval = 1;

    }

    return func_retval;
}




/*****************************************************************
 * @brief  Function for getting TCP sever ACK packets
 *         (validates TCP checksum)
 * @param  *ethernet        : Reference to Ethernet handle
 * @param  *sequence_number : Reference to TCP sequence number
 * @param  *ack_number      : Reference to acknowledgment number
 * @param  server_src_port  : TCP destination /server port
 * @param  client_src_port  : TCP client source port
 * @param  *sever_ip        : Destination server IP
 * @retval uint8_t          : Error = 0, Success = TCP ACK number
 *****************************************************************/
static tcp_ctl_flags_t ether_get_tcp_server_ack(ethernet_handle_t *ethernet,  uint32_t *sequence_number, uint32_t *ack_number,
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




/**********************************************************
 * @brief  Function for sending TCP ACK packet
 *         sequence number and ACK number are swapped,
 *         then passed as parameters by user.
 * @param  *ethernet        : Reference to Ethernet handle
 * @param  source_port      : TCP source port
 * @param  destination_port : TCP destination port
 * @param  sequence_number  : TCP sequence number
 * @param  ack_number       : TCP acknowledgment number
 * @param  *destination_ip  : Destination server IP
 * @param  ack_type         : TCP ACK value
 * @retval int8_t           : Error = 0, Success = 1
 **********************************************************/
static int8_t ether_send_tcp_ack(ethernet_handle_t *ethernet, uint16_t source_port, uint16_t destination_port,
                          uint32_t sequence_number, uint32_t ack_number, uint8_t *destination_ip, tcp_ctl_flags_t ack_type)
{

    int8_t func_retval = 0;

    net_ip_t  *ip;
    net_tcp_t *tcp;

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

        /* Shift data offset to Big-endian MSB (4 bits) */
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


        func_retval = 1;

    }

    return func_retval;
}




/***************************************************************
 * @brief  Function to get PSH ACK packet (TCP data packet)
 * @param  *ethernet          : Reference to Ethernet handle
 * @param  *tcp_data          : TCP data / payload
 * @param  data_buffer_length : Data buffer length
 * @retval int8_t             : Error = 0, Success = bytes read
 ***************************************************************/
static uint16_t ether_get_tcp_psh_ack(ethernet_handle_t *ethernet, char *tcp_data, uint16_t data_buffer_length)
{
    uint16_t func_retval = 0;

    net_ip_t  *ip;
    net_tcp_t *tcp;

    uint16_t tcp_packet_length = 0;
    uint16_t tcp_data_length   = 0;

    if(ethernet->ether_obj == NULL || tcp_data == NULL || data_buffer_length > UINT16_MAX)
    {
        func_retval = 0;
    }
    else
    {
        ip  = (void*)&ethernet->ether_obj->data;

        tcp = (void*)( (uint8_t*)ip + IP_HEADER_SIZE );

        tcp_packet_length = ( ntohs(ip->total_length) - IP_HEADER_SIZE );

        tcp_data_length = abs(tcp_packet_length - TCP_FRAME_SIZE);

        if(data_buffer_length > tcp_data_length)
            data_buffer_length = tcp_data_length;

        memcpy(tcp_data, &tcp->data, data_buffer_length);

        func_retval = tcp_data_length;
    }

    return func_retval;
}





/****************************************************************
 * @brief  Function for sending TCP PSH ACK packet (data packet)
 *         sequence number and ACK number are swapped,
 *         then passed as parameters by user.
 * @param  *ethernet        : Reference to Ethernet handle
 * @param  source_port      : TCP source port
 * @param  destination_port : TCP destination port
 * @param  sequence_number  : TCP sequence number
 * @param  ack_number       : TCP acknowledgment number
 * @param  *destination_ip  : Destination server IP
 * @param  *tcp_data        : TCP data / payload
 * @param  data_length      : TCP data length
 * @retval int8_t           : Error = 0, Success = 1
 ****************************************************************/
static int8_t ether_send_tcp_psh_ack(ethernet_handle_t *ethernet, uint16_t source_port, uint16_t destination_port,
                              uint32_t sequence_number, uint32_t ack_number, uint8_t *destination_ip,
                              char *tcp_data, uint16_t data_length)
{
    int8_t func_retval = 0;

    net_ip_t  *ip;
    net_tcp_t *tcp;

    uint8_t *data_copy;

    uint16_t index = 0;

    /* Ethernet Frame related variables */
    uint8_t  destination_mac[ETHER_MAC_SIZE] = {0};


    if(ethernet->ether_obj == NULL || destination_ip == NULL || data_length > ETHER_MTU_SIZE || data_length > UINT16_MAX)
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

        func_retval = 1;

    }

    return func_retval;
}





/************************************************************************
 * @brief  helper function for reading TCP data
 * @param  *ethernet         : Reference to the Ethernet Handle
 * @param  *network_data     : Network data
 * @param  *client           : Reference to TCP client handle
 * @param  *application_data : application_data
 * @param  data_length       : application data length
 * @retval uint16_t          : Error = 0, Success = number of bytes read
 *                                              1 = ACK received
 ************************************************************************/
static int32_t ether_tcp_read_data_hf(ethernet_handle_t *ethernet, uint8_t *network_data, tcp_handle_t *client,
                            char *application_data, uint16_t data_length)
{
    int32_t func_retval = NET_FUNC_NO_RDWR;

    uint8_t tcp_read_loop = 0;

    tcp_ctl_flags_t ack_type;

    uint16_t tcp_data_length = 0;


    if(ethernet->ether_obj == NULL || client == NULL || data_length > UINT16_MAX || data_length > ETHER_MTU_SIZE)
    {
        func_retval = NET_TCP_READ_ERROR;
    }
    else
    {

        tcp_read_loop = 1;

        while(tcp_read_loop)
        {
            if(ether_get_data(ethernet, network_data, ETHER_MTU_SIZE) && client->client_flags.connect_established == 1)
            {

                /* Handle ARP requests */
                if(get_ether_protocol_type(ethernet) == ETHER_ARP)
                {

                    ether_handle_arp_resp_req(ethernet);

                }
                /* handle transport layer protocol type packets */
                else if(get_ether_protocol_type(ethernet) == ETHER_IPV4 && (get_ip_communication_type(ethernet) == 1))
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


                        case TCP_ACK:

                            func_retval = 1;

                            break;


                        case TCP_PSH_ACK:

                            tcp_data_length = ether_get_tcp_psh_ack(ethernet, application_data, data_length);

                            client->sequence_number += tcp_data_length;

                            ether_send_tcp_ack(ethernet, client->source_port, client->destination_port, client->acknowledgement_number,
                                               client->sequence_number, client->server_ip, TCP_ACK);

                            tcp_read_loop = 0;

                            func_retval = tcp_data_length;

                            break;


                        case TCP_FIN_ACK:

                            /* Increment the sequence number and pass it as acknowledgment number*/
                            client->sequence_number += 1;

                            ether_send_tcp_ack(ethernet, client->source_port, client->destination_port, client->acknowledgement_number,
                                               client->sequence_number, client->server_ip, TCP_FIN_ACK);

                            tcp_read_loop = 0;

                            client->client_flags.server_close = 1;
                            client->client_flags.connect_established = 0;

                            func_retval = 0;

                            break;


                        case TCP_FIN_PSH_ACK:

                            /* Increment thetcp_read_loop = 0; sequence number and pass it as acknowledgment number*/
                            client->sequence_number += 1;

                            /* not handled correctly*/
                            ether_send_tcp_ack(ethernet, client->source_port, client->destination_port, client->acknowledgement_number,
                                               client->sequence_number, client->server_ip, TCP_FIN_ACK);

                            tcp_read_loop = 0;

                            client->client_flags.server_close = 1;
                            client->client_flags.connect_established = 0;

                            func_retval = 0;

                            break;


                        default:

                            /* NOP */

                            break;

                        }


                    } /* IP is TCP condition */

                } /* ETHER is IP packet condition */
            }

            //memset(network_data, 0, sizeof(ETHER_MTU_SIZE));

            tcp_read_loop = client->client_flags.client_blocking;

        }/* while loop */

    }

    return func_retval;
}





/******************************************************************************/
/*                                                                            */
/*                               TCP Functions                                */
/*                                                                            */
/******************************************************************************/




/********************************************************************
 * @brief  Function to create TCP client object (STATIC)
 * @param  source_port      : TCP source port
 * @param  destination_port : TCP destination port
 * @param  *server_ip       : Server IP
 * @retval int8_t           : Error = 0, Success = TCP client object
 ********************************************************************/
tcp_handle_t* ether_tcp_create_client(ethernet_handle_t *ethernet, uint8_t *network_data, uint16_t source_port,
                                      uint16_t destination_port, uint8_t *server_ip)
{

    static tcp_handle_t tcp_client;

    if(server_ip == NULL)
    {
        return NULL;
    }
    else
    {
        tcp_client.source_port      = source_port;
        tcp_client.destination_port = destination_port;

        tcp_client.sequence_number        = 0;
        tcp_client.acknowledgement_number = 0;

        /* Not tested */
        tcp_client.client_flags.client_blocking = 1;

        memcpy((char*)tcp_client.server_ip, (char*)server_ip, ETHER_IPV4_SIZE);

        ether_send_arp_req(ethernet, ethernet->host_ip, server_ip);

        if(ether_is_arp(ethernet, network_data, 60))
        {
            ether_handle_arp_resp_req(ethernet);
        }

    }

    return &tcp_client;
}




/*****************************************************************
 * @brief  Function to initialize TCP values to TCP client object
 * @param  *client          : Reference to TCP client handle
 * @param  source_port      : TCP source port
 * @param  destination_port : TCP destination port
 * @param  *server_ip       : Server IP
 * @retval int8_t           : Error = 0, Success = 1
 *****************************************************************/
uint8_t tcp_init_client(tcp_handle_t *client, uint16_t source_port, uint16_t destination_port, uint8_t *server_ip)
{

    uint8_t func_retval = 0;

    if(server_ip == NULL)
    {
        func_retval = 0;
    }
    else
    {

        client->source_port      = source_port;
        client->destination_port = destination_port;

        client->sequence_number        = 0;
        client->acknowledgement_number = 0;

        /* Not tested */
        client->client_flags.client_blocking = 1;

        memcpy(client->server_ip, server_ip, ETHER_IPV4_SIZE);

    }
    return func_retval;
}





/**********************************************************
 * @brief  Function to establish connection to TCP server
 * @param  *ethernet     : Reference to Ethernet handle
 * @param  *network_data : Network data
 * @param  *client       : reference to TCP client handle
 * @retval int8_t        : Error = -11, Success = 1
 **********************************************************/
int8_t ether_tcp_connect(ethernet_handle_t *ethernet, uint8_t *network_data ,tcp_handle_t *client)
{
    int8_t func_retval = 0;
    uint8_t api_retval = 0;

    uint8_t tcp_read_loop = 1;

    tcp_ctl_flags_t ack_type;

    if(ethernet->ether_obj == NULL || client == NULL)
    {
        func_retval = NET_TCP_CONNECT_ERROR;
    }
    else
    {

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

                    func_retval = 1;

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

                    /* TODO Implementation pending */

                    /* Start retransmission timer */
                    //tcp_read_loop = 0;

                    break;


                default:


                    break;

                }

            }while(client->client_flags.client_blocking == 1 && tcp_read_loop);

        }
    }

    return func_retval;
}




/*****************************************************************
 * @brief  Function to control TCP read behavior
 * @param  *client     : Reference to TCP handle
 * @param  app_state   : TCP read type (blocking or non blocking)
 * @retval int8_t      : Error = 0, Success = 1
 ****************************************************************/
int8_t tcp_control(tcp_handle_t *client, tcp_read_state_t app_state)
{
    int8_t func_retval = 0;

    if(client == NULL || app_state > 2 || app_state < 1)
    {
        func_retval = 0;
    }
    else
    {
        if(app_state == TCP_READ_NONBLOCK)
            client->client_flags.client_blocking = 0;

        func_retval = 1;
    }

    return func_retval;
}



/***************************************************************
 * @brief  Function for sending TCP data
 * @param  *ethernet         : Reference to the Ethernet Handle
 * @param  *network_data     : Network data
 * @param  *client           : Reference to TCP client handle
 * @param  *application_data : application_data
 * @param  data_length       : application data length
 * @retval int8_t            : Error   = -12,
 *                             Success =  1
 *                                       -14(Connection closed)
 ***************************************************************/
int32_t ether_tcp_send_data(ethernet_handle_t *ethernet, uint8_t *network_data, tcp_handle_t *client, char *application_data,
                              uint16_t data_length)
{
    int32_t func_retval = NET_FUNC_NO_RDWR;

    tcp_ctl_flags_t ack_type;

    uint16_t tcp_data_length = 0;

    uint8_t tcp_read_loop = 0;


    if(ethernet->ether_obj == NULL || client == NULL || data_length > UINT16_MAX || data_length > ETHER_MTU_SIZE)
    {
        func_retval = NET_TCP_SEND_ERROR;
    }
    else if(client->client_flags.connect_established == 0)
    {
        func_retval = 0;
    }
    else
    {

        if(client->client_flags.connect_established == 1)
        {
            /* Send PSH ACK packet to the server (SEQ and ACK numbers swapped) */
            ether_send_tcp_psh_ack(ethernet, client->source_port, client->destination_port,client->acknowledgement_number,
                                   client->sequence_number, client->server_ip, application_data, data_length);

            func_retval = 1;
        }


        tcp_read_loop = 1;

        do
        {
            if(ether_get_data(ethernet, network_data, ETHER_MTU_SIZE))
            {

                /* Handle ARP requests */
                if(get_ether_protocol_type(ethernet) == ETHER_ARP)
                {

                    ether_handle_arp_resp_req(ethernet);

                }
                /* handle transport layer protocol type packets */
                else if(get_ether_protocol_type(ethernet) == ETHER_IPV4 && (get_ip_communication_type(ethernet) == 1))
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

                        case TCP_ACK:

                            tcp_read_loop = 0;
                            func_retval   = 1;

                            break;


                        case TCP_PSH_ACK:

                            /* Read PSH ACK */
                            tcp_data_length = ether_get_tcp_psh_ack(ethernet, ethernet->net_application_data, data_length);

                            ethernet->status.net_app_data_rdy = 1;

                            /* Increment the sequence number and pass it as acknowledgment number*/
                            client->sequence_number += tcp_data_length;

                            /* Send ACK */
                            ether_send_tcp_ack(ethernet, client->source_port, client->destination_port, client->acknowledgement_number,
                                               client->sequence_number, client->server_ip, TCP_ACK);

                            tcp_read_loop = 0;
                            func_retval   = 2;

                            ethernet->net_app_data_length = tcp_data_length;

                            func_retval = tcp_data_length;

                            break;


                        case TCP_FIN_ACK:

                            /* Increment the sequence number and pass it as acknowledgment number*/
                            client->sequence_number += 1;

                            /* Send FIN ACK */
                            ether_send_tcp_ack(ethernet, client->source_port, client->destination_port, client->acknowledgement_number,
                                               client->sequence_number, client->server_ip, TCP_FIN_ACK);

                            client->client_flags.server_close = 1;
                            client->client_flags.connect_established = 0;

                            func_retval =  0;

                            /* Read ACK in next iteration */

                            break;


                        case TCP_FIN_PSH_ACK:

                            /* Increment the sequence number and pass it as acknowledgment number*/
                            client->sequence_number += 1;

                            /* not handled correctly*/
                            ether_send_tcp_ack(ethernet, client->source_port, client->destination_port, client->acknowledgement_number,
                                               client->sequence_number, client->server_ip, TCP_FIN_ACK);

                            client->client_flags.server_close = 1;
                            client->client_flags.connect_established = 0;

                            func_retval   =  0;

                            break;


                        default:

                            /* NOP */

                            break;

                        }

                    } /* IP is TCP condition */

                } /* ETHER is IP packet condition */
            }

        }while(tcp_read_loop);/* while loop */

    }

    return func_retval;
}





/************************************************************************
 * @brief  Function for reading TCP data
 * @param  *ethernet         : Reference to the Ethernet Handle
 * @param  *network_data     : Network data
 * @param  *client           : Reference to TCP client handle
 * @param  *application_data : application_data
 * @param  data_length       : application data length
 * @retval uint16_t          : Error = 0, Success = number of bytes read
 *                                              1 = ACK received
 ************************************************************************/
int32_t ether_tcp_read_data(ethernet_handle_t *ethernet, uint8_t *network_data, tcp_handle_t *client, char *tcp_data, uint16_t data_length)
{
    int32_t func_retval      = NET_FUNC_NO_RDWR;
    uint16_t tcp_data_length = 0;

    if(ethernet->ether_obj == NULL || client == NULL || data_length > UINT16_MAX || data_length > ETHER_MTU_SIZE)
    {
        func_retval = NET_TCP_READ_ERROR;
    }
    else if(client->client_flags.connect_established == 0)
    {
        func_retval = 0;
    }
    else
    {

        if(ethernet->status.net_app_data_rdy == 1)
        {

            /* Check if user data buffer length size is not greater than network application data size */
            if(data_length > ethernet->net_app_data_length)
                data_length = ethernet->net_app_data_length;

            /* Copy data to user buffer */
            memcpy(tcp_data, ethernet->net_application_data, data_length);

            /* Specify available data size from data read by network */
            tcp_data_length = ethernet->net_app_data_length;

            /* Clear network application buffer and length */
            memset(ethernet->net_application_data, 0, ethernet->net_app_data_length);

            ethernet->net_app_data_length = 0;

            /* Clear associated flags */
            ethernet->status.net_app_data_rdy = 0;

            func_retval = tcp_data_length;

        }
        else
        {
            func_retval = ether_tcp_read_data_hf(ethernet, network_data, client, tcp_data, data_length);
        }
    }

    return func_retval;
}





/***************************************************************
 * @brief  Function for close socket
 * @param  *ethernet         : Reference to the Ethernet Handle
 * @param  *network_data     : Network data
 * @param  *client           : Reference to TCP handle
 * @retval uint16_t          : Error = 0, Success = 1;
 ***************************************************************/
uint8_t ether_tcp_close(ethernet_handle_t *ethernet, uint8_t *network_data, tcp_handle_t *client)
{

    uint8_t func_retval = 0;

    tcp_ctl_flags_t ack_type;
    uint8_t tcp_read_loop = 1;

    if(ethernet->ether_obj == NULL || client == NULL)
    {
        func_retval = 0;
    }
    else
    {

        while(tcp_read_loop)
        {

            if(ether_get_data(ethernet, network_data, ETHER_MTU_SIZE) && \
                    get_ether_protocol_type(ethernet) == ETHER_IPV4 && (get_ip_communication_type(ethernet) == 1))
            {
                /* Read ACK from the TCP server */
                ack_type = ether_get_tcp_server_ack(ethernet, &client->sequence_number, &client->acknowledgement_number,
                                                    client->destination_port, client->source_port, client->server_ip);

                if(ack_type == TCP_FIN_ACK)
                {

                    /* Increment the sequence number and pass it as acknowledgment number*/
                    client->sequence_number += 1;

                    ether_send_tcp_ack(ethernet, client->source_port, client->destination_port, client->acknowledgement_number,
                                       client->sequence_number, ethernet->gateway_ip, TCP_ACK);


                    client->client_flags.server_close = 1;
                    client->client_flags.connect_established = 0;

                    tcp_read_loop = 0;

                    memset(client, 0, sizeof(tcp_handle_t));
                }

            }

            if(client->client_flags.connect_established == 1)
            {
                /* Send PSH ACK packet to the server (SEQ and ACK numbers swapped) */
                ether_send_tcp_ack(ethernet, client->source_port, client->destination_port, client->acknowledgement_number,
                                   client->sequence_number, ethernet->gateway_ip, TCP_FIN_ACK);

                client->client_flags.client_close = 1;

                client->client_flags.connect_established = 0;

                tcp_read_loop = 0;

                func_retval = 1;
            }

        }
    }

    return func_retval;
}


