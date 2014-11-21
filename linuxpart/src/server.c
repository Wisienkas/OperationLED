// General libraries
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Socket libraries
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>

/* Server defines*/
#define PORT 8888
#define BACKLOG 10
#define MAXSIZE 1024

/* Collection data */
#define RESULT_SIZE_DEFAULT 20
#define RESULT_FIRST_INDEX 0

/* String defines*/
#define END_OF_LINE_CHAR 1
#define AFTER_LAST_COMMA 1

/* Parameter defines */
// this will work until Sat, 20 Nov 2286 17:46:39 GMT
#define UNIX_TIME_LENGTH 10
#define PARAMETER_F_COUNT 5
#define PARAMETER_F_PRECISION 6
#define PARAMETER_SPECIAL_CHARS 8

#define PARAMETER_BUFFERSIZE UNIX_TIME_LENGTH + PARAMETER_SPECIAL_CHARS + PARAMETER_F_PRECISION * PARAMETER_F_COUNT + END_OF_LINE_CHAR

/* Structs */
struct values 
{
	time_t time;
	float min;
	float max;
	float mean;
	float sd;
};

/* global variables */
struct values *results;
int r_index = 0;
int result_size = RESULT_SIZE_DEFAULT;

int client_fd, socket_fd, num;

struct sockaddr_in server;
struct sockaddr_in dest;

socklen_t size;

char server_buffer[MAXSIZE];

const int OPTION_ENABLED = 1;

/* Function declarations */
char *getParameter(int i, char *ptr);
char *addParameter(char *sql, int i);
char *substring(char *str, int start, int length);
void setup();
void loop();
void addData();
int countNumbers();
void sendData();
float * getNumbers();
float getMean(float *num, int n);
float getSd(float *num, int n);
float getMin(float *num, int n);
float getMax(float *num, int n);

int main(int argc, char *argv[]) 
{
	if(argc > 1)
	{
		result_size = atoi(argv[1]);
	}
	setup();
	loop();

	return 0;
}

void setup()
{
	// Initiates the result with the given size
	results = calloc(result_size, sizeof(struct values *));

	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "Socket Failure\n");
		exit(1);
	}

	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &OPTION_ENABLED, sizeof(int)) == -1)
	{
		perror("setsockopt\n");
		exit(1);
	}

	memset(&server, '0', sizeof(server));
	memset(&dest, '0', sizeof(dest));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htons(INADDR_ANY);
	server.sin_port = htons(PORT);

	if ((bind(socket_fd, (struct sockaddr*)&server, sizeof(struct sockaddr))) == -1)
	{
		fprintf(stderr, "Binding Failure\n");
		exit(1);
	}

	if ((listen(socket_fd, BACKLOG)) == -1)
	{
		fprintf(stderr, "Listening Failure\n");
		exit(1);
	}
}

void loop()
{
	while(1){
		size = sizeof(struct sockaddr_in);
		
		if ((client_fd = accept(socket_fd, (struct sockaddr *)&dest, &size)) == -1)
		{
			perror("accept");
			exit(1);
		}
		printf("Server got connection from client %s\n", inet_ntoa(dest.sin_addr));

		while(1) {

			if ((num = recv(client_fd, server_buffer, MAXSIZE, 0)) == -1) 
			{
				perror("recv");
				exit(1);
			}
			else if(num == 0) {
				printf("Connection closed\n");
				break;
			}
			server_buffer[num] = '\0';
			addData();
			printf("Server: Message Received %s\n", server_buffer);
		}
		close(client_fd);
	}
}

int countNumbers()
{
	int i = 0;
	int j = 0;
	while(server_buffer[j] != '\0')
	{
		if(server_buffer[j] == ',')
		{
			i++;
		}
		j++;
	}
	return i + AFTER_LAST_COMMA;
}

void addData()
{
	printf("Adding Data\n");
	int n = countNumbers();
	float *ptr = malloc(sizeof(float) * n);
	ptr = getNumbers(ptr);

	printf("r index: %d\n", r_index);
	(results + r_index)->time = time(NULL);
	(results + r_index)->mean = getMean(ptr, n);
	(results + r_index)->sd = getSd(ptr, n);
	(results + r_index)->min = getMin(ptr, n);
	(results + r_index)->max = getMax(ptr, n);

	printf("Data collected: %d of %d\n", r_index + 1, result_size);

	if(r_index >= result_size - 1)
	{
		sendData();
		r_index = RESULT_FIRST_INDEX;
		memset(results, 0, result_size * sizeof(struct values));
	} else {
		r_index++;
	}
}

