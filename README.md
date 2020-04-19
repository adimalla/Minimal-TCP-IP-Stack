# Minimal-TCP-IP-Stack

### Minimal-TCP/IP stack for enc28j60 module.

![Version:1.0](https://img.shields.io/badge/Version-1.0-green)
![IDE:CCStudio](https://img.shields.io/badge/IDE-CCStudio-red)
![Terminal:Putty](https://img.shields.io/badge/Teminal-puTTY-blue)

![License:MIT](https://img.shields.io/github/license/adimalla/MQTT-3.1-C?label=License)
<br/>
<br/>

## Description
**minimal-TCP/IP stack** is a portable network stack API using the enc28j60 ethernet module on ARM-cortex M4 architecture based board for non OS enviornment. Tested on (Texas Instruments TM4c123gh series processor), developed as a part of course projects (EE6314) at the University of Texas at Arlington. This API was used to provide a network layer interface for the home automation project and MQTT gateway project for low power sensor network.  
<br/>

## Features

##### Current:-
* Simple and easy to use API.
* Implements Ethernet layer, ARP, ICMP, TCP and UDP protocols.
* Supports Dynamic IP through DHCP.
* API based layers for raw TCP and UDP sockets.
* Supports max MTU of 1500 bytes.
* Lightweight stack usage of approximate 4096 bytes for medium scale MCU based embedded systems.
* Portable, can be ported to other platforms.
* Extensively tested as TCP and UDP clients and also as clients with application layer protocols like MQTT. 

##### Future Version:-
* Linux Sockets like API.
* Support for multiple sockets.
* RTOS based testing and support.

## Testing

**The API is tested on TIVA Launcpad by Texas Instruments and enc28j60 module.**
</br>

##### Hardware Configuration

ENC28J60 Ethernet controller connected on the following pins :- </br>
* MOSI (SSI2Tx)  on PB7 </br>
* MISO (SSI2Rx)  on PB6 </br>
* SCLK (SSI2Clk) on PB4 </br>
* ~CS  (GPIO)    on PB1 </br>

##### Dependencies

Test code is dependent on the following APIs:- </br>
* Clone "clterm" Repo from     : https://github.com/adimalla/clTerm
* Clone "MQTT-3.1-C" Repo from : https://github.com/adimalla/MQTT-3.1-C


## Disclaimer
If you are a student at The University of Texas at Arlington, please take prior permissions from Dr.Jason Losh and author of this repository before using any part of the source code in your project, in order to abide by the academic integrity of the university.

## Contact

* aditya.mall1990@gmail.com
* aditya.mall@mavs.uta.edu

<br/>

