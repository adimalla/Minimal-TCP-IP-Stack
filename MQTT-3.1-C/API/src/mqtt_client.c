/**
 ******************************************************************************
 * @file    mqtt_client.c
 * @author  Aditya Mall,
 * @brief   Example MQTT publish client, for mosquitto MQTT Broker
 *
 *  Info
 *          MQTT API Source File
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
 * Standard Header and API Header files
 */
#include <stdint.h>
#include <string.h>
#include "mqtt_client.h"



/******************************************************************************/
/*                                                                            */
/*                  Data Structures and Defines                               */
/*                                                                            */
/******************************************************************************/


/* @brief Defines for MQTT control packet Variable sizes */
#define CONNECT_PROTOCOL_LENGTH_SIZE   2                     /*!< */
#define CONNECT_PROTOCOL_NAME_SIZE     PROTOCOL_NAME_LENGTH  /*!< */
#define CONNECT_PROTOCOL_VERSION_SIZE  1                     /*!< */
#define CONNECT_FLAGS_SIZE             1                     /*!< */
#define CONNECT_KEEP_ALIVE_TIME_SIZE   2                     /*!< */
#define CONNECT_CLIENT_ID_LENGTH_SIZE  2                     /*!< */
#define CONNECT_USER_NAME_LENGTH_SIZE  2                     /*!< */
#define CONNECT_PASSWORD_LENGTH_SIZE   2                     /*!< */
#define PUBLISH_TOPIC_LENGTH_SIZE      2                     /*!< */
#define SUBSCRIBE_MESSAGE_ID_SIZE      2                     /*!< */
#define SUBSCRIBE_TOPIC_LENGTH_SIZE    2                     /*!< */
#define SUBSCRIBE_QOS_SIZE             1                     /*!< */


/* TODO correct mqtt error codes */
/* return codes for mqtt api functions */
typedef enum function_return_codes
{
	FUNC_OPTS_ERROR       = -1,  /*!< */
	FUNC_OPTS_SUCCESS     = 1,   /*!< */
	MAIN_MAIN_FUNC_ERROR  = 0,   /*!< */
	MAIN_FUNC_ERROR       = 0    /*!< */

}return_codes_t;



/******************************************************************************/
/*                                                                            */
/*                              API Functions                                 */
/*                                                                            */
/******************************************************************************/



/*
 * @brief  static function to convert to network byte order for mqtt packets
 * @param  value    : value in host byte order format
 * @retval uint16_t : value network byte order format
 */
static uint16_t mqtt_htons(uint16_t value)
{
	value  = ((value & 0xFF00) >> 8) + ((value & 0x00FF) << 8);

	return value;
}



/*
 * @brief  static function to convert from network byte order to host for mqtt packets
 * @param  value    : value in network byte order format
 * @retval uint16_t : value host byte order format
 */
static uint16_t mqtt_ntohs(uint16_t value)
{
	value  = ((value & 0xFF00) >> 8) + ((value & 0x00FF) << 8);

	return value;
}




/*
 * @brief  Configures mqtt client user name and password.
 * @param  *client   : pointer to mqtt client structure (mqtt_client_t).
 * @param  user_name : mqtt client user name
 * @param  password  : mqtt client password
 * @retval int8_t    : 1 = Success, -1 = Error
 */
int8_t mqtt_client_username_passwd(mqtt_client_t *client, char *user_name, char *password)
{

	uint8_t user_name_length = 0;
	uint8_t password_length  = 0;
	int8_t  func_retval      = 0;

	user_name_length = strlen(user_name);
	password_length = strlen(password);

	/* check if user name is not null */
	if( client == NULL || user_name == NULL  || password == NULL)
	{
		func_retval = FUNC_OPTS_ERROR;
	}
	/* check if user name and password doesn't exceed defined length, if yes return 0 */
	else if( user_name_length > USER_NAME_LENGTH || password_length > PASSWORD_LENGTH)
	{
		return FUNC_OPTS_ERROR;
	}
	else
	{
		client->connect_msg->connect_flags.user_name_flag = ENABLE;
		client->connect_msg->connect_flags.password_flag  = ENABLE;

		client->connect_msg->payload_options.user_name_length = mqtt_htons(user_name_length);
		strcpy(client->connect_msg->payload_options.user_name, user_name);

		client->connect_msg->payload_options.password_length  = mqtt_htons(password_length);
		strcpy(client->connect_msg->payload_options.password, password);

		func_retval = FUNC_OPTS_SUCCESS;
	}

	return func_retval;
}


