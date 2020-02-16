/**
 ******************************************************************************
 * @file    network_utilities.c
 * @author  Aditya Mall,
 * @brief   Network utility source file
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
#include <stdlib.h>

#include "network_utilities.h"



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




/******************************************************************************/
/*                                                                            */
/*                       Network Utility Functions                            */
/*                                                                            */
/******************************************************************************/



/************************************************************************
 * @brief  Function to convert from host to network order and vice versa
 *         (For 16 bit data)
 * @retval uint16_t : host to network or network to host converted data
 ************************************************************************/
uint16_t htons(uint16_t value)
{
    /* Ignore byte swap for 8 bit variables */
    if((value << 8) == 0)
        return value;
    else
        return ((value & 0xFF00) >> 8) + ((value & 0x00FF) << 8);
}




/************************************************************************
 * @brief  Function to convert from host to network order and vice versa
 *         (For 32 bit data)
 *         Copyright (C) 1997-2019 Free Software Foundation, Inc.
 *
 * @retval uint32_t : host to network or network to host converted data
 ************************************************************************/


/* Swap bytes in 32-bit value.  */
#define __bswap_constant_32(x)                                      \
        ((((x) & 0xff000000u) >> 24) | (((x) & 0x00ff0000u) >> 8)   \
       | (((x) & 0x0000ff00u) << 8) | (((x) & 0x000000ffu) << 24))


uint32_t htonl(uint32_t x)
{
    return (uint32_t)__bswap_constant_32 (x);
}






/********************************************************
 * @brief  Function to set mac address
 * @param  *device_mac  : device mac address (Hex)
 * @param  *mac_address : mac address (string)
 * @retval int8_t       : Error = -1, Success = 0
 ********************************************************/
int8_t set_mac_address(uint8_t *device_mac, char *mac_address)
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



/********************************************************
 * @brief  function to set ip address
 * @param  *host_ip    : host ip address (integer)
 * @param  *ip_address : ip address (string)
 * @retval int8_t      : Error = -1, Success = 0
 ********************************************************/
int8_t set_ip_address(uint8_t *host_ip, char *ip_address)
{

    int8_t func_retval = 0;

    char ip_address_copy[16] = {0};   /*!< Copy variable    */
    char *rest_ptr;                   /*!< Tracking pointer */
    char *token;                      /*!< token            */


    /* Copy ip address to copy variable for null termination (required for string function) */
    strncpy(ip_address_copy, ip_address, 16);  /* Size of ip address entered as string with null at index 18 */

    uint8_t index = 0;

    if(ip_address_copy[15] != 0)
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



/***********************************************
 * @brief  Function to swap network address
 *         (used in response messages)
 * @param  *l_address : address value LHS
 * @param  *r_address : address value RHS
 * @param   size      : size of address
 * @retval int8_t     : Error = -1, Success = 0
 ***********************************************/
int8_t net_swap_address(uint8_t* l_address, uint8_t *r_address, uint8_t size)
{
    int8_t func_retval = 0;
    int8_t index       = 0;

    if(l_address == NULL || r_address == NULL || size == 0 || size > 6)
    {
        func_retval = -1;
    }
    else
    {
        for(index = 0; index < size; index++)
        {
            l_address[index] = l_address[index] ^ r_address[index];
            r_address[index] = l_address[index] ^ r_address[index];
            l_address[index] = l_address[index] ^ r_address[index];
        }

    }

    return func_retval;
}





/*********************************************************
 * @brief  Function to set broadcast address
 * @param  *destination_address : destination address
 * @param  size                 : size of address
 * @retval int8_t               : Error = -1, Success = 0
 *********************************************************/
int8_t set_broadcast_address(uint8_t *destination_address, uint8_t size)
{
    int8_t func_retval = 0;
    int8_t index       = 0;

    if(destination_address == NULL || size == 0)
    {
        func_retval = -1;
    }
    else
    {
        switch(size)
        {

        case 4: /* For IP address length */

            for(index = 0; index < 4; index++)
            {
                destination_address[index] = 0xFF;
            }

            break;


        case 6: /* For MAC address length */

            for(index = 0; index < 6; index++)
            {
                destination_address[index] = 0xFF;
            }

            break;


        default:

            func_retval = -1;

            break;
        }

    }

    return func_retval;
}