char *substring(char *str, int start, int length)
{
	char *ptr = malloc(length + END_OF_LINE_CHAR);
	int c = 0;
	
	while ( c < start ) 
	{
		str++;
		c++;
	}
	
	c = 0;
	while ( c < length ) 
	{
		*(ptr + c) = *str;
		str++;
		c++;
	}

	*(ptr + c) = '\0';
	
	return ptr;
}

float * getNumbers(float * ptr){
	
	int i = 0;
	int n = 0;
	int start = 0;
	int length = 0;
	char *holder;
	while(server_buffer[i] != '\0')
	{
		if(server_buffer[i] == ','){
			
			holder = substring(server_buffer, start, length);
			ptr[n++] = (float)atoi(holder);
			start = i + 1;
			length = 0;
		} else {
			length++;
		}
		i++;
	}
	holder = substring(server_buffer, start, length);
	ptr[n++] = (float)atoi(holder);

	return ptr;
}

const char *cmd = "python /usr/bin/StoreToDb.py";

void sendData()
{

	int MAX_PARAMETER_SIZE = PARAMETER_BUFFERSIZE * result_size + END_OF_LINE_CHAR;
	char *ptrParameters = calloc(MAX_PARAMETER_SIZE , sizeof(char));
	int i = 0;
	while( i < result_size )
	{
		ptrParameters = addParameter(ptrParameters, i);
		i++;
	}
	
	char *syscmd = calloc((END_OF_LINE_CHAR + strlen(cmd) + strlen(ptrParameters)), sizeof(char));
	strcat(syscmd, strdup(cmd));
	strcat(syscmd, ptrParameters);
	
	printf("Calling command\n");
	printf("%s\n", syscmd);
	int returnVal = system(syscmd);
	if (returnVal != 0)
	{
	  printf("Command execution failed, no data was sent\n");
	}
	else
	{
	  printf("Command executed, data sent to database\n");
	}
	free(syscmd);
	printf("Pointer has been set free\n");
}

const int F_COUNT = PARAMETER_F_COUNT;
const int F_PRECISION = PARAMETER_F_PRECISION;
const int F_BUFFERSIZE = PARAMETER_F_PRECISION + END_OF_LINE_CHAR;
const int CHAR_PARAMETER_BUFFERSIZE = sizeof(char) * PARAMETER_BUFFERSIZE;
const char *FORMAT_PARAMETER = " %d '%s,%s,%s,%s' %s";

char *addParameter(char *ptrParameters, int i)
{
	printf("addParameter");
	
	char buffers[F_COUNT][F_PRECISION];
	memset(buffers, 0, sizeof(buffers));
	
	char *parameter = calloc(CHAR_PARAMETER_BUFFERSIZE, sizeof(char));
	
	snprintf(buffers[0], F_PRECISION, "%f", results[i].mean);
	snprintf(buffers[1], F_PRECISION, "%f", results[i].sd);
	snprintf(buffers[2], F_PRECISION, "%f", results[i].max);
	snprintf(buffers[3], F_PRECISION, "%f", results[i].min);
  
	if(snprintf(parameter, CHAR_PARAMETER_BUFFERSIZE, FORMAT_PARAMETER, 
	    results[i].time, buffers[0], buffers[1], buffers[2], buffers[3], buffers[0]) >= CHAR_PARAMETER_BUFFERSIZE)
	{
	  printf(" failed because of a buffer overflow\n");
	} 
	else 
	{
	  printf(" added \"%s\"\n", parameter);
	  strcat(ptrParameters, parameter);
	}
	
	return ptrParameters;
}

float getMean(float *num, int n)
{
	if(n == 0)
	{
		return n;
	}
	int i = 0;
	float result = 0;
	while( i < n )
	{
		result = result + num[i];
		i++;
	}
	return result / (float) n;
}

float getSd(float *num, int n)
{
	if(n == 0)
	{
		return n;
	}
	float mean = getMean(num, n);
	float deviation = 0;
	
	int i = 0;
	while( i < n )
	{
		deviation += (num[i] - mean) * 	(num[i] - mean);
		i++;
	}
	
	return sqrt( deviation / n);
}

float getMin(float *num, int n)
{
	if(n == 0)
	{
		return n;
	}
	int i = 0;
	float result = num[i++];
	while( i < n )
	{
		result = num[i] < result ? num[i] : result;
		i++;
	}
	return result;
	
}

float getMax(float *num, int n)
{
	int i = 0;
	if(n == 0)
	{
		return n;
	}
	float result = num[i];
	while( i < n )
	{
		result = num[i] > result ? num[i] : result;
		i++;
	}
	return result;
}
