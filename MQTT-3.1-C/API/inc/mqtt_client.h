/**
 ******************************************************************************
 * @file    mqtt_client.h
 * @author  Aditya Mall,
 * @brief   MQTT publish client API Header File for mosquitto MQTT Broker
 *
 *  Info
 *          MQTT API Header File
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

/* TODO test username password with broker*/

#ifndef MQQT_CLIENT_H_
#define MQQT_CLIENT_H_

/*
 * Standard Header and API Header files
 */
#include <stdint.h>
#include "mqtt_configs.h"


/******************************************************************************/
/*                                                                            */
/*                            Macro Defines                                   */
/*                                                                            */
/******************************************************************************/

/*MQTT API Version and Info Defines */
#define MQTT_API_VERSION  1.0
#define MQTT_VERSION      MQTT_PROTOCOL_VERSION

/* Avoid Structure padding */
#pragma pack(1)


/* State Machine defines */
#define FSM_RUN                   1               /*!< Value of Run state of finite state machine     */
#define FSM_SUSPEND               0               /*!< Value of Suspend state of finite state machine */
#define READ_STATE                16              /*!< Value of Read state of finite state machine    */
#define IDLE_STATE                17              /*!< Value of Idle state of finite state machine    */
#define EXIT_STATE                18              /*!< Value of Exit state of finite state machine    */


/* MQTT Header and Connect Options */ /*TODO header and connect options should be enum */
#define MQTT_MESSAGE_RETAIN       1               /*!< Retain mqtt message at server/broker       */ // message_retain
#define MQTT_MESSAGE_NO_RETAIN    0               /*!< Do not retain message at server/broker     */ // message_no_retain
#define MQTT_CLEAN_SESSION        1               /*!< Request a clean connect session            */ // connect_clean_session
#define MQTT_MESSASE_NULL         ""              /*!< Define for NULL Message, for clearing message retain */

typedef enum mqtt_qos
{
	MQTT_QOS_FIRE_FORGET  = 0,
	MQTT_QOS_ATLEAST_ONCE = 1,
	MQTT_QOS_EXACTLY_ONCE = 2,
	MQTT_QOS_RESERVED     = 3

}mqtt_qos_t;



/* @brief Defines for CONNECT Message */
#define MQTT_CONNECT_MESSAGE      1               /*!< MQTT Connect message identifier value */


/* @brief Defines for CONNACK Message */
#define MQTT_CONNACK_MESSAGE      2               /*!< MQTT Connack message identifier value     */
#define MQTT_CONNECTION_ACCEPTED  0               /*!< Connack message accepted identifier value */
#define MQTT_CONNECTION_REFUSED   2               /*!< Connack message refused identifier value  */


/* @brief Defines for PUBLISH Message */
#define MQTT_PUBLISH_MESSAGE      3               /*!< MQTT Publish message identifier value                     */
#define MQTT_TOPIC_LENGTH         TOPIC_LENGTH    /*!< Publish message topic length, mqtt_configs.h              */
#define PUBLISH_PAYLOAD_LENGTH    MESSAGE_LENGTH  /*!< Publish message payload message length,mqtt_configs.h     */
#define MQTT_MESSAGE_ID_OFFSET    2               /*!< Publish message, message ID length offset value           */
#define PUBLISH_NULL_MESSAGE      "\0"            /*!< Publish NULL message for clearing retain at broker/server */


/* PUBACK, PUBREC, PUBREL, PUBCOMP */
#define MQTT_PUBACK_MESSAGE       4               /*!< MQTT Puback message identifier value           */
#define MQTT_PUBREC_MESSAGE       5               /*!< MQTT Publish receive message identifier value  */
#define MQTT_PUBREL_MESSAGE       6               /*!< MQTT Publish release message identifier value  */
#define MQTT_PUBCOMP_MESSAGE      7               /*!< MQTT Publish complete message identifier value */


/* @brief Defines for Disconnect */
#define MQTT_DISCONNECT_MESSAGE   14              /*!< MQTT Publish message identifier value   */


/* Defines for MQTT SUBSRIBE, SUBACK message */
#define MQTT_SUBSCRIBE_MESSAGE    8               /*!< MQTT Subscribe message identifier value */
#define MQTT_SUBACK_MESSAGE       9               /*!< MQTT Suback message identifier value    */


