/**
 ******************************************************************************
 * @file    headers.h
 * @author  Aditya Mall,
 * @brief   Example MQTT publish client, for mosquitto MQTT Broker
 *
 *  Info
 *          Only for testing, (POSIX compatible only.)
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2019 Aditya Mall </center></h2>
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

#ifndef APP_HEADERS_H_
#define APP_HEADERS_H_



/* Header files */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>

#include "mqtt_client.h"
#include "iot_client.h"




/* @brief MACRO defines */

#define APP_VERSION         1.0
#define DEFAULT_BROKER_PORT 1883

#define LOOPBACK 1
#define IOT_LAB  0
#define WLAN     0
#define DHCP     1


#define LOCALHOST "127.0.0.1"

#if WLAN

#if IOT_LAB
#define HOST_IP_ADDR "192.168.1.186"
#else
#define HOST_IP_ADDR "192.168.1.12"
#endif

#else

#if DHCP
#define HOST_IP_ADDR "192.168.10.58"
#else
#define HOST_IP_ADDR "10.42.0.217"
#endif

#endif



/* Function Prototypes */
int mqtt_broker_connect(int *fd, int port, char *server_address);

int parse_command_line_args(IotClient *clientObj, int argc, char **argv, char *buffer);



#endif /* MQTT_3_1_C_EXAMPLES_PUBLISHER_HEADERS_H_ */
