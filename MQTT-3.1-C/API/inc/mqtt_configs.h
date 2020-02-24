/**
 ******************************************************************************
 * @file    mqtt_configs.h, file name will change
 * @author  Aditya Mall,
 * @brief   Example MQTT publish client, for mosquitto MQTT Broker
 *
 *  Info
 *          API
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2019 Aditya Mall, MIT License </center></h2>
 *
 * MIT License
 *
 * Copyright (c) 2019 Aditya Mall

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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ******************************************************************************
 */
#ifndef MQTT_CONFIGS_H_
#define MQTT_CONFIGS_H_



/******************************************************************************/
/*                                                                            */
/*                  MQTT Message Macros and Defines                           */
/*                                                                            */
/******************************************************************************/


/* @brief Generic Defines */
#define ENABLE  1
#define DISABLE 0
#define GCC     1


/* @brief MQTT defines */
#define FIXED_HEADER_LENGTH     2         /*!< Size of MQTT Fixed Header                                    */
#define PROTOCOL_NAME_LENGTH    6         /*!< Size of MQTT Protocol                                        */
#define PROTOCOL_NAME           "MQIsdp"  /*!< MQTT Protocol name                                           */
#define MQTT_PROTOCOL_VERSION   3         /*!< MQTT Protocol version                                        */
#define CLIENT_ID_LENGTH        23        /*!< MQTT client ID, fixed by mosquitto broker, do not change!    */
#define USER_NAME_LENGTH        15        /*!< MQTT username size, variable can be changed by user          */
#define PASSWORD_LENGTH         10        /*!< MQTT passwords size, variable can be changed by user         */
#define TOPIC_LENGTH            30        /*!< MQTT topic length size, variable can be changed by user      */
#define MESSAGE_LENGTH          100       /*!< MQTT message/payload length, variable can be changed by user */
#define MQTT_DEFAULT_KEEPALIVE  60

#endif /* INC_MQTT_CONFIGS_H_ */
