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
/*                         Macros and Defines                                 */
/*                                                                            */
/******************************************************************************/

/* structure padding */
#define pragma pack(1)


#define ETHER_PHY_DATA_OFFSET 4

#define ETHER_MAC_SIZE    6     /*!< Size of MAC address        */
#define ETHER_FRAME_SIZE  14    /*!< Ethernet Frame size        */
#define ETHER_IPV4_SIZE   4     /*!< IP protocol version 4 size */
#define ARP_TABLE_SIZE    5     /*!< ARP Table size define      */
#define ETHER_MTU_SIZE    1460  /*!< MAX MTU size               */
#define APP_BUFF_SIZE     350   /*!< Application buffer size    */


/* Function define for random number generator function */
#define get_unique_id    get_random_port
#define get_unique_id_l  get_random_port_l


/******************************************************************************/
/*                                                                            */
/*                           Data Structures                                  */
/*                                                                            */
/******************************************************************************/



/* Ethernet Frame structure (14 Bytes) */
typedef struct _ether_frame
{
  uint8_t  destination_mac_addr[6];   /*!< Destination MAC address */
  uint8_t  source_mac_addr[6];        /*!< Source MAC address      */
  uint16_t type;                      /*!< Ethernet frame type     */
  uint8_t  data;                      /*!< Ethernet data/payload   */

}ether_frame_t;



/* Ethernet Handle type defined */
typedef struct _ethernet_handle ethernet_handle_t;


/* ARP Table */
typedef struct _arp_table
{
    uint8_t ip_address[ETHER_IPV4_SIZE];  /*!< Device IP address  */
    uint8_t mac_address[ETHER_MAC_SIZE];  /*!< Device MAC address */

}arp_table_t;


/* */
typedef struct _network_timer_operations
{
    uint8_t (*open)(void);
    uint8_t (*start)(uint8_t timer_type, uint32_t seconds);
    uint8_t (*stop)(void);
    uint8_t (*reset)(void);

}net_timer_ops_t;


/* Ethernet/Network Operations handle */
typedef struct _ethernet_operations
{
    uint8_t  function_lock;                                          /*!< Function Lock for exclusive access of functions (experimental )        */
    uint8_t  (*open)(uint8_t *mac_address);                          /*!< Function initialize network/ETHER PHY, takes MAC address as parameter  */
    uint8_t  (*network_interface_status)(void);                      /*!< Network / Ethernet module packet receive status / trigger              */
    uint16_t (*random_gen_seed)(void);                               /*!< Seed value return function for random number generation                */
    int16_t  (*ether_send_packet)(uint8_t *data, uint16_t length);   /*!< Callback function to send Ethernet packet                              */
    uint16_t (*ether_recv_packet)(uint8_t *data, uint16_t length);   /*!< Callback function to receive Ethernet packet                           */

}ether_operations_t;


/* Ethernet status fields */
typedef struct _network_status
{
    uint8_t mode_static         : 1;
    uint8_t mode_dynamic        : 1;
    uint8_t mode_dhcp_init      : 1;
    uint8_t mode_dhcp_bound     : 1;
    uint8_t mode_read_blocking  : 1;
    uint8_t reserved            : 3;

}net_status_t;



/* Ethernet/Network Handle */
struct _ethernet_handle
{
    ether_frame_t      *ether_obj;                 /*!< Ethernet frame object                           */
    net_status_t       status;                     /*!< Ethernet status fields                          */
    ether_operations_t *ether_commands;            /*!< Network Operations                              */
    arp_table_t        arp_table[ARP_TABLE_SIZE];  /*!< ARP Table                                       */

    uint16_t source_port;                     /*!< Ethernet source port, gets random source port value  */
    uint8_t  *application_data;               /*!< Network application data, allocated by create handle */
    uint8_t  host_mac[ETHER_MAC_SIZE];        /*!< Host MAC address, given by handle                    */
    uint8_t  host_ip[ETHER_IPV4_SIZE];        /*!< Host IP address, static or dynamic                   */
    uint8_t  broadcast_mac[ETHER_MAC_SIZE];   /*!< Broadcast MAC address                                */
    uint8_t  broadcast_ip[ETHER_IPV4_SIZE];   /*!< Broadcast IP address                                 */
    uint8_t  subnet_mask[ETHER_IPV4_SIZE];    /*!< SUBNET mask, gets value from DHCP sever              */
    uint8_t  gateway_ip[ETHER_IPV4_SIZE];     /*!< Gateway or DHCP Server IP, from DHCP server          */
    uint32_t lease_time;                      /*!< IP lease time, from DHCP server                      */

};


typedef enum _ether_control_modes
{
    ETHER_READ_NONBLOCK = 0,
    ETHER_READ_BLOCK    = 1,
    ETHER_IP_STATIC     = 2,
    ETHER_IP_DYNAMIC    = 3,

}ether_control_t;




/* Ethernet source address fields, used for raw transport layer protocol functions */
typedef struct _ethernet_source_addr
{
    uint8_t  source_mac[ETHER_MAC_SIZE];  /*!< Source MAC address  */
    uint8_t  source_ip[ETHER_IPV4_SIZE];  /*!< Source IP address   */
    uint16_t source_port;                 /*!< Source port         */
    uint16_t identifier;                  /*!< Protocol Message ID */

}ether_source_t;



