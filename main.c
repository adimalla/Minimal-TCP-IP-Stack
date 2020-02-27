// Ethernet Example
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL w/ ENC28J60
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// ENC28J60 Ethernet controller
//   MOSI (SSI2Tx) on PB7controller
//   MISO (SSI2Rx) on PB6
//   SCLK (SSI2Clk) on PB4
//   ~CS connected to PB1

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "enc28j60.h"
#include "wait.h"

#include "ethernet.h"
#include "network_utilities.h"
#include "arp.h"
#include "ipv4.h"
#include "icmp.h"
#include "udp.h"
#include "dhcp.h"
#include "tcp.h"

#include "cl_term.h"
#include "mqtt_client.h"


#define RED_LED      (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))
#define GREEN_LED    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))
#define BLUE_LED     (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 2*4)))
#define PUSH_BUTTON  (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 4*4)))



//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    // Note UART on port A must use APB
    SYSCTL_GPIOHBCTL_R = 0;

    // Enable GPIO port B and E peripherals
    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOB | SYSCTL_RCGC2_GPIOD | SYSCTL_RCGC2_GPIOF | SYSCTL_RCGC2_GPIOA | SYSCTL_RCGC2_GPIOE;

    // Configure LED and pushbutton pins
    GPIO_PORTF_DIR_R = 0x0E;  // bits 1-3 are outputs, other pins are inputs
    GPIO_PORTF_DR2R_R = 0x0E; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTF_DEN_R = 0x1E;  // enable LEDs and pushbuttons
    GPIO_PORTF_PUR_R = 0x1E;  // enable internal pull-up for push button

    /* ether module interrupt */



#if IOT_COURSE_TEST

    // Configure ~CS for ENC28J60
    GPIO_PORTA_DIR_R = (1 << 3);  // make bit 1 an output
    GPIO_PORTA_DEN_R = (1 << 3);  // enable bits 1 for digital

    //    GPIO_PORTD_DIR_R = (1 << 1);  // make bit 1 an output
    //    GPIO_PORTD_DEN_R = (1 << 1);  // enable bits 1 for digital

    SYSCTL_RCGCSSI_R   |= SYSCTL_RCGCSSI_R0;
    GPIO_PORTA_DIR_R   |= ( 1 << 2) | (1 << 3) | (1 << 5);
    GPIO_PORTA_DR2R_R  |= ( 1 << 2) | (1 << 4) | (1 << 5);
    GPIO_PORTA_AFSEL_R |= ( 1 << 2) | (1 << 4) | (1 << 5);
    GPIO_PORTA_PCTL_R   = GPIO_PCTL_PA5_SSI0TX | GPIO_PCTL_PA4_SSI0RX | GPIO_PCTL_PA2_SSI0CLK;
    GPIO_PORTA_DEN_R   |= ( 1<< 2) | (1 << 4) | (1 << 5);

    SSI0_CR1_R  &= ~SSI_CR1_SSE;
    SSI0_CR1_R  = 0;                                 // select master mode
    SSI0_CC_R   = 0;                                 // select system clock as the clock source
    SSI0_CPSR_R = 20;                                // set bit rate to 1 MHz (if SR=0 in CR0)
    SSI0_CR0_R  = SSI_CR0_FRF_MOTO | SSI_CR0_DSS_8;  // set SR=0, mode 0 (SPH=0, SPO=0), 8-bit
    SSI0_CR1_R |= SSI_CR1_SSE;

