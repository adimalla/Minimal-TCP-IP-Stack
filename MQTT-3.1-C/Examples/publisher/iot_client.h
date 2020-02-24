/**
 ******************************************************************************
 * @file    iot_client.h
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


#ifndef IOT_CLIENT_H_
#define IOT_CLIENT_H_

#include <stdio.h>
#include <string.h>
#include "error_codes.h"


/******************************************************************************/
/*                                                                            */
/*                            Macro Defines                                   */
/*                                                                            */
/******************************************************************************/

#pragma pack(1)

#define MAX_ADDRESS_LENGTH  15
#define MAX_TOPIC_LENGTH    30


/******************************************************************************/
/*                                                                            */
/*                  Data Structures for Client Objects                        */
/*                                                                            */
/******************************************************************************/


/* Typedef to error codes strcuture */
typedef enum error_codes ClientRetVal;



/* Client Structure */
typedef struct client
{
	/* Members */
	int  socketDescriptor;
	int  serverPortNumber;
	char *serverAddress;
	char *topicName;
	int  qualityOfService;
	int  messageRetain;
	int  cleanSession;
	int  keepAliveTime;
	int  debugRequest;

	ClientRetVal returnValue;

	/* Methods */
	int     (*getCommands)(struct client *clientObj, int  argc, char **agrv, char *buffer);
	int     (*connectServer)(int *descriptor, int portNumber, char *serverAddr);

	ssize_t (*write)(int descriptor, const void *buffer, size_t length);
	ssize_t (*read)(int descriptor, void *buffer, size_t length);
	int     (*close)(int descriptor);

}IotClient;





/******************************************************************************/
/*                                                                            */
/*                           Function Prototypes                              */
/*                                                                            */
/******************************************************************************/


ClientRetVal clientBegin(IotClient *client);

ClientRetVal clientConnect(IotClient *client);

ClientRetVal clientEnd(IotClient *client);




#endif /* MQTT_3_1_C_EXAMPLES_PUBLISHER_IOT_CLIENT_H_ */