/* Ethernet type values */
typedef enum _ether_type
{
    ETHER_IPV4 = 0x0800,  /*!< IP protocol version 4 Ethernet type value */
    ETHER_ARP  = 0x0806,  /*!< ARP protocol Ethernet type value          */
    ETHER_RARP = 0x8035,  /*!< RARP protocol Ethernet type value         */

}ether_type_t;



/* Network error codes protocol functions */
typedef enum _network_error_codes
{
    NET_ARP_REQ_ERROR      = -1,  /*!< */
    NET_ARP_RESP_ERROR     = -2,  /*!< */
    NET_ARP_RESP_IGNORE    = -3,  /*!< */
    NET_IP_GET_ERROR       = -4,  /*!< */
    NET_IP_CHECKSUM_ERROR  = -5,  /*!< */
    NET_ICMP_RESP_ERROR    = -6,  /*!< */
    NET_ICMP_RESP_IGNORE   = -7,  /*!< */
    NET_ICMP_REQ_ERROR     = -8,  /*!< */
    NET_UDP_RAW_SEND_ERROR = -9,  /*!< */
    NET_UDP_SEND_ERROR     = -10, /*!< */

}network_erro_codes_t;





/******************************************************************************/
/*                                                                            */
/*                     Ethernet Function Prototypes                           */
/*                                                                            */
/******************************************************************************/



/******************************************************
 * @brief  Function to sum the data in network packet
 * @param  *sum          : Total 32 bit sum
 * @param  *data         : data to be summed
 * @param  size_in_bytes : size of the data
 * @retval uint16_t      : Error = -1, Success = 0
 ******************************************************/
int8_t ether_sum_words(uint32_t *sum, void *data, uint16_t size_in_bytes);



/******************************************************
 * @brief  Function to get checksum of network packet
 * @param  *sum          : Total 32 bit sum
 * @retval uint16_t      : checksum value
 ******************************************************/
uint16_t ether_get_checksum(uint32_t sum);




/******************************************************
 * @brief  Function to get random number above a bound
 * @param  *ethernet   : reference to Ethernet handle
 * @param  lower_bound : lower bound value
 * @retval uint16_t    : Error = -1, Success = 0
 ******************************************************/
int16_t get_random_port(ethernet_handle_t *ethernet, uint16_t lower_bound);




/******************************************************
 * @brief  Function to get random number above a bound
 *         (32 bit)
 * @param  *ethernet   : reference to Ethernet handle
 * @param  lower_bound : lower bound value
 * @retval uint32_t    : Error = -1, Success = 0
 ******************************************************/
int32_t get_random_port_l(ethernet_handle_t *ethernet, uint32_t lower_bound);




/**************************************************************************
 * @brief  constructor function to create Ethernet handle
 *         (Multiple exit points)
 * @param  *network_data  : reference to the network data buffer
 * @param  *mac_address   : MAC address (string)
 * @param  *ip_address    : ip address (string)
 * @param  *ether_ops     : reference to the Ethernet operations structure
 * @retval int8_t         : Error = NULL, Success = Ethernet object
 **************************************************************************/
ethernet_handle_t* create_ethernet_handle(uint8_t *network_data, char *mac_address, char *ip_address, ether_operations_t *ether_ops);





uint8_t ether_control(ethernet_handle_t *ethernet, ether_control_t ether_mode);



/**********************************************************
 * @brief  Function to get the Ethernet device status
 * @param  *ethernet : reference to the  Ethernet Handle
 * @retval  uint8_t  : Error = 0, Success =  1
 *********************************************************/
uint8_t ether_module_status(ethernet_handle_t *ethernet);



/***********************************************************
 * @brief  Function get Ethernet network data
 * @param  *ethernet    : reference to the Ethernet handle
 * @param  *data        : network data
 * @param  *data_length : source MAC address
 * @retval  uint8_t     : Error = 0, Success = 1
 ***********************************************************/
uint8_t ether_get_data(ethernet_handle_t *ethernet, uint8_t *data, uint16_t data_length);



/***********************************************************
 * @brief  Function send Ethernet network data
 * @param  *ethernet    : reference to the Ethernet handle
 * @param  *data        : network data
 * @param  *data_length : source MAC address
 * @retval  uint8_t     : Error = 0, Success = 1
 ***********************************************************/
uint8_t ether_send_data(ethernet_handle_t *ethernet, uint8_t *data, uint16_t data_length);



/**********************************************************************
 * @brief  Function to fill the Ethernet frame
 * @param  *ethernet                : reference to the Ethernet handle
 * @param  *destination_mac_address : destination MAC address
 * @param  *source_mac_address      : source MAC address
 * @param  frame type               : Ethernet frame type
 * @retval int8_t                   : Error = -1, Success = 0
 **********************************************************************/
int8_t fill_ether_frame(ethernet_handle_t *ethernet, uint8_t *destination_mac_addr, uint8_t *source_mac_addr, ether_type_t frame_type);



/***********************************************************
 * @brief  Function get ethernet protocol type
 * @param  *ethernet     : reference to the Ethernet handle
 * @retval  ether_type_t : Error = 0, Success = 1
 ***********************************************************/
ether_type_t get_ether_protocol_type(ethernet_handle_t *ethernet);



#endif /* ETHERNET_H_ */
