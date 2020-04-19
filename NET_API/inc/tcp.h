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
 * Standard header and API header files
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

    TCP_FIN         = 0x01,
    TCP_SYN         = 0x02,
    TCP_RST         = 0x04,
    TCP_PSH         = 0x08,
    TCP_ACK         = 0x10,
    TCP_FIN_ACK     = 0x11,
    TCP_SYN_ACK     = 0x12,
    TCP_RST_ACK     = 0x14,
    TCP_PSH_ACK     = 0x18,
    TCP_FIN_PSH_ACK = 0x19,

}tcp_ctl_flags_t;


typedef enum _tcp_read_state
{
    TCP_READ_BLOCKING  = 1,
    TCP_READ_NONBLOCK  = 2,

}tcp_read_state_t;


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
typedef struct _tcp_handle
{
    uint16_t source_port;
    uint16_t destination_port;
    uint32_t sequence_number;
    uint32_t acknowledgement_number;
    uint8_t  server_ip[4];

    tcp_client_flags_t client_flags;


}tcp_handle_t;



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
tcp_handle_t* ether_tcp_create_client(ethernet_handle_t *ethernet,
                                      uint8_t           *network_data,
                                      uint16_t           source_port,
                                      uint16_t           destination_port,
                                      uint8_t           *server_ip);




/*****************************************************************
 * @brief  Function to initialize TCP values to TCP client object
 * @param  *client          : Reference to TCP client handle
 * @param  source_port      : TCP source port
 * @param  destination_port : TCP destination port
 * @param  *server_ip       : Server IP
 * @retval int8_t           : Error = 0, Success = 1
 *****************************************************************/
uint8_t tcp_init_client(tcp_handle_t *client,
                        uint16_t      source_port,
                        uint16_t      destination_port,
                        uint8_t      *server_ip);




/**********************************************************
 * @brief  Function to establish connection to TCP server
 * @param  *ethernet     : Reference to Ethernet handle
 * @param  *network_data : Network data
 * @param  *client       : reference to TCP client handle
 * @retval int8_t        : Error = -11, Success = 1
 **********************************************************/
int8_t ether_tcp_connect(ethernet_handle_t *ethernet, uint8_t *network_data ,tcp_handle_t *client);




/*****************************************************************
 * @brief  Function to control TCP read behavior
 * @param  *client     : Reference to TCP handle
 * @param  app_state   : TCP read type (blocking or non blocking)
 * @retval int8_t      : Error = 0, Success = 1
 ****************************************************************/
int8_t tcp_control(tcp_handle_t *client, tcp_read_state_t app_state);





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
int32_t ether_tcp_send_data(ethernet_handle_t *ethernet,
                            uint8_t           *network_data,
                            tcp_handle_t      *client,
                            char              *application_data,
                            uint16_t           data_length);





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
int32_t ether_tcp_read_data(ethernet_handle_t *ethernet,
                            uint8_t           *network_data,
                            tcp_handle_t      *client,
                            char              *tcp_data,
                            uint16_t           data_length);




/***************************************************************
 * @brief  Function for close socket
 * @param  *ethernet         : Reference to the Ethernet Handle
 * @param  *network_data     : Network data
 * @param  *client           : Reference to TCP handle
 * @retval uint16_t          : Error = 0, Success = 1;
 ***************************************************************/
uint8_t ether_tcp_close(ethernet_handle_t *ethernet, uint8_t *network_data, tcp_handle_t *client);




#endif /* TCP_H_ */
