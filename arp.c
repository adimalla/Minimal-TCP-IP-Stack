/**
 ******************************************************************************
 * @file    arp.c
 * @author  Aditya Mall,
 * @brief   ARP Protocol source file
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
#include <limits.h>
#include <string.h>


#include "arp.h"
#include "network_utilities.h"



/******************************************************************************/
/*                                                                            */
/*                      Data Structures and Defines                           */
/*                                                                            */
/******************************************************************************/


/* ARP protocol structure */
typedef struct _net_arp
{
    uint16_t hardware_type;      /*!< Network hardware type         */
    uint16_t protocol_type;      /*!< Network protocol type         */
    uint8_t  hardware_size;      /*!< Hardware address (MAC) length */
    uint8_t  protocol_size;      /*!< Protocol address (IP) length  */
    uint16_t opcode;             /*!< Opcode                        */
    uint8_t  sender_hw_addr[6];  /*!< Source hardware address       */
    uint8_t  sender_ip[4];       /*!< Source IP address             */
    uint8_t  target_hw_addr[6];  /*!< Destination hardware address  */
    uint8_t  target_ip[4];       /*!< Source hardware address       */

}net_arp_t;


/* ARP protocol constant values */
typedef enum _arp_protocol_constants
{
    ARP_HRD_ETHERNET = 1,       /*!< Value for hardware type (Ethernet) */
    ARP_PRO_IPV4     = 0x0800,  /*!< Value for IPV4 protocol type       */
    ARP_HLN          = 6,       /*!< Length of hardware address         */
    ARP_PLN          = 4        /*!< Length of IP address               */

}arp_protocol_constants_t;



/* ARP protocol opcode values */
typedef enum _arp_opcodes
{
    ARP_REQUEST  = 1,
    ARP_REPLY    = 2,
    RARP_REQUEST = 3,
    RARP_REPLY   = 4,

}arp_opcodes_t;





/******************************************************************************/
/*                                                                            */
/*                              Private Functions                             */
/*                                                                            */
/******************************************************************************/



/********************************************************
 * @brief  Static function to set broadcast mac address
 * @param  *mac address  : mac address to be configured
 * @retval int16_t    : Error = -1, Success = 0
 ********************************************************/
static int8_t set_broadcast_mac_addr(uint8_t *mac_address)
{
    int8_t func_retval = 0;

    int8_t index = 0;

    if(mac_address == NULL)
    {
        func_retval = -1;
    }
    else
    {
        for(index = 0; index < 6; index++ )
        {
            mac_address[index] = 0xFF;
        }
    }

    return func_retval;
}




/******************************************************************************
 * @brief  Static Function to update ARP table
 * @param  *ethernet    : reference to the Ethernet handle
 * @param  *ip_address  : device ip address
 * @param  *mac_address : device mac_address
 * @retval int8_t       : Success = 0 (device added), 1(device already exists)
 ******************************************************************************/
static uint8_t update_arp_table(ethernet_handle_t *ethernet, uint8_t *ip_address, uint8_t *mac_address)
{

    uint8_t func_retval = 0;

    uint8_t index = 0;

    uint8_t found = 0;

    if(ethernet->ether_obj == NULL)
    {
        func_retval = 0;
    }
    else
    {

        for(index = 0; index < ARP_TABLE_SIZE; index++)
        {
            if( (strncmp((char*)ethernet->arp_table[index].ip_address, (char*)ip_address, ETHER_IPV4_SIZE) == 0) )
            {
                found = 1;

                break;
            }
            else if( (strncmp((char*)ethernet->arp_table[index].ip_address, NULL, ETHER_IPV4_SIZE) == 0) )
            {
                found = 0;

                strncpy((char*)ethernet->arp_table[index].ip_address, (char*)ip_address, ETHER_IPV4_SIZE);

                strncpy((char*)ethernet->arp_table[index].mac_address, (char*)mac_address, ETHER_MAC_SIZE);

                break;
            }
        }

        func_retval = found;
    }

    return func_retval;
}




/******************************************************************************/
/*                                                                            */
/*                           ARP Functions                                    */
/*                                                                            */
/******************************************************************************/



/********************************************************
 * @brief  Function to send arp request
 * @param  *ethernet  : reference to the Ethernet handle
 * @param  *sender_ip : sender ip address
 * @param  *target_ip : target ip address
 * @retval int16_t    : Error = -1, Success = 0
 ********************************************************/
