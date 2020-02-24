/**
 ******************************************************************************
 * @file    mqtt_pub.c
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
#include "headers.h"



/* Main function */
int main(int argc, char **argv)
{

	/* Socket API related variable initializations */
	char   message[2000]     = {0};
	char   read_buffer[1500] = {0};

	/* MQTT client structure initializations */
	mqtt_client_t publisher;

	/* Client State machine related variable initializations */
	size_t  message_length      = 0;
	int8_t  retval              = 0;
	int8_t  message_status      = 0;
	uint8_t loop_state          = 0;
	uint8_t mqtt_message_state  = 0;


	/* MQTT message buffers */
	char *my_client_name   = "Sender|1990-adityamall";
	char user_name[]       = "device1.sensor";
	char pass_word[]       = "4321";

	char publish_message[100];

	/* Initialize client object */
	IotClient Publisher =
	{
			.connectServer = mqtt_broker_connect,
			.getCommands   = parse_command_line_args,
			.write         = write,
			.read          = read,
			.close         = close,
	};


	/* Initialize client handle */
	Publisher.returnValue = clientBegin(&Publisher);


	/* Get Input from user */
	Publisher.returnValue = Publisher.getCommands(&Publisher, argc, argv, publish_message);
	if(Publisher.returnValue < 0)
	{
		fprintf(stderr, "ERROR!!: Command Line Args Error: %d\n", Publisher.returnValue);
		exit(EXIT_FAILURE);
	}


	/* Connect to MQTT broker */
	Publisher.returnValue = clientConnect(&Publisher);
	if(Publisher.returnValue < 0)
	{
		fprintf(stderr, "ERROR!!: Client Connect Error: %d\n", Publisher.returnValue);
		exit(EXIT_FAILURE);
	}


	/* State machine initializations */
	loop_state = FSM_RUN;


	/* Update state to connect to send connect message */
	mqtt_message_state = mqtt_connect_state;


	/* MQTT Finite state machine */
	while(loop_state)
	{

		switch(mqtt_message_state)
		{

		case mqtt_idle_state:

			if(Publisher.debugRequest > 1)
				fprintf(stdout,"FSM Idle State\n");

			break;


		case mqtt_read_state:

			/* Read Message type from the socket read buffer (Non Blocking) */

			if(Publisher.debugRequest > 1)
				fprintf(stdout,"FSM Read State\n");

			memset(read_buffer, 0, sizeof(read_buffer));

			/* Check for keep alive time for subscriber */
			while( ( Publisher.read(Publisher.socketDescriptor, read_buffer, 1500) ) < 0 );

			publisher.message = (void*)read_buffer;

			/* get MQTT message type and update state */
			mqtt_message_state = get_mqtt_message_type(&publisher);
			if(!mqtt_message_state)
			{
				mqtt_message_state = mqtt_disconnect_state;

			}

			break;


		case mqtt_connect_state:

			/* Fill mqtt CONNECT message structure */

			memset(message, '\0', sizeof(message));

			publisher.connect_msg = (void *)message;

			/* Setup User name password (optional) */
			retval = mqtt_client_username_passwd(&publisher, user_name, pass_word);
			if(retval == -1)
			{
				fprintf(stdout,"Bad value of user name or password\n");

				mqtt_message_state = mqtt_exit_state;

				break;
			}


			/* Set connect options */
			retval = mqtt_connect_options(&publisher, MQTT_CLEAN_SESSION, (uint8_t)Publisher.messageRetain, (mqtt_qos_t)Publisher.qualityOfService);
			if(retval == -1)
			{
				fprintf(stdout,"Bad value of connect options params\n");

				mqtt_message_state = mqtt_exit_state;

				break;
			}


			/* Setup MQTT CONNECT Message  */

			message_length = mqtt_connect(&publisher, my_client_name, (int16_t)Publisher.keepAliveTime);

			retval = Publisher.write(Publisher.socketDescriptor, (char*)publisher.connect_msg, message_length);
			if(retval < 0)
			{
				printf("write error, Socket closed by server\n");

				mqtt_message_state = mqtt_exit_state;

				break;
			}

			/* Print debug message */
			if(Publisher.debugRequest > 0)
				fprintf(stdout, "%s :Sending CONNECT\n", my_client_name);

			/* Update state */
			mqtt_message_state = mqtt_read_state;


			break;


		case mqtt_connack_state:

			/* @brief print debug message */
			if(Publisher.debugRequest > 0)
				fprintf(stdout,"%s :Received CONNACK\n", my_client_name);

			/* Check return code of CONNACK message */
			publisher.connack_msg = (void *)read_buffer;

			mqtt_message_state = get_connack_status(&publisher);

			break;


		case mqtt_publish_state:


			/* Fill mqtt PUBLISH message structure */
			memset(message, '\0', sizeof(message));

			publisher.publish_msg = (void *)message;

			/*Configure publish options */
			message_status = mqtt_publish_options(&publisher, Publisher.messageRetain, Publisher.qualityOfService);
			if(message_status == -1)
			{
				fprintf(stdout, "publish options param error\n");

				mqtt_message_state = mqtt_disconnect_state;

				break;
			}

			/* Configure publish message */
			message_length = mqtt_publish(&publisher, Publisher.topicName, publish_message);
			if(message_length == 0)
			{
				fprintf(stdout,"publish message param error\n");

				mqtt_message_state = mqtt_disconnect_state;

				break;
			}

			Publisher.write(Publisher.socketDescriptor, (char*)publisher.publish_msg, message_length);

			/* print debug message */
			if(Publisher.debugRequest > 0)
				fprintf(stdout, "%s :Sending PUBLISH(\"%s\",...(%ld bytes))\n", my_client_name, Publisher.topicName, strlen(publish_message));

			/* Update State according to quality of service */
			if(message_status == MQTT_QOS_ATLEAST_ONCE || message_status == MQTT_QOS_EXACTLY_ONCE)
			{
				mqtt_message_state = mqtt_read_state;
			}
			else
			{
				mqtt_message_state =  mqtt_disconnect_state;
			}

			break;



		case mqtt_puback_state:

			if(Publisher.debugRequest > 0)
				printf("%s :Received PUBACK\n",my_client_name);

			mqtt_message_state =  mqtt_disconnect_state;

			break;



		case mqtt_pubrec_state:

			if(Publisher.debugRequest > 0)
				printf("%s :Received PUBREC\n",my_client_name);

			mqtt_message_state = mqtt_pubrel_state;

			break;



		case mqtt_pubrel_state:

			/* Fill MQTT PUBREL message structure */
			memset(message, '\0', sizeof(message));

			publisher.pubrel_msg = (void *)message;

			message_length = mqtt_publish_release(&publisher);

			/* Send PUBREL message (Socket API) */
			Publisher.write(Publisher.socketDescriptor, (char*)publisher.pubrel_msg, message_length);

			if(Publisher.debugRequest > 0)
				fprintf(stdout,"%s :Sending PUBREL\n",my_client_name);

			mqtt_message_state = mqtt_read_state;

			break;


		case mqtt_pubcomp_state:

			if(Publisher.debugRequest > 0)
				fprintf(stdout,"%s :Received PUBCOMP\n",my_client_name);

			mqtt_message_state =  mqtt_disconnect_state;

			break;


		case mqtt_disconnect_state:

			/* Fill DISCONNECT structure */
			memset(message,'\0',sizeof(message));

			publisher.disconnect_msg = (void*)message;

			message_length = mqtt_disconnect(&publisher);

			/* Send Disconnect Message */
			Publisher.write(Publisher.socketDescriptor, (char*)publisher.disconnect_msg, message_length);

			/* brief print debug message */
			if(Publisher.debugRequest == 1)
				fprintf(stdout,"%s :Sending DISCONNECT\n",my_client_name);

			/* Update State */
			mqtt_message_state = mqtt_exit_state;

			break;


		case mqtt_exit_state:

			if(Publisher.debugRequest > 1)
				fprintf(stdout,"FSM Exit state \n");

			/* Suspend while loop */
			loop_state = FSM_SUSPEND;

			/* Deinit Client */
			clientEnd(&Publisher);

			break;

		default:
			break;

		}

	}

	if(Publisher.debugRequest)
		fprintf(stdout,"Exited FSM \n");

	return 0;
}