/* TODO Correct will topic error in connect message */
/*
 * @brief  Configures mqtt connect options. (qos and retain don't have any effect on control packets currently)
 * @param  *client     : pointer to mqtt client structure (mqtt_client_t).
 * @param  session     : configure session type
 * @param  message_qos : configure quality of service
 * @param  retain      : configure mention retention at broker.
 * @retval int8_t      : qos value = Success, -1 = Error
 */
int8_t mqtt_connect_options(mqtt_client_t *client, uint8_t session, uint8_t retain, mqtt_qos_t message_qos)
{
	int8_t func_retval = 0;

	/* Check for correct values of session, retain and qos(quality of service) */
	if(session > MQTT_CLEAN_SESSION || retain > MQTT_MESSAGE_RETAIN || message_qos > MQTT_QOS_RESERVED || client == NULL)
	{
		func_retval = FUNC_OPTS_ERROR;
	}
	else
	{
		client->connect_msg->connect_flags.clean_session = session;

		/* Enable will flag and if qos and retain are enabled by user */
		if(message_qos || retain)
		{
			client->connect_msg->connect_flags.will_qos    = message_qos;
			client->connect_msg->connect_flags.will_retain = retain;
			client->connect_msg->connect_flags.will_flag   = DISABLE;
		}
		else
		{
			client->connect_msg->connect_flags.will_qos    = message_qos;
			client->connect_msg->connect_flags.will_retain = retain;
			client->connect_msg->connect_flags.will_flag   = DISABLE;
		}

		func_retval = message_qos;
	}

	return func_retval;
}



/*
 * @brief  Configures mqtt CONNECT message structure.
 * @param  *client         : pointer to mqtt client structure (mqtt_client_t).
 * @param  client_name     : Name of the mqtt client given by user.
 * @param  keep_alive_time : Keep Alive time for the client.
 * @retval size_t          : Length of connect message.
 */