/* Defines for MQTT PPINGREQ and PINGRESP */
#define MQTT_PINGREQ_MESSAGE      12              /*!< MQTT Subscribe message identifier value */
#define MQTT_PINRESP_MESSAGE      13              /*!< MQTT Suback message identifier value    */


/******************************************************************************/
/*                                                                            */
/*                  Data Structures for MQTT Control Messages                 */
/*                                                                            */
/******************************************************************************/


/* MQTT Fixed Header, common to all control messages */
typedef struct mqtt_header
{
	uint8_t retain_flag     : 1;  /*!< Message Retain Flag, (LSB)                                              */
	uint8_t qos_level       : 2;  /*!< Quality of service Level, (LSB)                                         */
	uint8_t dup_flag        : 1;  /*!< Duplicate Flag, (LSB)                                                   */
	uint8_t message_type    : 4;  /*!< MQTT Control Packet / Message Type, (MSB)                               */
	uint8_t message_length;       /*!< Length of MQTT Message (Remaining Length, excludes fixed header length) */

}mqtt_header_t;

/* MQTT Payload options */
typedef struct mqtt_payload_opts
{
	uint16_t             client_id_length;             /*!< Client ID Length                                  */
	char                 client_id[CLIENT_ID_LENGTH];  /*!< Client ID, length defined in mqtt_config.h        */
	uint16_t             user_name_length;             /*!< Client User Name Length                           */
	char                 user_name[USER_NAME_LENGTH];  /*!< Client User Name, length defined in mqtt_config.h */
	uint16_t             password_length;              /*!< Client Password Length                            */
	char                 password[PASSWORD_LENGTH];    /*!< Client Password, length defined in mqtt_config.h  */

}payload_opts_t;


/* @brief MQTT CONNECT structures */

/* Connect Flags bit fields */
typedef struct mqtt_connect_flags
{
	uint8_t reserved        : 1;  /*!< Reserved not set                                 */
	uint8_t clean_session   : 1;  /*!< Clean / New session, previous history is deleted */
	uint8_t will_flag       : 1;  /*!< Flag for enabling the qos and retain options     */
	uint8_t will_qos        : 2;  /*!< Enable qos (quality of service) option           */
	uint8_t will_retain     : 1;  /*!< Enable retain option                             */
	uint8_t password_flag   : 1;  /*!< Enable password option                           */
	uint8_t user_name_flag  : 1;  /*!< Enable user name option                          */

}mqtt_connect_flags_t;

/* Main MQTT Connect Structure */
typedef struct mqtt_connect
{
	mqtt_header_t        fixed_header;                         /*!< MQTT Fixed Header                               */
	uint16_t             protocol_name_length;                 /*!< MQTT Protocol Name Length                       */
	char                 protocol_name[PROTOCOL_NAME_LENGTH];  /*!< MQTT Protocol Name,length defined in configs.h  */
	uint8_t              protocol_version;                     /*!< MQTT Protocol Version                           */
	mqtt_connect_flags_t connect_flags;                        /*!< Connect Message Flags                           */
	int16_t              keep_alive_value;                     /*!< Client Keep Alive Value                         */
	char                 message_payload[MESSAGE_LENGTH];      /*!< Connect Pay-load field                          */
	payload_opts_t       payload_options;                      /*!< Connect Pay-load options, not sent to broker    */

}mqtt_connect_t;


/* @brief MQTT CONNACK structure */
typedef struct mqtt_connack
{
	mqtt_header_t fixed_header;  /*!< MQTT Fixed Header            */
	uint8_t       Reserved;      /*!< Reserved                     */
	uint8_t       return_code;   /*!< Connack Message return codes */

}mqtt_connack_t;


/* @brief MQTT PUBLISH structure */
typedef struct mqtt_publish
{
	mqtt_header_t fixed_header;              /*!< MQTT Fixed Header            */
	uint16_t      topic_length;              /*!< Publish message topic length */
	char          payload[MESSAGE_LENGTH];   /*!< publish message pay load     */

}mqtt_publish_t;


/* MQTT PUBREL structure  */
typedef struct mqtt_pubrel
{
	mqtt_header_t fixed_header;  /*!< MQTT Fixed Header                  */
	uint16_t      message_id;    /*!< Publish release message Identifier */

}mqtt_pubrel_t;



