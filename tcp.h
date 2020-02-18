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



#define TCP_FRAME_SIZE 20


/**/
typedef struct _net_tcp
{
    uint16_t source_port;
    uint16_t destination_port;
    uint32_t sequence_number;
    uint32_t ack_number;
    uint8_t  data_offset;
    uint8_t  control_bits;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent_pointer;
    uint8_t  data;

}net_tcp_t;


/**/
typedef enum _tcp_option_kinds
{
    TCP_NO_OPERATION     = 1,
    TCP_MAX_SEGMENT_SIZE = 2,
    TCP_SACK_PERMITTED   = 4,
    TCP_WINDOW_SCALING   = 3,
    TCP_TIMESTAMPS       = 8,

}tcp_opts_kind;


/**/
typedef struct tcp_max_segment_size
{
    uint8_t  option_kind;
    uint8_t  length;
    uint16_t value;

}tcp_mss_t;


/**/
typedef struct tcp_sack_permitted_size
{
    uint8_t option_kind;
    uint8_t length;

}tcp_sack_t;


/**/
typedef struct tcp_no_operation
{
    uint8_t option_kind;

}tcp_nop_t;


/**/
typedef struct tcp_window_scaling
{
    uint8_t option_kind;
    uint8_t length;
    uint8_t value;

}tcp_win_scale_t;


/**/
typedef struct _tcp_syn_options
{
    tcp_mss_t       mss;
    tcp_sack_t      sack;
    tcp_nop_t       nop;
    tcp_nop_t       nop1;
    tcp_nop_t       nop2;
    tcp_win_scale_t window_scale;

}tcp_syn_opts_t;




/******************************************************************************/
/*                                                                            */
/*                       TCP Functions Prototypes                             */
/*                                                                            */
/******************************************************************************/



int8_t ether_send_tcp_syn(ethernet_handle_t *ethernet, uint16_t source_port, uint16_t destination_port,
                              uint32_t sequence_number, uint32_t ack_number, uint8_t *destination_ip);




#endif /* TCP_H_ */
