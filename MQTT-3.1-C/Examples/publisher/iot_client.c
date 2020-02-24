/**
 ******************************************************************************
 * @file    iot_client.c
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


/* header files */
#include <stdlib.h>
#include "iot_client.h"

/*
 * @brief  Initializes Client structure members
 * @param  IotClient : pointer to IOT client structure
 * @retval int8_t    : 1 = Success, -1 = Error
 */
ClientRetVal clientBegin(IotClient *client)
{
	ClientRetVal errorCode = 0;

	/* Check function parameters */
	if(client == NULL)
	{
		errorCode = CLIENT_INIT_ERROR;
	}
	else
	{
		/* Initialize members */
		client->serverPortNumber = 0;
		client->serverPortNumber = 0;
		client->qualityOfService = 0;
		client->messageRetain    = 0;
		client->cleanSession     = 0;
		client->returnValue      = 0;
		client->keepAliveTime    = 0;
		client->debugRequest     = 0;

		/* Allocate Memory */
		client->serverAddress = malloc(sizeof(char) * MAX_ADDRESS_LENGTH);
		client->topicName     = malloc(sizeof(char) * MAX_TOPIC_LENGTH);

		memset(client->serverAddress, 0, sizeof(MAX_ADDRESS_LENGTH));
		memset(client->topicName, 0, sizeof(MAX_TOPIC_LENGTH));

		errorCode = FUNC_CODE_SUCCESS;

	}

	return errorCode;
}





/*
 * @brief  Connects to Messaging Server
 * @param  IotClient : pointer to IOT client structure
 * @retval int8_t    : 1 = Success, -1 = Error
 */
ClientRetVal clientConnect(IotClient *client)
{
	ClientRetVal errorCode = 0;

	if(client == NULL)
	{
		errorCode = CLIENT_SOCKET_ERROR;
	}
	else
	{
		errorCode = client->connectServer(&client->socketDescriptor, client->serverPortNumber, client->serverAddress);

		errorCode = FUNC_CODE_SUCCESS;

	}
	return errorCode;
}




/*
 * @brief  De-initializes Client structure members
 * @param  IotClient : pointer to IOT client structure
 * @retval int8_t    : 1 = Success, -1 = Error
 */
ClientRetVal clientEnd(IotClient *client)
{
	ClientRetVal errorCode = 0;

	/* Check param error */
	if(client == NULL)
	{
		errorCode = CLIENT_DEINIT_ERROR;
	}
	else
	{
		/* De-initialize members */
		client->serverPortNumber = 0;
		client->serverPortNumber = 0;
		client->qualityOfService = 0;
		client->messageRetain    = 0;
		client->cleanSession     = 0;
		client->returnValue      = 0;
		client->keepAliveTime    = 0;
		client->debugRequest     = 0;

		free(client->serverAddress);
		client->serverAddress = NULL;

		free(client->topicName);
		client->topicName = NULL;

		client->returnValue = FUNC_CODE_SUCCESS;

		client->close(client->socketDescriptor);

	}

	return errorCode;
}