#else

    // Configure ~CS for ENC28J60
    GPIO_PORTB_DIR_R = (1 << 5);  // make bit 1 an output
    //GPIO_PORTB_DR2R_R = 0x02; // set drive strength to 2mA
    GPIO_PORTB_DEN_R = (1 << 5);  // enable bits 1 for digital

    // Configure SSI2 pins for SPI configuration
    SYSCTL_RCGCSSI_R |= SYSCTL_RCGCSSI_R2;           // turn-on SSI2 clocking
    GPIO_PORTB_DIR_R |= 0x90;                        // make bits 4 and 7 outputs
    GPIO_PORTB_DR2R_R |= 0x90;                       // set drive strength to 2mA
    GPIO_PORTB_AFSEL_R |= 0xD0;                      // select alternative functions for MOSI, MISO, SCLK pins
    GPIO_PORTB_PCTL_R = GPIO_PCTL_PB7_SSI2TX | GPIO_PCTL_PB6_SSI2RX | GPIO_PCTL_PB4_SSI2CLK; // map alt fns to SSI2
    GPIO_PORTB_DEN_R |= 0xD0;                        // enable digital operation on TX, RX, CLK pins

    // Configure the SSI2 as a SPI master, mode 3, 8bit operation, 1 MHz bit rate
    SSI2_CR1_R &= ~SSI_CR1_SSE;                      // turn off SSI2 to allow re-configuration
    SSI2_CR1_R = 0;                                  // select master mode
    SSI2_CC_R = 0;                                   // select system clock as the clock source
    SSI2_CPSR_R = 40;                                // set bit rate to 1 MHz (if SR=0 in CR0)
    SSI2_CR0_R = SSI_CR0_FRF_MOTO | SSI_CR0_DSS_8;   // set SR=0, mode 0 (SPH=0, SPO=0), 8-bit
    SSI2_CR1_R |= SSI_CR1_SSE;                       // turn on SSI2
#endif

}


void init_uart0(void)
{

    // Enable GPIO port F peripherals
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;

    // Configure UART0 pins
    SYSCTL_RCGCUART_R  |= SYSCTL_RCGCUART_R0;                       // Turn-on UART0, leave other uarts in same status
    GPIO_PORTA_DEN_R   |= 3;                                        // Turn on Digital Operations on PA0 and PA1
    GPIO_PORTA_AFSEL_R |= 3;                                        // Select Alternate Functionality on PA0 and PA1
    GPIO_PORTA_PCTL_R  |= GPIO_PCTL_PA1_U0TX | GPIO_PCTL_PA0_U0RX;  // Select UART0 Module

    // Configure UART0 to 115200 baud, 8N1 format (must be 3 clocks from clock enable and config writes)
    UART0_CTL_R   = 0;                                              // turn-off UART0 to allow safe programming
    UART0_CC_R   |= UART_CC_CS_SYSCLK;                              // use system clock (40 MHz)
    UART0_IBRD_R  = 21;                                             // r = 40 MHz / (Nx115.2kHz), set floor(r)=21, where N=16
    UART0_FBRD_R  = 45;                                             // round(fract(r)*64)=45
    UART0_LCRH_R |= UART_LCRH_WLEN_8 | UART_LCRH_FEN;               // configure for 8N1 w/ 16-level FIFO
    UART0_CTL_R  |= UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN;  // enable TX, RX, and module

}



// Blocking function that writes a serial character when the UART buffer is not full
void putcUart0(const char c)
{
    while(UART0_FR_R & UART_FR_TXFF);
    UART0_DR_R = c;

}


// Blocking function that returns with serial data once the buffer is not empty
char getcUart0(void)
{
    while (UART0_FR_R & UART_FR_RXFE);
    return UART0_DR_R & 0xFF;
}



// Blocking function that writes a string when the UART buffer is not full
void putsUart0(const char* str)
{
    uint8_t i;

    for (i = 0; i < strlen(str); i++)
        putcUart0(str[i]);
}




void init_adc(void)
{

    // Enable GPIO port B and E peripherals

    // Configure AN0 as an analog input
    SYSCTL_RCGCADC_R |= SYSCTL_RCGCADC_R0;                           // turn on ADC module 0 clocking
    GPIO_PORTE_AFSEL_R |= (1 << 3);                      // select alternative functions for AN0 (PE3)
    GPIO_PORTE_DEN_R &= ~(1 << 3);                       // turn off digital operation on pin PE3
    GPIO_PORTE_AMSEL_R |= (1 << 3);                      // turn on analog operation on pin PE3
    ADC0_CC_R = ADC_CC_CS_SYSPLL;                    // select PLL as the time base (not needed, since default value)
    ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN3;                // disable sample sequencer 3 (SS3) for programming
    ADC0_EMUX_R = ADC_EMUX_EM3_PROCESSOR;            // select SS3 bit in ADCPSSI as trigger
    ADC0_SSMUX3_R = 0;                               // set first sample to AN0
    ADC0_SSCTL3_R = ADC_SSCTL3_END0 | ADC_SSCTL3_TS0;                 // mark first sample as the end
    ADC0_ACTSS_R |= ADC_ACTSS_ASEN3;                 // enable SS3 for operation

}


