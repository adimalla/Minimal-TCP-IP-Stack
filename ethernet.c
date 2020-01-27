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
#include "ethernet.h"




/******************************************************************************/
/*                                                                            */
/*                              Private Functions                             */
/*                                                                            */
/******************************************************************************/


/* Implementation of glibc reentrant strtok Copyright (C) 1991-2019 GNU C Library */

static char *api_strtok_r (char *s, const char *delim, char **save_ptr)
{
    char *end;
    if (s == NULL)
        s = *save_ptr;
    if (*s == '\0')
    {
        *save_ptr = s;
        return NULL;
    }
    /* Scan leading delimiters.  */
    s += strspn (s, delim);
    if (*s == '\0')
    {
        *save_ptr = s;
        return NULL;
    }
    /* Find the end of the token.  */
    end = s + strcspn (s, delim);
    if (*end == '\0')
    {
        *save_ptr = end;
        return s;
    }
    /* Terminate the token and make *SAVE_PTR point past it.  */
    *end = '\0';
    *save_ptr = end + 1;
    return s;
}



/********************************************************
 * @brief  static function to set mac address
 * @param  *device_mac  : device mac address (Hex)
 * @param  *mac_address : mac address (string)
 * @retval int8_t       : Error = -1, Success = 0
 ********************************************************/
static int8_t set_mac_address(char *device_mac, char *mac_address)
{
    int8_t func_retval = 0;

    char mac_address_copy[18] = {0};  /*!< Copy variable    */
    char *rest_ptr;                   /*!< Tracking pointer */
    char *token;                      /*!< token            */


    /* Copy mac address to copy variable for null termination (required for string function) */
    strncpy(mac_address_copy, mac_address, 18);  /* Size of mac address entered as string with null at index 18 */

    uint8_t index = 0;

    if(mac_address_copy[17] != 0)
    {
        func_retval = -1;
    }
    else
    {
        rest_ptr = mac_address_copy;

        /* strtok_r function for non glibc compliant code */
        while( (token = api_strtok_r(rest_ptr, ":", &rest_ptr)) )
        {
            /* Convert to hex */
            device_mac[index] = strtol(token, NULL, 16);

            index++;
        }

    }


    return func_retval;
}




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



/* API and supporting functions */



/************************************************************************
 * @brief  Function to convert from host to network order and vice versa
 *         (For 16 bit data)
 * @retval uint16_t : host to network or network to host converted data
 ************************************************************************/
uint16_t htons(uint16_t value)
{
    return ((value & 0xFF00) >> 8) + ((value & 0x00FF) << 8);
}



/********************************************************
 * @brief  function to set ip address
 * @param  *host_ip    : host ip address (integer)
 * @param  *ip_address : ip address (string)
 * @retval int8_t      : Error = -1, Success = 0
 ********************************************************/
int8_t set_ip_address(uint8_t *host_ip, char *ip_address)
{

    int8_t func_retval = 0;

    char ip_address_copy[13] = {0};   /*!< Copy variable    */
    char *rest_ptr;                   /*!< Tracking pointer */
    char *token;                      /*!< token            */


    /* Copy ip address to copy variable for null termination (required for string function) */
    strncpy(ip_address_copy, ip_address, 13);  /* Size of ip address entered as string with null at index 18 */

    uint8_t index = 0;

    if(ip_address_copy[12] != 0)
    {
        func_retval = -1;
    }
    else
    {
        rest_ptr = ip_address_copy;

        /* strtok_r function for non glibc compliant code */
        while( (token = api_strtok_r(rest_ptr, ".", &rest_ptr)) )
        {
            /* Convert to hex */
            host_ip[index] = strtol(token, NULL, 10);

            index++;
        }

    }

    return func_retval;


}



/*************************************************************************
 * @brief  constructor function to create ethernet handle
 *         (Multiple exit points)
 * @param  *network_data  : reference to the network data buffer
 * @param  *mac_address   : mac address (string)
 * @param  *ip_address    : ip address (string)
 * @param  *ether_ops     : reference to the ethernet operations structure
 * @retval int8_t         : Error = NULL
 **************************************************************************/
ethernet_handle_t* create_ethernet_handle(uint8_t *network_data, char *mac_address, char *ip_address, ethernet_operations_t *ether_ops)
{

    static ethernet_handle_t ethernet;

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

        /* Configure network operations and weak linking of default functions */
        ethernet.ether_commands = ether_ops;

        if(ether_ops->ether_send_packet == NULL)
            ether_ops->ether_send_packet = ethernet_send_packet;

        if(ether_ops->ether_recv_packet == NULL)
            ether_ops->ether_recv_packet = ethernet_recv_packet;

    }

    return &ethernet;
}


