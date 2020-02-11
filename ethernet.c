/**
 ******************************************************************************
 * @file    ethernet.c
 * @author  Aditya Mall,
 * @brief   ethernet layer source file
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
#include <stdlib.h>
#include <time.h>

#include "ethernet.h"
#include "network_utilities.h"




/******************************************************************************/
/*                                                                            */
/*                           Ethernet Functions                               */
/*                                                                            */
/******************************************************************************/



/* Weak implementations of Ethernet operation functions */

__attribute__((weak))int16_t ethernet_send_packet(uint8_t *data, uint16_t length)
{

    return 0;
}



__attribute__((weak))uint16_t ethernet_recv_packet(uint8_t *data, uint16_t length)
{

    return 0;
}



__attribute__((weak))uint16_t random_seed(void)
{

    return 0;
}




/******************************************************
 * @brief  Function to sum the data in network packet
 * @param  *sum          : Total 32 bit sum
 * @param  *data         : data to be summed
 * @param  size_in_bytes : size of the data
 * @retval uint16_t      : Error = -1, Success = 0
 ******************************************************/
int8_t ether_sum_words(uint32_t *sum, void *data, uint16_t size_in_bytes)
{

    int8_t func_retval = 0;

    uint8_t  *data_ptr = (uint8_t *)data;

    uint16_t data_temp;
    uint8_t  phase = 0;

    uint16_t i     = 0;

    if(data == NULL)
    {
        func_retval = -1;
    }
    else
    {
        for(i = 0; i < size_in_bytes; i++)
        {
            if(phase)
            {
                data_temp = *data_ptr;

                *sum += data_temp << 8;
            }
            else
            {
                *sum += *data_ptr;
            }

            phase = 1 - phase;

            data_ptr++;
        }
    }

    return func_retval;
}




/******************************************************
 * @brief  Function to get checksum of network packet
 * @param  *sum          : Total 32 bit sum
 * @retval uint16_t      : checksum value
 ******************************************************/