size_t mqtt_connect(mqtt_client_t *client, char *client_name, int16_t keep_alive_time)
{
	size_t  message_length     = 0;
	uint8_t client_name_length = 0;
	uint8_t user_name_length   = 0;
	uint8_t password_length    = 0;
	uint8_t payload_index      = 0;

	size_t func_retval;

	/* TODO Implement error checks in mqtt connect */
	if(keep_alive_time < 0 || client_name == NULL || client == NULL)
	{
		func_retval = (size_t)MAIN_MAIN_FUNC_ERROR;
	}
	else
	{
		/* Check for client id length and truncate if greater than configuration specified length */
		client_name_length = strlen(client_name);
		if(client_name_length > CLIENT_ID_LENGTH)
		{
			client_name_length = CLIENT_ID_LENGTH;
		}

		/* Fill mqtt  connect structure */
		client->connect_msg->fixed_header.message_type  = MQTT_CONNECT_MESSAGE;

		client->connect_msg->protocol_name_length = mqtt_htons(PROTOCOL_NAME_LENGTH);
		strcpy(client->connect_msg->protocol_name, PROTOCOL_NAME);

		client->connect_msg->protocol_version = MQTT_PROTOCOL_VERSION;

		client->connect_msg->keep_alive_value = mqtt_htons(keep_alive_time);


		/*
		 * @brief Populate payload message fields as per available payload options,
		 *        and append the index of payload array as per options.
		 */
		if(client->connect_msg->connect_flags.user_name_flag)
		{
			/* Configure client ID and length */
			client->connect_msg->message_payload[0] = 0;
			client->connect_msg->message_payload[1] = client_name_length;
			strncpy(client->connect_msg->message_payload + CONNECT_CLIENT_ID_LENGTH_SIZE, client_name, client_name_length);

			user_name_length = strlen(client->connect_msg->payload_options.user_name);
			password_length = strlen(client->connect_msg->payload_options.password);

			/* Update index for user name and length details */
			payload_index = CONNECT_CLIENT_ID_LENGTH_SIZE + client_name_length;

			client->connect_msg->message_payload[payload_index]     = 0;
			client->connect_msg->message_payload[payload_index + 1] = user_name_length;
			strncpy(client->connect_msg->message_payload + payload_index + CONNECT_USER_NAME_LENGTH_SIZE, client->connect_msg->payload_options.user_name, user_name_length);


			/* Update index for password and length details */
			payload_index += CONNECT_USER_NAME_LENGTH_SIZE + user_name_length;

			client->connect_msg->message_payload[payload_index]     = 0;
			client->connect_msg->message_payload[payload_index + 1] = password_length;
			strncpy(client->connect_msg->message_payload + payload_index + 2, client->connect_msg->payload_options.password, password_length);

			/* Configure message length */
			message_length = (size_t)(FIXED_HEADER_LENGTH + CONNECT_PROTOCOL_LENGTH_SIZE + CONNECT_PROTOCOL_NAME_SIZE + CONNECT_PROTOCOL_VERSION_SIZE + CONNECT_FLAGS_SIZE + \
					CONNECT_KEEP_ALIVE_TIME_SIZE + CONNECT_CLIENT_ID_LENGTH_SIZE + client_name_length + CONNECT_USER_NAME_LENGTH_SIZE + CONNECT_PASSWORD_LENGTH_SIZE + \
					user_name_length + password_length);

		}
		else
		{
			/* Configure client ID and length */
			client->connect_msg->message_payload[0] = 0;
			client->connect_msg->message_payload[1] = client_name_length;
			strncpy(client->connect_msg->message_payload + CONNECT_CLIENT_ID_LENGTH_SIZE, client_name, client_name_length);

			/* Configure message length */
			message_length = (size_t)(FIXED_HEADER_LENGTH + CONNECT_PROTOCOL_LENGTH_SIZE + CONNECT_PROTOCOL_NAME_SIZE + CONNECT_PROTOCOL_VERSION_SIZE + CONNECT_FLAGS_SIZE + \
					CONNECT_KEEP_ALIVE_TIME_SIZE + CONNECT_CLIENT_ID_LENGTH_SIZE + client_name_length);

		}

		client->connect_msg->fixed_header.message_length = message_length - FIXED_HEADER_LENGTH;

		func_retval = message_length;
	}

	return func_retval;
}



/*
 * @brief  Returns the value of message type from input buffer.
 * @param  *client  : pointer to mqtt client structure (mqtt_client_t).
 * @retval  uint8_t : value of message type.
 */
uint8_t get_mqtt_message_type(mqtt_client_t *client)
{
	return client->message->message_type;
}



/*
 * @brief  Returns the value of connack message status from input buffer.
 * @param  *client  : pointer to mqtt client structure (mqtt_client_t).
 * @retval  uint8_t : connack message return code.
 */
uint8_t get_connack_status(mqtt_client_t *client)
{
	uint8_t message_state;

	/* If connection accepted then publish message */
	if(client->connack_msg->return_code == MQTT_CONNECTION_ACCEPTED)
	{
		message_state = MQTT_PUBLISH_MESSAGE;
	}
	else
	{
		message_state = MQTT_DISCONNECT_MESSAGE;
	}

	return message_state;
}



/*
 * @brief  Configures mqtt PUBLISH message options.
 * @param  *client        : pointer to mqtt client structure (mqtt_client_t).
 * @param  message_retain : Enable retain for message retention at broker
 * @param  message_qos    : Quality of service value (1:At-least once, 2:Exactly once)
 * @retval int8_t         : qos value = Success, -1 = Error
 */