uint16_t readAdc0Ss3()
{
    ADC0_PSSI_R |= ADC_PSSI_SS3;                     // set start bit
    while (ADC0_ACTSS_R & ADC_ACTSS_BUSY);           // wait until SS3 is not busy
    return ADC0_SSFIFO3_R & 0x0F;                           // get single result from the FIFO
}



/* wrapper function */

uint8_t ether_open(uint8_t *mac_address)
{

    etherInit(ETHER_UNICAST | ETHER_BROADCAST | ETHER_HALFDUPLEX, mac_address);

    return 0;
}


uint8_t myUartOpen(uint32_t baud_rate)
{
    init_uart0();

    //return 1 for success, here
    return 1;
}


uint8_t myPutChar(char data)
{
    putcUart0(data);

    return 0;
}


console_ops_t myUartOperations =
{

 .open       = myUartOpen,
 .print_char = myPutChar,
 .read_char  = getcUart0,

};



typedef enum _app_state
{
    APP_INIT   = 0,
    APP_READ   = 1,
    APP_WRITE  = 2,
    USER_INPUT = 3,

}app_state_t;




#define STATIC   1
#define UDP_TEST 0
#define TCP_TEST 1


int main(void)
{

    enc28j60_frame_t  *network_hardware;
    ethernet_handle_t *ethernet;

    uint8_t data[ETHER_MTU_SIZE] = {0};

    network_hardware = (void*)data;

    /* For UDP packets */
    char udp_data[APP_BUFF_SIZE] = {0};

    uint8_t loop   = 0;
    int16_t retval = 0;

    /* init controller */
    initHw();

    init_adc();


    /* Console definitions */
    cl_term_t         *my_console;
    char    serial_buffer[MAX_INPUT_SIZE] = {0};

    my_console = console_open(&myUartOperations, 115200, serial_buffer, CONSOLE_STATIC);

    console_print(my_console, CONSOLE_CLEAR_SCREEN);


    /* Link network operation functions */
    ether_operations_t ether_ops =
    {
     .open                     = ether_open,
     .network_interface_status = etherKbhit,
     .ether_send_packet        = etherPutPacket,
     .ether_recv_packet        = etherGetPacket,
     .random_gen_seed          = readAdc0Ss3,
    };

    /* Create Ethernet handle */
    ethernet = create_ethernet_handle(&network_hardware->data, "02:03:04:50:60:48", "192.168.1.199", &ether_ops);

    // flash phy leds
    etherWritePhy(PHLCON, 0x0880);
    RED_LED = 1;
    waitMicrosecond(500000);
    etherWritePhy(PHLCON, 0x0990);
    RED_LED = 0;
    waitMicrosecond(500000);


#if STATIC

    set_ip_address(ethernet->gateway_ip, "192.168.1.196");

#else

    /* test DHCP */
    ether_dhcp_enable(ethernet, (uint8_t*)network_hardware, DHCP_INIT_STATE);

#endif

    /* Test ARP packets */
    uint8_t sequence_no = 1;

    ether_send_arp_req(ethernet, ethernet->host_ip, ethernet->gateway_ip);

    if(ether_is_arp(ethernet, (uint8_t*)network_hardware, 128))
    {

        ether_handle_arp_resp_req(ethernet);

        GREEN_LED = 1;
        waitMicrosecond(50000);
        GREEN_LED = 0;
    }


    /* Test ICMP packets */
    ether_send_icmp_req(ethernet, ICMP_ECHOREQUEST, ethernet->gateway_ip, &sequence_no, \
                        ethernet->arp_table[0].mac_address, ethernet->host_mac);


#if UDP_TEST

    ether_send_udp(ethernet, ethernet->gateway_ip, 8080, "Hello", 5);

    ether_read_udp(ethernet, (uint8_t*)network_hardware, udp_data, APP_BUFF_SIZE);

    if(strncmp(udp_data, "Hello from server", 18) == 0)
        ether_send_udp(ethernet, ethernet->gateway_ip, 8080, "Hello again", 11);

#endif


    /* Test TCP application */
    uint16_t tcp_src_port  = 0;
    uint16_t tcp_dest_port = 0;
    int32_t  tcp_retval    = 0;
    int16_t  input_length  = 0;

    uint16_t count = 0;


    char tcp_data[50] = {0};
    uint8_t destination_ip[4] = {0};

    set_ip_address(destination_ip, "192.168.1.13");

    tcp_handle_t *test_client;

    /* APP state machine */
    app_state_t app_state = APP_INIT;

    loop = 1 ;


    while(loop)
    {
        switch(app_state)
        {

        case APP_INIT:

            tcp_dest_port = 7788;

            tcp_src_port = get_random_port(ethernet, 6534);

            test_client = ether_tcp_create_client(ethernet, (uint8_t*)network_hardware, tcp_src_port, tcp_dest_port, destination_ip);

            ether_tcp_connect(ethernet, (uint8_t*)network_hardware, test_client);

            tcp_control(test_client, TCP_READ_NONBLOCK);

            app_state = APP_WRITE;

            break;


        case APP_READ:

            tcp_retval = 0;

            console_print(my_console, "Read State \n");

            ether_tcp_read_data(ethernet, (uint8_t*)network_hardware, test_client, tcp_data, 50);

            app_state = APP_WRITE;

            console_print(my_console,tcp_data);
            console_print(my_console, "\n");

            if(count > 10000)
            {
                console_print(my_console,"Connection Closed");

                ether_tcp_close(ethernet, (uint8_t*)network_hardware, test_client);
                loop = 0;
                count = 0;
            }

            if(test_client->client_flags.connect_established == 0)
            {
                loop = 0;
            }

            break;


        case APP_WRITE:

            console_print(my_console, "Write State \n");
#if 0
            input_length = 0;

            input_length = console_get_string(my_console, MAX_INPUT_SIZE);

            if(input_length)
            {
                tcp_retval = ether_tcp_send_data(ethernet, (uint8_t*)network_hardware, test_client, serial_buffer, input_length);
            }
#else

            tcp_retval = ether_tcp_send_data(ethernet, (uint8_t*)network_hardware, test_client, "hey", 3);

#endif

            if(tcp_retval > 0)
                count++;

            app_state = APP_READ;

            break;

        }

    }


    /* State machine */
    loop = 1;

    while(loop)
    {

        if (ether_get_data(ethernet, (uint8_t*)network_hardware, ETHER_MTU_SIZE))
        {
            if (etherIsOverflow())
            {
                RED_LED = 1;
                waitMicrosecond(50000);
                RED_LED = 0;
            }

            switch(get_ether_protocol_type(ethernet))
            {

            case ETHER_ARP:

                retval = ether_handle_arp_resp_req(ethernet);

                if(retval == 0)
                {
                    RED_LED   = 1;
                    GREEN_LED = 1;
                    waitMicrosecond(50000);
                    RED_LED   = 0;
                    GREEN_LED = 0;
                }

                break;

            case ETHER_IPV4:

                /* Checks if UNICAST, validates checksum */
                retval = get_ip_communication_type(ethernet);

                if(retval == 1)
                {

                    /* Get transport layer protocol type */
                    switch(get_ip_protocol_type(ethernet))
                    {

                    case IP_ICMP:

                        /* Handle ICMP packets */
                        retval = ether_send_icmp_reply(ethernet);

                        if(retval == 0)
                        {
                            RED_LED  = 1;
                            BLUE_LED = 1;
                            waitMicrosecond(50000);
                            RED_LED  = 0;
                            BLUE_LED = 0;
                        }

                        break;

                    case IP_UDP:

                        /* Handle UDP packets */
                        if (ether_get_udp_data(ethernet, (uint8_t*)udp_data, APP_BUFF_SIZE))
                        {
                            BLUE_LED = 1;
                            waitMicrosecond(50000);
                            BLUE_LED = 0;
#if 1
                            /* test only */
                            if(strncmp(udp_data, "on", 2) == 0)
                            {
                                ether_send_udp(ethernet, ethernet->gateway_ip, 8080, "switched on", 11);
                            }
                            if(strncmp(udp_data, "off", 2) == 0)
                            {
                                ether_send_udp(ethernet, ethernet->gateway_ip, 8080, "switched off", 12);
                            }
#endif
                        }

                        break;


                    case IP_TCP:

                        break;


                    default:

                        break;

                    }

                }

                break;


            default:

                break;

            }

            memset(data, 0, sizeof(data));
        }

    }

    return 0;

}