uint16_t ether_get_checksum(uint32_t sum)
{
    uint16_t checksum = 0;

    while ((sum >> 16) > 0)
    {
        /* this is based on rfc1071 */
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    checksum = sum & 0xFFFF;

    /* return 1s complement */
    return ~checksum;
}



uint16_t get_random_port(ethernet_handle_t *ethernet, uint16_t lower_bound)
{
    int16_t func_retval    = 0;

    uint16_t random_number = 0;

    if(ethernet->ether_obj == NULL)
    {
        func_retval = 0;
    }
    else
    {
        srand(ethernet->ether_commands->random_gen_seed());

        random_number  = ( rand() % (UINT16_MAX - lower_bound) ) + lower_bound;

        func_retval = random_number;
    }

    return func_retval;
}




/**************************************************************************
 * @brief  constructor function to create Ethernet handle
 *         (Multiple exit points)
 * @param  *network_data  : reference to the network data buffer
 * @param  *mac_address   : MAC address (string)
 * @param  *ip_address    : ip address (string)
 * @param  *ether_ops     : reference to the Ethernet operations structure
 * @retval int8_t         : Error = NULL, Success = Ethernet object
 **************************************************************************/
ethernet_handle_t* create_ethernet_handle(uint8_t *network_data, char *mac_address, char *ip_address, ether_operations_t *ether_ops)
{

    static ethernet_handle_t ethernet;

    static uint8_t application_buffer[APP_BUFF_SIZE] = {0};

    int8_t api_retval = 0;

    if(network_data == NULL)
    {
        return NULL;
    }
    else
    {
        /* Give starting address of network data to*/
        ethernet.ether_obj = (void*)network_data;

        /* Set mac address */
        api_retval = set_mac_address((char*)ethernet.host_mac, mac_address);

        /* Set ip address */
        api_retval = set_ip_address(ethernet.host_ip, ip_address);

        if(api_retval < 0)
            return NULL;

        /* Configure application buffer */
        ethernet.application_data = application_buffer;

        /* Configure network operations and weak linking of default functions */
        ethernet.ether_commands = ether_ops;

        if(ether_ops->network_interface_status == NULL)
            return NULL;

        if(ether_ops->random_gen_seed == NULL)
            ether_ops->random_gen_seed = random_seed;

        if(ether_ops->ether_send_packet == NULL)
            ether_ops->ether_send_packet = ethernet_send_packet;

        if(ether_ops->ether_recv_packet == NULL)
            ether_ops->ether_recv_packet = ethernet_recv_packet;

        ethernet.source_port = get_random_port(&ethernet, 2000);

    }

    return &ethernet;
}



/**********************************************************
 * @brief  Function to get the Ethernet device status
 * @param  *ethernet : reference to the  Ethernet Handle
 * @retval  uint8_t  : Error = 0, Success =  1
 *********************************************************/
uint8_t ether_module_status(ethernet_handle_t *ethernet)
{
    uint8_t func_retval = 0;

    if(ethernet->ether_obj == NULL)
    {
        func_retval = 0;
    }
    else
    {
        if(ethernet->ether_commands->function_lock == 0)
        {
            ethernet->ether_commands->function_lock = 1;

            func_retval = ethernet->ether_commands->network_interface_status();

            ethernet->ether_commands->function_lock = 0;
        }
    }

    return func_retval;
}



/***********************************************************
 * @brief  Function  Ethernet network data
 * @param  *ethernet    : reference to the Ethernet handle
 * @param  *data        : network data
 * @param  *data_length : source MAC address
 * @retval  uint8_t     : Error = 0, Success = 1
 ***********************************************************/
uint8_t ether_get_data(ethernet_handle_t *ethernet, uint8_t *data, uint16_t data_length)
{

    uint8_t func_retval = 0;

    if(ethernet->ether_obj == NULL || data == NULL || data_length == 0 || data_length > UINT16_MAX)
    {
        func_retval = 0;
    }
    else
    {
        if(ether_module_status(ethernet))
        {
            ethernet->ether_commands->function_lock = 1;

            /* get data from network including PHY module frame */
            ethernet->ether_commands->ether_recv_packet(data, data_length);

            ethernet->ether_obj = (void*)((uint8_t*)data + ETHER_PHY_DATA_OFFSET);

            func_retval = 1;

            ethernet->ether_commands->function_lock = 0;
        }

    }

    return func_retval;
}



/***********************************************************
 * @brief  Function send Ethernet network data
 * @param  *ethernet    : reference to the Ethernet handle
 * @param  *data        : network data
 * @param  *data_length : source MAC address
 * @retval  uint8_t     : Error = 0, Success = 1
 ***********************************************************/
uint8_t ether_send_data(ethernet_handle_t *ethernet, uint8_t *data, uint16_t data_length)
{

    uint8_t func_retval = 0;

    if(ethernet->ether_obj == NULL || data == NULL || data_length == 0 || data_length > UINT16_MAX)
    {
        func_retval = 0;
    }
    else
    {
        ethernet->ether_commands->function_lock = 1;

        ethernet->ether_commands->ether_send_packet(data, data_length);

        func_retval = 1;

        ethernet->ether_commands->function_lock = 0;

    }

    return func_retval;
}



/**********************************************************************
 * @brief  Function to fill the Ethernet frame
 * @param  *ethernet                : reference to the Ethernet handle
 * @param  *destination_mac_address : destination MAC address
 * @param  *source_mac_address      : source MAC address
 * @param  frame type               : Ethernet frame type
 * @retval int8_t                   : Error = -1, Success = 0
 **********************************************************************/
int8_t fill_ether_frame(ethernet_handle_t *ethernet, uint8_t *destination_mac_addr, uint8_t *source_mac_addr, ether_type_t frame_type)
{
    int8_t func_retval = 0;

    uint8_t index = 0;

    if(ethernet->ether_obj == NULL || destination_mac_addr == NULL || source_mac_addr == NULL)
    {
        func_retval = -1;
    }
    else
    {
        /* Fill MAC address */
        for(index = 0; index < 6; index++)
        {
            ethernet->ether_obj->destination_mac_addr[index] = destination_mac_addr[index];
            ethernet->ether_obj->source_mac_addr[index]      = source_mac_addr[index];
        }

        ethernet->ether_obj->type = htons(frame_type);
    }

    return func_retval;
}





