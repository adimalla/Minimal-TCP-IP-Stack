// ENC28J60 Driver
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "enc28j60.h"
#include "wait.h"

#include <string.h>


#pragma pack(1)

// ------------------------------------------------------------------------------
//  Globals
// ------------------------------------------------------------------------------

uint8_t nextPacketLsb = 0x00;
uint8_t nextPacketMsb = 0x00;
uint8_t sequenceId = 1;
uint32_t sum;
uint8_t ipv4Address[4];

// ------------------------------------------------------------------------------
//  Structures
// ------------------------------------------------------------------------------

// This M4F is little endian (TI hardwired)
// Network byte order is big endian
// Must interpret uint16_t in reverse order






//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void spiWrite(uint8_t data)
{
#if IOT_COURSE_TEST

    SSI0_DR_R = data;
    while (SSI0_SR_R & SSI_SR_BSY);

#else
    SSI2_DR_R = data;
    while (SSI2_SR_R & SSI_SR_BSY);
#endif
}

uint8_t spiRead()
{
#if IOT_COURSE_TEST

    return SSI0_DR_R;

#else
    return SSI2_DR_R;
#endif
}

void etherCsOn()
{
    PIN_ETHER_CS = 0;
    __asm (" NOP");                    // allow line to settle
    __asm (" NOP");
    __asm (" NOP");
    __asm (" NOP");
}

void etherCsOff()
{
    PIN_ETHER_CS = 1;
}

void etherWriteReg(uint8_t reg, uint8_t data)
{
    etherCsOn();
    spiWrite(0x40 | (reg & 0x1F));
    spiRead();
    spiWrite(data);
    spiRead();
    etherCsOff();
}

uint8_t etherReadReg(uint8_t reg)
{
    uint8_t data;
    etherCsOn();
    spiWrite(0x00 | (reg & 0x1F));
    spiRead();
    spiWrite(0);
    data = spiRead();
    etherCsOff();
    return data;
}

void etherSetReg(uint8_t reg, uint8_t mask)
{
    etherCsOn();
    spiWrite(0x80 | (reg & 0x1F));
    spiRead();
    spiWrite(mask);
    spiRead();
    etherCsOff();
}

void etherClearReg(uint8_t reg, uint8_t mask)
{
    etherCsOn();
    spiWrite(0xA0 | (reg & 0x1F));
    spiRead();
    spiWrite(mask);
    spiRead();
    etherCsOff();
}

void etherSetBank(uint8_t reg)
{
    etherClearReg(ECON1, 0x03);
    etherSetReg(ECON1, reg >> 5);
}

void etherWritePhy(uint8_t reg, uint16_t data)
{
    etherSetBank(MIREGADR);
    etherWriteReg(MIREGADR, reg);
    etherWriteReg(MIWRL, data & 0xFF);
    etherWriteReg(MIWRH, (data >> 8) & 0xFF);
}

uint16_t etherReadPhy(uint8_t reg)
{
    uint16_t data, data2;
    etherSetBank(MIREGADR);
    etherWriteReg(MIREGADR, reg);
    etherWriteReg(MICMD, etherReadReg(MICMD) | MIIRD);
    waitMicrosecond(50);
    while ((etherReadReg(MISTAT) | MIBUSY) != 0);
    etherWriteReg(MICMD, etherReadReg(MICMD) & ~MIIRD);
    data = etherReadReg(MIRDL);
    data2 = etherReadReg(MIRDH);
    data |= (data2 << 8);
    return data;
}

void etherWriteMemStart()
{
    etherCsOn();
    spiWrite(0x7A);
    spiRead();
}

void etherWriteMem(uint8_t data)
{
    spiWrite(data);
    spiRead();
}

void etherWriteMemStop()
{
    etherCsOff();
}

void etherReadMemStart()
{
    etherCsOn();
    spiWrite(0x3A);
    spiRead();
}

uint8_t etherReadMem()
{
    spiWrite(0);
    return spiRead();
}

void etherReadMemStop()
{
    etherCsOff();
}


void setPhyMacAddr(uint8_t *macAddr)
{
    // setup mac address
    etherSetBank(MAADR0);
    etherWriteReg(MAADR5, macAddr[0]);
    etherWriteReg(MAADR4, macAddr[1]);
    etherWriteReg(MAADR3, macAddr[2]);
    etherWriteReg(MAADR2, macAddr[3]);
    etherWriteReg(MAADR1, macAddr[4]);
    etherWriteReg(MAADR0, macAddr[5]);

}