/* MQTT DISCONNECT structure */
typedef struct mqtt_disconnect
{
	mqtt_header_t fixed_header;  /*!< MQTT Fixed Header */

}mqtt_disconnect_t;



/* MQTT SUBSCRIBE structure */
typedef struct mqtt_subscribe
{
	mqtt_header_t fixed_header;            /*!< MQTT Fixed header            */
	uint16_t      message_identifier;      /*!< Subscribe message identifier */
	uint16_t      topic_length;            /*!< Subscribe topic length       */
	char          payload[MESSAGE_LENGTH]; /*!< Subscribe message payload    */

}mqtt_subscribe_t;



/* MQTT PINREQUEST structure */
typedef struct mqtt_pingreq
{
	mqtt_header_t fixed_header;  /*!< MQTT Fixed Header */

}mqtt_pingreq_t;



/* @brief MQTT client handle structure */
typedef struct mqtt_client_handle
{
	mqtt_header_t     *message;          /*!< Pointer to the fixed header structure       */
	mqtt_connect_t    *connect_msg;      /*!< Pointer to the connect message structure    */
	mqtt_connack_t    *connack_msg;      /*!< Pointer to the connack message structure    */
	mqtt_publish_t    *publish_msg;      /*!< Pointer to the publish message structure    */
	mqtt_pubrel_t     *pubrel_msg;       /*!< Pointer to the pubrel message structure     */
	mqtt_subscribe_t  *subscribe_msg;    /*!< Pointer to the subscribe structure          */
	mqtt_pingreq_t    *pingrequest_msg;  /*!< Pointer to the pingrequest structure        */
	mqtt_disconnect_t *disconnect_msg;   /*!< Pointer to the disconnect message structure */

}mqtt_client_t;



/* @brief MQTT State Machine message states */
typedef enum mqtt_message_states
{
	mqtt_idle_state         = IDLE_STATE,               /*!< State machine Idle State               */
	mqtt_read_state         = READ_STATE,               /*!< State machine read State               */
	mqtt_exit_state         = EXIT_STATE,               /*!< State machine exit state               */

	mqtt_connect_state      = MQTT_CONNECT_MESSAGE,     /*!< Connect message send state             */
	mqtt_connack_state      = MQTT_CONNACK_MESSAGE,     /*!< Connack message return code read state */
	mqtt_disconnect_state   = MQTT_DISCONNECT_MESSAGE,  /*!< Disconnect message send state          */

	mqtt_publish_state      = MQTT_PUBLISH_MESSAGE,     /*!< Publish message send state             */
	mqtt_puback_state       = MQTT_PUBACK_MESSAGE,      /*!< Puback message read state              */
	mqtt_pubrec_state       = MQTT_PUBREC_MESSAGE,      /*!< Pubrec message read state              */
	mqtt_pubrel_state       = MQTT_PUBREL_MESSAGE,      /*!< Pubrel message read state              */
	mqtt_pubcomp_state      = MQTT_PUBCOMP_MESSAGE,     /*!< Pubcomp message read state             */

	mqtt_subscribe_state    = MQTT_SUBSCRIBE_MESSAGE,   /*!< Subscribe message send state           */
	mqtt_suback_state       = MQTT_SUBACK_MESSAGE,      /*!< Suback message receive state           */

	mqtt_pingrequest_state  = MQTT_PINGREQ_MESSAGE,     /*!< Pingreq message send state             */
	mqtt_pingresponse_state = MQTT_PINRESP_MESSAGE      /*!< Pinresponse message receive state      */

}mqtt_message_states_t;



/******************************************************************************/
/*                                                                            */
/*                       API Function Prototypes                              */
/*                                                                            */
/******************************************************************************/



/*
 * @brief  Configures mqtt client user name and password.
 * @param  *client   : pointer to mqtt client structure (mqtt_client_t).
 * @param  user_name : mqtt client user name
 * @param  password  : mqtt client password
 * @retval int8_t    : 1 = Success, -1 = Error
 */
int8_t mqtt_client_username_passwd(mqtt_client_t *client, char *user_name, char *password);



