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
    return ADC0_SSFIFO3_R;                           // get single result from the FIFO
}






#define TEST 1



int main(void)
{
    uint8_t udpData[100] = {0};
    uint8_t data[128] = {0};

    uint8_t loop = 0;

    int16_t retval = 0;

    // init controller
    initHw();

    init_adc();

    // init ethernet interface
    etherInit(ETHER_UNICAST | ETHER_BROADCAST | ETHER_HALFDUPLEX);

    // flash phy leds
    etherWritePhy(PHLCON, 0x0880);
    RED_LED = 1;
    waitMicrosecond(500000);
    etherWritePhy(PHLCON, 0x0990);
    RED_LED = 0;
    waitMicrosecond(500000);

    enc28j60_frame_t  *network_hardware;
    ethernet_handle_t *ethernet;

    network_hardware = (void*)data;

    /* Link network operation functions */
    ether_operations_t ether_ops =
    {
     .network_interface_status = etherKbhit,
     .ether_send_packet        = etherPutPacket,
     .ether_recv_packet        = etherGetPacket,
     .random_gen_seed          = readAdc0Ss3,
    };

    /* Create Ethernet handle */
    ethernet = create_ethernet_handle(&network_hardware->data, "02:03:04:05:06:07", "192.168.10.2", &ether_ops);



#if TEST

    /* Test ARP packets */
    uint8_t test_ip[4] = {0};

    uint8_t sequence_no = 1;

    set_ip_address(test_ip, "192.168.10.1");

    ether_send_arp_req(ethernet, ethernet->host_ip,test_ip);

    if(ether_is_arp(ethernet, (uint8_t*)network_hardware, 128))
    {

        ether_handle_arp_resp_req(ethernet);

        GREEN_LED = 1;
        waitMicrosecond(50000);
        GREEN_LED = 0;
    }


    /* Test ICMP packets */
    ether_send_icmp_req(ethernet, ICMP_ECHOREQUEST, test_ip, &sequence_no,
                        ethernet->arp_table[0].mac_address, ethernet->host_mac);


    /* Test UDP packets */
    ether_source_t source_addresses;

    set_mac_address((char*)source_addresses.source_mac, "02:03:04:05:06:07");

    set_ip_address(source_addresses.source_ip, "192.168.10.2");

    source_addresses.source_port = get_random_port(ethernet, 2000);

    ether_send_udp_raw(ethernet, &source_addresses, test_ip, ethernet->arp_table[0].mac_address, 8080, (uint8_t*)"Hello", 5);

    if(ether_is_udp(ethernet, (uint8_t*)network_hardware, 128))
    {

        ether_send_udp_raw(ethernet, &source_addresses, test_ip, ethernet->arp_table[0].mac_address, 8080, (uint8_t*)"Received", 9);

        BLUE_LED = 1;
        waitMicrosecond(50000);
        BLUE_LED = 0;
    }


#endif

    /* State machine */

    loop = 1;

    while(loop)
    {

        if (ether_get_data(ethernet, (uint8_t*)network_hardware, 128))
        {
            if (etherIsOverflow())
            {
                RED_LED = 1;
                waitMicrosecond(50000);
                RED_LED = 0;
            }

            switch(ntohs(ethernet->ether_obj->type))
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
                        if (ether_get_udp_data(ethernet, ethernet->application_data, APP_BUFF_SIZE))
                        {
                            BLUE_LED = 1;
                            waitMicrosecond(50000);
                            BLUE_LED = 0;
                        }

                        break;

                    default:

                        break;

                    }
                }

                break;


            default:

                break;

            }

        }

        memset(data, 0, sizeof(data));
    }

    return 0;

}
