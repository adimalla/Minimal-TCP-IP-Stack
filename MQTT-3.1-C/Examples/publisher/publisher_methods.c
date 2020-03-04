/**
 ******************************************************************************
 * @file    publisher_setup.c
 * @author  Aditya Mall,
 * @brief   Example MQTT publish client, for mosquitto MQTT Broker
 *
 *  Info
 *          Publisher App function source file
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

/* Header Files */
#include "headers.h"
#include "error_codes.h"

/* Macros */
#define MIN_ARGS_COUNT 2


#define HOST_MACHINE_FLAG        "--host"
#define HOST_MACHINE_FLAG_OPTNL  "-h"
#define TOPIC_FLAG               "--topic"
#define TOPIC_FLAG_OPTNL         "-t"
#define QOS_FLAG                 "--qos"
#define QOS_FLAG_OPTNL           "-q"
#define RETAIN_FLAG              "--retain"
#define RETAIN_FLAG_OPTNL        "-r"
#define MESSAGE_FLAG             "-m"
#define VERSION_FLAG             "--version"
#define HELP_FLAG                "--help"
#define KEEP_ALIVE_FLAG          "-k"
#define PORT_FLAG                "--port"
#define PORT_FLAG_OPTNL          "-p"
#define DEBUG_FLAG               "-d"
#define DEBUG_ALL_FLAG           "-dl"



/* Static Prototypes */
static void versionInfo(void);
static void errorHanlde(int errorNum);
static int help_info(char **argv);


/* Function Definitions */
static void errorHanlde(int errorNum)
{

	/* Print Error messages */
	switch(errorNum)
	{

	case NO_TOPIC_NAME:
		fprintf(stderr,"\nError!!: Publish Topic not entered through command line \n");
		break;

	case NO_MESSAGE_ERROR:
		fprintf(stderr,"\nError!!: Publish Message not entered through command line \n");
		break;

	case COMMAND_NO_ARGS:
		fprintf(stderr,"\nError!!: Both Publish Topic and Message not entered through command line \n");
		break;

	case BROKER_PORT_ERROR:
		fprintf(stderr,"\nError!!: No Connection port given through command line \n");
		break;

	case QOS_ERROR:
		fprintf(stderr,"\nError!!: Wrong QOS value given through command line \n");
		break;

	case KEEP_ALIVE_ERROR:
		fprintf(stderr,"\nError!!: Wrong Keep Alive Time value given through command line \n");
		break;

	case COMMAND_WRONG_ARGS:
		fprintf(stderr,"\nError!!: Wrong Arguments received through command line \n");
		break;

	}

	/* Print version info */
	versionInfo();

}