int8_t mqtt_publish_options(mqtt_client_t *client, uint8_t message_retain, mqtt_qos_t message_qos)
{
	if(message_retain)
	{
		client->publish_msg->fixed_header.retain_flag = ENABLE;
	}

	/* Check if Quality of service value (qos) is less than reserved value (val:3) */
	if(message_qos < MQTT_QOS_RESERVED)
	{
		client->publish_msg->fixed_header.qos_level = message_qos;
	}
	else
	{
		return FUNC_OPTS_ERROR;
	}

	return message_qos;
}



/*
 * @brief  Configures mqtt PUBLISH message structure.
 * @param  *client          : pointer to mqtt client structure (mqtt_client_t).
 * @param  *publish_topic   : publish topic name
 * @param  *publish_message : message to be published
 * @retval size_t           : length of publish control packet, fail = 0;
 */
size_t mqtt_publish(mqtt_client_t *client, char *publish_topic, char *publish_message)
{

	uint8_t message_length         = 0;
	uint8_t publish_topic_length   = 0;
	uint8_t publish_message_length = 0;
	uint8_t payload_index          = 0;


	publish_topic_length = strlen(publish_topic);


	/*Check if quality of service is > 0 and accordingly adjust the length of publish message */
	if(client->publish_msg->fixed_header.qos_level > 0)
	{
		publish_message_length = strlen(publish_message) + MQTT_MESSAGE_ID_OFFSET;
	}
	else
	{
		publish_message_length = strlen(publish_message);
	}


	/* Check for overflow condition, if topic and message length is not greater than specified length */
	if(publish_topic_length > MQTT_TOPIC_LENGTH || publish_message_length > PUBLISH_PAYLOAD_LENGTH)
	{
		return MAIN_MAIN_FUNC_ERROR;
	}

	/* Fill main publish structure */
	client->publish_msg->fixed_header.message_type = MQTT_PUBLISH_MESSAGE;

	client->publish_msg->topic_length = mqtt_htons(publish_topic_length);


	/* Copy message to publish pay load, insert message ID if quality of service > 0 */
	if(client->publish_msg->fixed_header.qos_level > 0)
	{

		/* Copy topic to publish topic member */
		strncpy(client->publish_msg->payload, publish_topic, publish_topic_length);

		/* Update pay load index*/
		payload_index = publish_topic_length;

		/* Configure topic length */
		client->publish_msg->payload[payload_index]     = 0;
		client->publish_msg->payload[payload_index + 1] = 1;

		/* Copy pay-load message */
		strcpy(client->publish_msg->payload + payload_index + MQTT_MESSAGE_ID_OFFSET, publish_message);
	}
	else
	{
		/* Copy topic to publish topic member */
		strncpy(client->publish_msg->payload, publish_topic, publish_topic_length);

		strncpy(client->publish_msg->payload + publish_topic_length, publish_message, publish_message_length);
	}

	/* Configure Message Length */
	message_length = publish_message_length + publish_topic_length + PUBLISH_TOPIC_LENGTH_SIZE + FIXED_HEADER_LENGTH;

	client->publish_msg->fixed_header.message_length = message_length - FIXED_HEADER_LENGTH;

	return message_length;
}



/*
 * @brief  Configures mqtt PUBREL message structure.
 * @param  *client         : pointer to mqtt client structure (mqtt_client_t).
 * @retval size_t          : Length of publish release message.
 */
size_t mqtt_publish_release(mqtt_client_t *client)
{
	size_t message_length = 0;

	client->pubrel_msg->fixed_header.qos_level    = MQTT_QOS_ATLEAST_ONCE;
	client->pubrel_msg->fixed_header.message_type = MQTT_PUBREL_MESSAGE;

	client->pubrel_msg->message_id = mqtt_htons(1);

	message_length = sizeof(mqtt_pubrel_t);

	client->pubrel_msg->fixed_header.message_length = (uint8_t)(message_length - FIXED_HEADER_LENGTH);

	return message_length;
}


/*
 * @brief  Configures mqtt DISCONNECT message structure.
 * @param  *client         : pointer to mqtt client structure (mqtt_client_t).
 * @retval size_t          : Length of disconnect message.
 */