int16_t ether_send_arp_req(ethernet_handle_t *ethernet, uint8_t *sender_ip, uint8_t *target_ip)
{
    int16_t func_retval = 0;

    net_arp_t *arp;

    uint8_t broadcast_mac_addr[6] = {0};
    uint8_t i = 0;

    if(ethernet->ether_obj == NULL)
    {
        func_retval = NET_ARP_REQ_ERROR;
    }
    else
    {
        /* Link network object data */
        arp = (void*)&ethernet->ether_obj->data;


        /* Set Broadcast MAC */
        set_broadcast_mac_addr(broadcast_mac_addr);


        /* Fill Ethernet frame */
        fill_ether_frame(ethernet, broadcast_mac_addr, ethernet->host_mac, ETHER_ARP);


        /* Fill ARP frame */
        arp->hardware_type = htons(ARP_HRD_ETHERNET);
        arp->protocol_type = htons(ARP_PRO_IPV4);
        arp->hardware_size = ARP_HLN;
        arp->protocol_size = ARP_PLN;
        arp->opcode        = htons(ARP_REQUEST);

        for (i = 0; i < 6; i++)
        {
            arp->sender_hw_addr[i] = ethernet->host_mac[i];
            arp->target_hw_addr[i] = broadcast_mac_addr[i];
        }

        for (i = 0; i < 4; i++)
        {
            arp->sender_ip[i] = sender_ip[i];
            arp->target_ip[i] = target_ip[i];
        }

        /* Send packet (uses callback) */
        ether_send_data(ethernet, (uint8_t*)ethernet->ether_obj, 42);
    }

    return func_retval;
}




/**********************************************************
 * @brief  Function to independently read ARP data
 *         (Blocking Call)
 * @param  *ethernet    : reference to the Ethernet handle
 * @param  *data        : network_data
 * @param  *data_length : length of data to be read
 * @retval uint8_t      : Error = 0, Success = 1
 **********************************************************/
uint8_t ether_arp_read_data(ethernet_handle_t *ethernet, uint8_t *data, uint16_t data_length)
{
    uint8_t func_retval = 0;
    uint8_t block_loop  = 0;

    if(ethernet->ether_obj == NULL || data == NULL || data_length == 0 || data_length > UINT16_MAX)
    {
        func_retval = 0;
    }
    else
    {
            block_loop = 1;

            /* Wait for data */
            while(block_loop)
            {
                if(ether_get_data(ethernet, data, data_length))
                {
                    /* Check if protocol is ARP */
                    if(ntohs(ethernet->ether_obj->type) == ETHER_ARP)
                        func_retval = 1;
                    else
                        func_retval = 0;

                    break;

                }
            }

    }

    return func_retval;
}




/******************************************************************
 * @brief  Function to handle ARP request and reply
 *         sends ARP reply if ARP request received
 * @param  *ethernet  : reference to the Ethernet handle
 * @retval int16_t    : Error = -2, -3 = reply ignore, Success = 0
 ******************************************************************/
int16_t ether_handle_arp_resp_req(ethernet_handle_t *ethernet)
{
    int16_t func_retval = 0;

    net_arp_t *arp;

    uint8_t i = 0;

    if(ethernet->ether_obj == NULL || ethernet->ether_obj->type != ntohs(ETHER_ARP))
    {
        func_retval = NET_ARP_RESP_ERROR;
    }
    else
    {

        arp = (void*)&ethernet->ether_obj->data;

        /* Check if ARP request is for host IP */
        if( (strncmp((char*)arp->target_ip, (char*)ethernet->host_ip, 4) == 0) )
        {
            /* Handle APR request */
            if(arp->opcode == ntohs(ARP_REQUEST))
            {

                /* Get data into ARP Table */
                update_arp_table(ethernet, arp->sender_ip, arp->sender_hw_addr);

                /* Swap Ethernet MAC Address */
                for (i = 0; i < 6; i++)
                {
                    ethernet->ether_obj->destination_mac_addr[i] = ethernet->ether_obj->source_mac_addr[i];
                    ethernet->ether_obj->source_mac_addr[i] = ethernet->host_mac[i];
                }

                /* Fill ARP frame */
                arp->hardware_type = htons(ARP_HRD_ETHERNET);
                arp->protocol_type = htons(ARP_PRO_IPV4);
                arp->hardware_size = ARP_HLN;
                arp->protocol_size = ARP_PLN;

                arp->opcode = htons(ARP_REPLY);

                /* Swap ARP hardware/MAC address */
                for (i = 0; i < 6; i++)
                {
                    arp->target_hw_addr[i] = arp->sender_hw_addr[i];
                    arp->sender_hw_addr[i] = ethernet->host_mac[i];
                }

                /* Swap ARP IP address */
                for (i = 0; i < 4; i++)
                {
                    arp->target_ip[i] = arp->sender_ip[i];
                    arp->sender_ip[i] = ethernet->host_ip[i];
                }

                /* Send packet (uses callback) */
                ether_send_data(ethernet, (uint8_t*)ethernet->ether_obj, 42);
            }

            /* Handle APR reply */
            else if(arp->opcode == ntohs(ARP_REPLY))
            {
                update_arp_table(ethernet, arp->sender_ip, arp->sender_hw_addr);
            }

        }
        else
        {
            func_retval = NET_ARP_RESP_IGNORE;
        }

    }

    return func_retval;
}