int mqtt_broker_connect(int *fd, int port, char *server_address)
{

	int func_retval = 0;

	struct sockaddr_in server;

	/* Get client socket type */
	if( (*fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		func_retval = CLIENT_SOCKET_ERROR;
	}
	else
	{
		server.sin_family = AF_INET;
		server.sin_port   = htons(port);

		server.sin_addr.s_addr = inet_addr(server_address);

		/* Connect to MQTT server host machine */
		if( ( connect(*fd, (struct sockaddr*)&server, sizeof(server)) ) < 0)
		{
			func_retval = CLIENT_CONNECT_ERROR;
		}
		else
		{
			fcntl(*fd, F_SETFL, O_NONBLOCK);

			func_retval = FUNC_CODE_SUCCESS;
		}
	}

	return func_retval;
}






static void versionInfo(void)
{
	printf("\n");
	printf("MQTT Publisher Application Version : %lf \n", APP_VERSION);
	printf("MQTT API Version                   : %lf \n", MQTT_API_VERSION);
	printf("MQTT Protocol Version              : %d \n", MQTT_PROTOCOL_VERSION);
	printf("\n");
}



static int help_info(char **argv)
{
	char *fileName;
	int  fileNameLen = 0;

	fileNameLen = strlen(argv[0]);

	fileName = malloc(sizeof(char) * fileNameLen - 2);

	/* Remove './' from front if the file name (Doesn't work in Debug/Run configs) */
	strcpy(fileName, (argv[0] + 2));

	printf("\n");

	printf("Usage : \"%s\" [-d Debug] [-dl Debug All] [-h hostaddr] [-k keepalive] [-p port] [-q qos] [-r retain] [-t topic] [-m message] \n", fileName);
	printf("\n");
	printf("        \"%s\" [--help] \n", fileName);

	printf("\n");
	printf("\n");

	printf("  -d         : Print debug messages on STDOUT                                                \n");
	printf("  -dl        : Print all debug messages on STDOUT                                            \n");
	printf("  -h,--host  : Host Address, address of the broker/server                                    \n");
	printf("  -k         : Keep Alive Time, keep alive time for the client to be connected to the server \n");
	printf("  -p,--port  : Port Number, port number on which broker/server is listening                  \n");
	printf("  -q,--qos   : Quality Of Service, quality of service level if client (0, 1, 2)              \n");
	printf("  -r         : Retain Message, for retaining the published messaged at the broker/server     \n");
	printf("  -t,--topic : Message Topic, topic of the messaged published by the client                  \n");
	printf("  -m         : Published Message, message to be published by the client                      \n");

	printf("\n");
	printf("\n");

	printf("Default Values : (if flags not given) :-                  \n");
	printf("-h             : Loopback 127.0.0.1                       \n");
	printf("-k             : 60 Seconds                               \n");
	printf("-q             : qos = 0 (Fire and Forget)                \n");
	printf("-p             : Port 1883 (Default for Mosquitto Broker) \n");

	printf("\n");

	exit(EXIT_SUCCESS);

	return 0;
}



static int args_check(int index, char **argv)
{
	int i = index;

	return !( strcmp(argv[i+1], HOST_MACHINE_FLAG) && strcmp(argv[i+1], TOPIC_FLAG) && strcmp(argv[i+1], QOS_FLAG)  && strcmp(argv[i+1], RETAIN_FLAG) && \
			strcmp(argv[i+1], HOST_MACHINE_FLAG_OPTNL) && strcmp(argv[i+1], TOPIC_FLAG_OPTNL) && strcmp(argv[i+1], QOS_FLAG_OPTNL) && \
			strcmp(argv[i+1], RETAIN_FLAG_OPTNL) && strcmp(argv[i+1], VERSION_FLAG) && strcmp(argv[i+1], HELP_FLAG) && strcmp(argv[i+1], KEEP_ALIVE_FLAG) && \
			strcmp(argv[i+1], PORT_FLAG_OPTNL) && strcmp(argv[i+1], PORT_FLAG) && strcmp(argv[i+1], DEBUG_FLAG) && strcmp(argv[i+1], DEBUG_ALL_FLAG) && \
			strcmp(argv[i+1], MESSAGE_FLAG) && strcmp(argv[i+1], KEEP_ALIVE_FLAG));
}




int parse_command_line_args(IotClient *clientObj, int argc, char **argv, char *buffer)
{
	ClientRetVal func_retval = 0;
	int index = 0;

	int argumentMatch = 0;

	/* Check args count */
	if(argc < MIN_ARGS_COUNT)
	{
		func_retval = COMMAND_NO_ARGS;
	}
	else
	{
		/* Search Args list 1st arg is the file name */
		for(index = 1; index < argc; index++)
		{

			if( (strcmp(argv[index], HOST_MACHINE_FLAG) == 0) || (strcmp(argv[index], HOST_MACHINE_FLAG_OPTNL) == 0) )
			{
				argumentMatch = 1;

				if(argv[index + 1] == NULL)
				{

					func_retval = COMMAND_NO_ARGS;

					break;
				}
				else if( args_check(index, argv) )
				{

					func_retval = COMMAND_WRONG_ARGS;

					break;
				}
				else
				{
					/* Check size */
					if(strlen(argv[index + 1]) <= MAX_ADDRESS_LENGTH || strlen(argv[index + 1]) > 0)
					{
						strcpy(clientObj->serverAddress, argv[index + 1]);
					}
					else
					{
						func_retval = ADDRESS_LENGTH_ERROR;

						break;
					}
				}
			}
			else if( (strcmp(argv[index], TOPIC_FLAG) == 0) || (strcmp(argv[index], TOPIC_FLAG_OPTNL) == 0) )
			{

				argumentMatch = 1;

				if(argv[index + 1] == NULL)
				{

					func_retval = NO_TOPIC_NAME;

					break;
				}
				else if( args_check(index, argv) )
				{

					func_retval = NO_TOPIC_NAME;

					break;
				}
				else
				{
					/* Check size */
					if(strlen(argv[index + 1]) > 0)
					{
						strcpy(clientObj->topicName, argv[index + 1]);
					}
					else
					{
						func_retval = TOPIC_LENGTH_ERROR;

						break;
					}
				}
			}
			else if( (strcmp(argv[index], QOS_FLAG) == 0) || (strcmp(argv[index], QOS_FLAG_OPTNL) == 0) )
			{

				argumentMatch = 1;

				if(argv[index + 1] == NULL)
				{

					func_retval = QOS_ERROR;

					break;
				}
				else if( args_check(index, argv) )
				{

					func_retval = QOS_ERROR;

					break;
				}
				else
				{

					/* Check size */
					if(strlen(argv[index + 1]) == 1)
					{
						clientObj->qualityOfService = atoi(argv[index + 1]);

						if(clientObj->qualityOfService > 2)
						{
							func_retval = QOS_ERROR;
							break;
						}

					}
					else
					{
						func_retval = QOS_ERROR;

						break;
					}
				}
			}
			else if( (strcmp(argv[index], RETAIN_FLAG) == 0) || (strcmp(argv[index], RETAIN_FLAG_OPTNL) == 0) )
			{

				argumentMatch = 1;

				clientObj->messageRetain = 1;


			}
			else if( (strcmp(argv[index], MESSAGE_FLAG) == 0) )
			{

				argumentMatch = 1;

				if(argv[index + 1] == NULL)
				{

					func_retval = NO_MESSAGE_ERROR;

					break;
				}
				else
				{

					/* Check size */
					if(strlen(argv[index + 1]) > 0)
					{
						strcpy(buffer, argv[index+1]);
					}
					else
					{
						func_retval = NO_MESSAGE_ERROR;

						break;
					}
				}

			}
			else if( (strcmp(argv[index], HELP_FLAG) == 0) )
			{

				argumentMatch = 1;

				func_retval = HELP_REQUEST;

				help_info(argv);

				break;
			}
			else if( (strcmp(argv[index], DEBUG_FLAG) == 0))
			{
				argumentMatch = 1;

				clientObj->debugRequest = 1;

				if(argv[index + 1] == NULL)
				{

					func_retval = COMMAND_NO_ARGS;

					break;
				}

			}
			else if ( (strcmp(argv[index], DEBUG_ALL_FLAG) == 0) )
			{
				argumentMatch = 1;

				clientObj->debugRequest = 2;

				if(argv[index + 1] == NULL)
				{

					func_retval = COMMAND_NO_ARGS;

					break;
				}

			}
			else if( (strcmp(argv[index], KEEP_ALIVE_FLAG) == 0))
			{

				argumentMatch = 1;

				if(argv[index + 1] == NULL)
				{

					func_retval = KEEP_ALIVE_ERROR;

					break;
				}
				else if( args_check(index, argv) )
				{

					func_retval = KEEP_ALIVE_ERROR;

					break;
				}
				else
				{

					/* Check size */
					if(strlen(argv[index + 1]) > 0)
					{
						clientObj->keepAliveTime = atoi(argv[index + 1]);

					}
					else
					{
						func_retval = KEEP_ALIVE_ERROR;

						break;
					}
				}
			}
			else if( (strcmp(argv[index], PORT_FLAG) == 0) || (strcmp(argv[index], PORT_FLAG_OPTNL) == 0))
			{

				argumentMatch = 1;

				if(argv[index + 1] == NULL)
				{

					func_retval = BROKER_PORT_ERROR;

					break;
				}
				else if( args_check(index, argv) )
				{

					func_retval = BROKER_PORT_ERROR;

					break;
				}
				else
				{

					/* Check size */
					if(strlen(argv[index + 1]) > 0)
					{
						clientObj->serverPortNumber = atoi(argv[index + 1]);

					}
					else
					{
						func_retval = BROKER_PORT_ERROR;

						break;
					}
				}
			}

		}/* Loop */

	}/* Else Condition */


	/* Set defaults if flags not given */
	if(strlen(clientObj->serverAddress) == 0)
	{
		strcpy(clientObj->serverAddress,"127.0.0.1");
	}

	if(clientObj->serverPortNumber == 0)
	{
		clientObj->serverPortNumber = DEFAULT_BROKER_PORT;
	}

	if(clientObj->keepAliveTime == 0)
	{
		clientObj->keepAliveTime = MQTT_DEFAULT_KEEPALIVE;
	}


	/* Handle Error */

	if(argumentMatch == 0)
		func_retval = COMMAND_WRONG_ARGS;

	if(func_retval != FUNC_CODE_SUCCESS)
	{
		errorHanlde(func_retval);
		help_info(argv);
	}

	return func_retval;
}






