/**
 ******************************************************************************
 * @file    ethernet.h
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

#ifndef ETHERNET_H_
#define ETHERNET_H_



/*
 * Standard header and api header files
 */
#include <stdint.h>



/******************************************************************************/
/*                                                                            */
/*                      Data Structures and Defines                           */
/*                                                                            */
/******************************************************************************/


#define ntohs htons


/* Ethernet Frame structure (14 Bytes) */
typedef struct _ether_frame
{
  uint8_t  destination_mac_addr[6];   /*!< Destination MAC address */
  uint8_t  source_mac_addr[6];        /*!< Source MAC address      */
  uint16_t type;                      /*!< Ethernet frame type     */
  uint8_t  data;                      /*!< Ethernet data/payload   */

}ether_frame_t;



/* Ethernet/Network Operations handle */
typedef struct _ethernet_operations
{
    int16_t (*ether_send_packet)(uint8_t *data, uint16_t length);   /*!< Callback function to send Ethernet packet    */
    uint16_t (*ether_recv_packet)(uint8_t *data, uint16_t length);  /*!< Callback function to receive Ethernet packet */

}ethernet_operations_t;


/* Ethernet/Network Handle */
typedef struct _ethernet_handle
{
    ether_frame_t *ether_obj;               /*!< Ethernet frame object */
    uint8_t        host_mac[6];             /*!< Host MAC address      */
    uint8_t        host_ip[4];              /*!< Host IP address       */

    ethernet_operations_t *ether_commands;  /*!< Network Operations    */

}ethernet_handle_t;


/* Ethernet type values */
typedef enum _ether_type
{
    ETHER_IPV4 = 0x0800,  /*!< */
    ETHER_ARP  = 0x0806,  /*!< */
    ETHER_RARP = 0x8035,  /*!< */

}ether_type_t;




/* Network error codes protocol functions */
typedef enum _network_error_codes
{
    NET_ARP_REQ_ERROR     = -1,  /*!< */
    NET_ARP_RESP_ERROR    = -2,  /*!< */
    NET_ARP_RESP_IGNORE   = -3,
    NET_IP_GET_ERROR      = -4,
    NET_IP_CHECKSUM_ERROR = -5,

}network_erro_codes_t;




/******************************************************************************/
/*                                                                            */
/*                     Ethernet Function Prototypes                           */
/*                                                                            */
/******************************************************************************/



/************************************************************************
 * @brief  Function to convert from host to network order and vice versa
 *         (For 16 bit data)
 * @retval uint16_t : host to network or network to host converted data
 ************************************************************************/
uint16_t htons(uint16_t value);




int8_t ether_sum_words(uint32_t *sum, void *data, uint16_t size_in_bytes);


uint16_t ether_get_checksum(uint32_t sum);




/********************************************************
 * @brief  Function to set mac address
 * @param  *device_mac  : device mac address (Hex)
 * @param  *mac_address : mac address (string)
 * @retval int8_t       : Error = -1, Success = 0
 ********************************************************/
int8_t set_mac_address(char *device_mac, char *mac_address);




/********************************************************
 * @brief  function to set ip address
 * @param  *host_ip    : host ip address (integer)
 * @param  *ip_address : ip address (string)
 * @retval int8_t      : Error = -1, Success = 0
 ********************************************************/
int8_t set_ip_address(uint8_t *host_ip, char *ip_address);




/*************************************************************************
 * @brief  constructor function to create ethernet handle
 *         (Multiple exit points)
 * @param  *network_data  : reference to the network data buffer
 * @param  *mac_address   : mac address (string)
 * @param  *ip_address    : ip address (string)
 * @param  *ether_ops     : reference to the ethernet operations structure
 * @retval int8_t         : Error = NULL
 **************************************************************************/
ethernet_handle_t* create_ethernet_handle(uint8_t *network_data, char *mac_address, char *ip_address, ethernet_operations_t *ether_ops);




int8_t fill_ether_frame(ethernet_handle_t *ethernet, uint8_t *destination_mac_addr, uint8_t *source_mac_addr, ether_type_t frame_type);



#endif /* ETHERNET_H_ */