/*
 * @brief  Configures mqtt CONNECT message structure.
 * @param  *client         : pointer to mqtt client structure (mqtt_client_t).
 * @param  client_name     : Name of the mqtt client given by user.
 * @param  keep_alive_time : Keep Alive time for the client.
 * @retval size_t          : Length of connect message.
 */
size_t mqtt_connect(mqtt_client_t *client, char *client_name, int16_t keep_alive_time);



/*
 * @brief  Configures mqtt connect options. (qos and retain don't have any effect on control packets currently)
 * @param  *client     : pointer to mqtt client structure (mqtt_client_t).
 * @param  session     : configure session type
 * @param  message_qos : configure quality of service
 * @param  retain      : configure mention retention at broker.
 * @retval int8_t      : 1 = Success, -1 = Error
 */
int8_t mqtt_connect_options(mqtt_client_t *client, uint8_t session, uint8_t retain, mqtt_qos_t message_qos);



/*
 * @brief  Returns the value of message type from input buffer.
 * @param  *client  : pointer to mqtt client structure (mqtt_client_t).
 * @retval  uint8_t : value of message type.
 */
uint8_t get_mqtt_message_type(mqtt_client_t *client);



/*
 * @brief  Returns the value of connack message status from input buffer.
 * @param  *client  : pointer to mqtt client structure (mqtt_client_t).
 * @retval  uint8_t : connack message return code.
 */
uint8_t get_connack_status(mqtt_client_t *client);



/*
 * @brief  Configures mqtt PUBLISH message options.
 * @param  *client        : pointer to mqtt client structure (mqtt_client_t).
 * @param  message_retain : Enable retain for message retention at broker
 * @param  message_qos    : Quality of service value (1:At-least once, 2:Exactly once)
 * @retval int8_t         : qos value = Success, -1 = Error
 */
int8_t mqtt_publish_options(mqtt_client_t *client, uint8_t message_retain, mqtt_qos_t message_qos);



/*
 * @brief  Configures mqtt PUBLISH message structure.
 * @param  *client          : pointer to mqtt client structure (mqtt_client_t).
 * @param  *publish_topic   : publish topic name
 * @param  *publish_message : message to be published
 * @retval size_t           : length of publish control packet.
 */
size_t mqtt_publish(mqtt_client_t *client, char *publish_topic, char *publish_message);



/*
 * @brief  Configures mqtt PUBREL message structure.
 * @param  *client         : pointer to mqtt client structure (mqtt_client_t).
 * @retval size_t          : Length of publish release message.
 */
size_t mqtt_publish_release(mqtt_client_t *client);



/*
 * @brief  Configures mqtt DISCONNECT message structure.
 * @param  *client : pointer to mqtt client structure (mqtt_client_t).
 * @retval size_t  : Length of disconnect message.
 */
size_t mqtt_disconnect(mqtt_client_t *client);



/*
 * @brief  Configures mqtt SUBSCRIBE message structure.
 * @param  *client          : pointer to mqtt client structure (mqtt_client_t).
 * @param  *subscribe_topic : subscribe topic name
 * @param  subscribe_qos    : Quality of service value (1:At-least once, 2:Exactly once)
 * @param  *message_id      : pointer to the message id variable
 * @retval size_t           : length of subscribe control packet, fail = 0;
 */
size_t mqtt_subscribe(mqtt_client_t *client, char *subscribe_topic, mqtt_qos_t subscribe_qos, uint16_t *message_id);



/*
 * @brief  Read MQTT PUBLISH message
 * @param  *client           : pointer to mqtt client structure (mqtt_client_t).
 * @param  *subscribe_topic  : subscribe topic name received from the broker
 * @param  *received_message : message received from topic subscribed to
 * @param  *message_status   : pointer to message status.
 * @retval size_t            : length of received message, fail = 0;
 */
size_t mqtt_read_publish(mqtt_client_t  *client, char *subscribed_topic, char *received_message, uint8_t *message_status);



/*
 * @brief  Configures mqtt PINGREQUEST message structure.
 * @param  *client         : pointer to mqtt client structure (mqtt_client_t).
 * @retval size_t          : Length of pingrequest message.
 */
size_t mqtt_pingreq(mqtt_client_t *client);



#endif /* MQQT_CLIENT_H_ */