size_t mqtt_disconnect(mqtt_client_t *client)
{

	size_t message_length = 0;

	client->disconnect_msg->fixed_header.message_type = MQTT_DISCONNECT_MESSAGE;

	message_length = sizeof(mqtt_disconnect_t);

	client->disconnect_msg->fixed_header.message_length = (uint8_t)(message_length - FIXED_HEADER_LENGTH);

	return message_length;
}




/*
 * @brief  Configures mqtt SUBSCRIBE message structure.
 * @param  *client          : pointer to mqtt client structure (mqtt_client_t).
 * @param  *subscribe_topic : subscribe topic name
 * @param  subscribe_qos    : Quality of service value (1:At-least once, 2:Exactly once)
 * @param  *message_id      : pointer to the message id variable
 * @retval size_t           : length of subscribe control packet, fail = 0;
 */
size_t mqtt_subscribe(mqtt_client_t *client, char *subscribe_topic, mqtt_qos_t subscribe_qos, uint16_t *message_id)
{

	size_t  func_retval             = 0;
	size_t  message_length          = 0;
	uint8_t subscribe_topic_length  = 0;

	subscribe_topic_length = (uint8_t)strlen(subscribe_topic);

	if(client == NULL || subscribe_topic == NULL)
	{
		func_retval = MAIN_FUNC_ERROR;
	}
	else
	{
		client->subscribe_msg->fixed_header.message_type = MQTT_SUBSCRIBE_MESSAGE;
		client->subscribe_msg->fixed_header.qos_level = subscribe_qos;

		/* TODO Increment message id if object is same */
		*message_id = *message_id + 1;

		client->subscribe_msg->message_identifier = mqtt_htons(*message_id);

		client->subscribe_msg->topic_length = mqtt_htons(subscribe_topic_length);

		strncpy(client->subscribe_msg->payload, subscribe_topic, subscribe_topic_length);

		client->subscribe_msg->payload[subscribe_topic_length] = subscribe_qos;

		message_length = FIXED_HEADER_LENGTH + SUBSCRIBE_MESSAGE_ID_SIZE + SUBSCRIBE_TOPIC_LENGTH_SIZE + subscribe_topic_length + SUBSCRIBE_QOS_SIZE;

		client->subscribe_msg->fixed_header.message_length = message_length - FIXED_HEADER_LENGTH;

		func_retval = message_length;

	}
	return func_retval;
}



/*
 * @brief  Read MQTT PUBLISH message
 * @param  *client           : pointer to mqtt client structure (mqtt_client_t).
 * @param  *subscribe_topic  : subscribe topic name received from the broker
 * @param  *received_message : message received from topic subscribed to
 * @param  *message_status   : pointer to message status.
 * @retval size_t            : length of received message, fail = 0;
 */
size_t mqtt_read_publish(mqtt_client_t  *client, char *subscribed_topic, char *received_message, uint8_t *message_status)
{
	size_t received_message_length = 0;
	size_t func_retval             = 0;
	uint8_t received_topic_length  = 0;

	/* Check for error*/
	if(client == NULL || subscribed_topic == NULL || received_message == NULL)
	{
		func_retval = MAIN_FUNC_ERROR;
	}
	else
	{

		*message_status = client->publish_msg->fixed_header.qos_level;

		received_topic_length = mqtt_ntohs(client->publish_msg->topic_length);

		strncpy(subscribed_topic, client->publish_msg->payload, received_topic_length);

		strcpy(received_message, client->publish_msg->payload + received_topic_length);

		received_message_length = strlen(received_message);

		func_retval = received_message_length;

	}

	return func_retval;
}




/*
 * @brief  Configures mqtt PINGREQUEST message structure.
 * @param  *client         : pointer to mqtt client structure (mqtt_client_t).
 * @retval size_t          : Length of pingrequest message.
 */
size_t mqtt_pingreq(mqtt_client_t *client)
{

	size_t message_length = 0;

	client->pingrequest_msg->fixed_header.message_type = MQTT_PINGREQ_MESSAGE;

	message_length = sizeof(mqtt_pingreq_t);

	client->pingrequest_msg->fixed_header.message_length = (uint8_t)(message_length - FIXED_HEADER_LENGTH);

	return message_length;
}



