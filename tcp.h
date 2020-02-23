/**
 ******************************************************************************
 * @file    TCP.h
 * @author  Aditya Mall,
 * @brief   TCP protocol header file
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




#ifndef TCP_H_
#define TCP_H_




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



/* TCP ACK flags */
typedef enum _tcp_control_flags
{

    TCP_FIN     = 0x01,
    TCP_SYN     = 0x02,
    TCP_RST     = 0x04,
    TCP_PSH     = 0x08,
    TCP_ACK     = 0x10,
    TCP_FIN_ACK = 0x11,
    TCP_SYN_ACK = 0x12,
    TCP_RST_ACK = 0x14,
    TCP_PSH_ACK = 0x18,

}tcp_ctl_flags_t;


/* TCP client handle flags */
typedef struct _tcp_client_flags
{
    uint8_t connect_request     : 1;
    uint8_t connect_established : 1;
    uint8_t server_tcp_reset    : 1;
    uint8_t server_close        : 1;
    uint8_t client_close        : 1;
    uint8_t client_blocking     : 1;
    uint8_t reserved            : 2;

}tcp_client_flags_t;


/* TCP client handle */
typedef struct _tcp_client
{
    uint16_t source_port;
    uint16_t destination_port;
    uint32_t sequence_number;
    uint32_t acknowledgement_number;
    uint8_t  server_ip[4];

    tcp_client_flags_t client_flags;


}tcp_client_t;



/******************************************************************************/
/*                                                                            */
/*                       TCP Functions Prototypes                             */
/*                                                                            */
/******************************************************************************/



/********************************************************************
 * @brief  Function to create TCP client object (STATIC)
 * @param  source_port      : TCP source port
 * @param  destination_port : TCP destination port
 * @param  *server_ip       : Server IP
 * @retval int8_t           : Error = 0, Success = TCP client object
 ********************************************************************/
tcp_client_t* tcp_create_client(uint16_t source_port, uint16_t destination_port, uint8_t *server_ip);




/*****************************************************************
 * @brief  Function to initialize TCP values to TCP client object
 * @param  *client          : Reference to TCP client handle
 * @param  source_port      : TCP source port
 * @param  destination_port : TCP destination port
 * @param  *server_ip       : Server IP
 * @retval int8_t           : Error = 0, Success = 1
 *****************************************************************/
uint8_t tcp_init_client(tcp_client_t *client, uint16_t source_port, uint16_t destination_port, uint8_t *server_ip);




/**********************************************************
 * @brief  Function to establish connection to TCP server
 * @param  *ethernet     : Reference to Ethernet handle
 * @param  *network_data : Network data
 * @param  *client       : reference to TCP client handle
 * @retval int8_t        : Error = -11, Success = 1
 **********************************************************/
int8_t ether_tcp_handshake(ethernet_handle_t *ethernet, uint8_t *network_data ,tcp_client_t *client);





/*****************************************************************
 * @brief  Function for sending TCP data
 * @param  *ethernet         : Reference to the Ethernet Handle
 * @param  *network_data     : Network data
 * @param  *client           : Reference to TCP client handle
 * @param  *application_data : application_data
 * @param  data_length       : application data length
 * @retval int8_t            : Error = -12, Success = 1
 *****************************************************************/
int8_t ether_send_tcp_data(ethernet_handle_t *ethernet, uint8_t *network_data, tcp_client_t *client, char *application_data,
                           uint16_t data_length);




/*****************************************************************
 * @brief  Function for reading TCP data
 * @param  *ethernet         : Reference to the Ethernet Handle
 * @param  *network_data     : Network data
 * @param  *client           : Reference to TCP client handle
 * @param  *application_data : application_data
 * @param  data_length       : application data length
 * @retval int8_t            : Error = -12, Success = 1
 *****************************************************************/
int16_t ether_read_tcp_data(ethernet_handle_t *ethernet, uint8_t *network_data, tcp_client_t *client,
                            char *application_data, uint16_t data_length);





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
tcp_ctl_flags_t ether_get_tcp_server_ack(ethernet_handle_t *ethernet,  uint32_t *sequence_number, uint32_t *ack_number,
                                 uint16_t server_src_port, uint16_t client_src_port, uint8_t *sender_src_ip);




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
int8_t ether_send_tcp_ack(ethernet_handle_t *ethernet, uint16_t source_port, uint16_t destination_port,
                           uint32_t sequence_number, uint32_t ack_number, uint8_t *destination_ip, tcp_ctl_flags_t ack_type);




/***************************************************************
 * @brief  Function to get PSH ACK packet (TCP data packet)
 * @param  *ethernet          : Reference to Ethernet handle
 * @param  *tcp_data          : TCP data / payload
 * @param  data_buffer_length : Data buffer length
 * @retval int8_t             : Error = 0, Success = bytes read
 ***************************************************************/
uint16_t ether_get_tcp_psh_ack(ethernet_handle_t *ethernet, char *tcp_data, uint16_t data_buffer_length);




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
int8_t ether_send_tcp_psh_ack(ethernet_handle_t *ethernet, uint16_t source_port, uint16_t destination_port,
                              uint32_t sequence_number, uint32_t ack_number, uint8_t *destination_ip, char *tcp_data, uint16_t data_length);




#endif /* TCP_H_ */
