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
#include <string.h>

#include "arp.h"




/******************************************************************************/
/*                                                                            */
/*                      Data Structures and Defines                           */
/*                                                                            */
/******************************************************************************/


/* ARP protocol structure */
typedef struct _arp
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

    uint8_t i = 0;

    if(ethernet->ether_obj == NULL)
    {
        func_retval = NET_ARP_REQ_ERROR;
    }
    else
    {
        /* Link network object data */
        arp = (void*)&ethernet->ether_obj->data;

        /* Fill Ethernet frame */
        for (i = 0; i < 6; i++)
        {
            ethernet->ether_obj->destination_mac_addr[i] = 0xFF;                   /* Broadcast MAC */
            ethernet->ether_obj->source_mac_addr[i]      = ethernet->host_mac[i];
        }

        ethernet->ether_obj->type = htons(ETHER_ARP);

        /* Fill ARP frame */
        arp->hardware_type = htons(ARP_HRD_ETHERNET);
        arp->protocol_type = htons(ARP_PRO_IPV4);
        arp->hardware_size = ARP_HLN;
        arp->protocol_size = ARP_PLN;
        arp->opcode        = htons(ARP_REQUEST);

        for (i = 0; i < 6; i++)
        {
            arp->sender_hw_addr[i] = ethernet->host_mac[i];
            arp->target_hw_addr[i] = 0xFF;
        }

        for (i = 0; i < 4; i++)
        {
            arp->sender_ip[i] = sender_ip[i];
            arp->target_ip[i] = target_ip[i];
        }

        /* Send packet (callback) */
        ethernet->ether_commands->ether_send_packet((uint8_t*)ethernet->ether_obj, 42);
    }

    return func_retval;
}



/********************************************************
 * @brief  Function to send arp response
 * @param  *ethernet  : reference to the Ethernet handle
 * @retval int16_t    : Error = -2, Success = 0
 ********************************************************/
int16_t ether_send_arp_resp(ethernet_handle_t *ethernet)
{
    int16_t func_retval = 0;

    net_arp_t *arp;

    uint8_t i = 0;

    if(ethernet->ether_obj == NULL)
    {
        func_retval = NET_ARP_RESP_ERROR;
    }
    else
    {
        arp = (void*)&ethernet->ether_obj->data;

        if( (strncmp((char*)arp->target_ip, (char*)ethernet->host_ip, 4) == 0) )
        {

            /* Fill ARP frame */
            arp->hardware_type = htons(ARP_HRD_ETHERNET);
            arp->protocol_type = htons(ARP_PRO_IPV4);
            arp->hardware_size = ARP_HLN;
            arp->protocol_size = ARP_PLN;

            /* Handle APR request */
            if(arp->opcode == ntohs(ARP_REQUEST))
            {
                arp->opcode = htons(ARP_REPLY);

                /* Swap Ethernet MAC Address */
                for (i = 0; i < 6; i++)
                {
                    ethernet->ether_obj->destination_mac_addr[i] = ethernet->ether_obj->source_mac_addr[i];
                    ethernet->ether_obj->source_mac_addr[i]      = ethernet->host_mac[i];
                }

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

                /* Send packet (callback) */
                ethernet->ether_commands->ether_send_packet((uint8_t*)ethernet->ether_obj, 42);
            }

        }
    }

    return func_retval;
}