// Initializes ethernet device
// Uses order suggested in Chapter 6 of datasheet except 6.4 OST which is first here
void etherInit(uint8_t mode, uint8_t *macAddress)
{
    // make sure that oscillator start-up timer has expired
    while ((etherReadReg(ESTAT) & CLKRDY) == 0) {}

    // disable transmission and reception of packets
    etherClearReg(ECON1, RXEN);
    etherClearReg(ECON1, TXRTS);

    // initialize receive buffer space
    etherSetBank(ERXSTL);
    etherWriteReg(ERXSTL, LOBYTE(0x0000));
    etherWriteReg(ERXSTH, HIBYTE(0x0000));
    etherWriteReg(ERXNDL, LOBYTE(0x1A09));
    etherWriteReg(ERXNDH, HIBYTE(0x1A09));

    // initialize receiver write and read ptrs
    // at startup, will write from 0 to 1A08 only and will not overwrite rd ptr
    etherWriteReg(ERXWRPTL, LOBYTE(0x0000));
    etherWriteReg(ERXWRPTH, HIBYTE(0x0000));
    etherWriteReg(ERXRDPTL, LOBYTE(0x1A09));
    etherWriteReg(ERXRDPTH, HIBYTE(0x1A09));
    etherWriteReg(ERDPTL, LOBYTE(0x0000));
    etherWriteReg(ERDPTH, HIBYTE(0x0000));

    // setup receive filter
    // always check CRC, use OR mode
    etherSetBank(ERXFCON);
    etherWriteReg(ERXFCON, (mode | 0x20) & 0xBF);
    // bring mac out of reset
    etherSetBank(MACON2);
    etherWriteReg(MACON2, 0);

    // enable mac rx, enable pause control for full duplex
    etherWriteReg(MACON1, TXPAUS | RXPAUS | MARXEN);

    // enable padding to 60 bytes (no runt packets)
    // add crc to tx packets, set full or half duplex
    if ((mode & ETHER_FULLDUPLEX) != 0)
        etherWriteReg(MACON3, FULDPX | FRMLNEN | TXCRCEN | PAD60);
    else
        etherWriteReg(MACON3, FRMLNEN | TXCRCEN | PAD60);

    // leave MACON4 as reset

    // set maximum rx packet size
    etherWriteReg(MAMXFLL, LOBYTE(1518));
    etherWriteReg(MAMXFLH, HIBYTE(1518));

    // set back-to-back uint8_ter-packet gap to 9.6us
    if ((mode & ETHER_FULLDUPLEX) != 0)
        etherWriteReg(MABBIPG, 0x15);
    else
        etherWriteReg(MABBIPG, 0x12);

    // set non-back-to-back uint8_ter-packet gap registers
    etherWriteReg(MAIPGL, 0x12);
    etherWriteReg(MAIPGH, 0x0C);

    // leave collision window MACLCON2 as reset

    setPhyMacAddr(macAddress);

    // initialize phy duplex
    if ((mode & ETHER_FULLDUPLEX) != 0)
        etherWritePhy(PHCON1, PDPXMD);
    else
        etherWritePhy(PHCON1, 0);

    // disable phy loopback if in half-duplex mode
    etherWritePhy(PHCON2, HDLDIS);

    // set LEDA (link status) and LEDB (tx/rx activity)
    // stretch LED on to 40ms (default)
    etherWritePhy(PHLCON, 0x0472);

    // enable reception
    etherSetReg(ECON1, RXEN);
}

// Returns TRUE if packet received
uint8_t etherKbhit()
{
    return ((etherReadReg(EIR) & PKTIF) != 0);
}

// Returns up to max_size characters in data buffer
// Returns number of bytes copied to buffer
// Contents written are 16-bit size, 16-bit status, payload excl crc
uint16_t etherGetPacket(uint8_t data[], uint16_t max_size)
{
    uint16_t i = 0, size, tmp;

    // enable read from FIFO buffers
    etherReadMemStart();

    // get next pckt information
    nextPacketLsb = etherReadMem();
    nextPacketMsb = etherReadMem();

    // calc size
    // don't return crc, instead return size + status, so size is correct
    size = etherReadMem();
    data[i++] = size;
    tmp = etherReadMem();
    data[i++] = tmp;
    size |= (tmp << 8);

    // copy status + data
    if (size > max_size)
        size = max_size;
    while (i < size)
        data[i++] = etherReadMem();

    // end read from FIFO buffers
    etherReadMemStop();

    // advance read ptr
    etherSetBank(ERXRDPTL);
    etherWriteReg(ERXRDPTL, nextPacketLsb); // hw ptr
    etherWriteReg(ERXRDPTH, nextPacketMsb);
    etherWriteReg(ERDPTL, nextPacketLsb); // dma rd ptr
    etherWriteReg(ERDPTH, nextPacketMsb);

    // decrement packet counter so that PKTIF is maintained correctly
    etherSetReg(ECON2, PKTDEC);

    return size;
}

// Returns TRUE is rx buffer overflowed after correcting the problem
uint8_t etherIsOverflow()
{
    uint8_t err;
    err = (etherReadReg(EIR) & RXERIF) != 0;
    if (err)
        etherClearReg(EIR, RXERIF);
    return err;
}

// Writes a packet
int16_t etherPutPacket(uint8_t data[], uint16_t size)
{
    uint16_t i;

    // clear out any tx errors
    if ((etherReadReg(EIR) & TXERIF) != 0)
    {
        etherClearReg(EIR, TXERIF);
        etherSetReg(ECON1, TXRTS);
        etherClearReg(ECON1, TXRTS);
    }

    // set DMA start address
    etherSetBank(EWRPTL);
    etherWriteReg(EWRPTL, LOBYTE(0x1A0A));
    etherWriteReg(EWRPTH, HIBYTE(0x1A0A));

    // start FIFO buffer write
    etherWriteMemStart();

    // write control byte
    etherWriteMem(0);

    // write data
    for (i = 0; i < size; i++)
        etherWriteMem(data[i]);

    // stop write
    etherWriteMemStop();

    // request transmit
    etherWriteReg(ETXSTL, LOBYTE(0x1A0A));
    etherWriteReg(ETXSTH, HIBYTE(0x1A0A));
    etherWriteReg(ETXNDL, LOBYTE(0x1A0A+size));
    etherWriteReg(ETXNDH, HIBYTE(0x1A0A+size));
    etherClearReg(EIR, TXIF);
    etherSetReg(ECON1, TXRTS);

    // wait for completion
    while ((etherReadReg(ECON1) & TXRTS) != 0);

    // determine success
    return ((etherReadReg(ESTAT) & TXABORT) == 0);
}




