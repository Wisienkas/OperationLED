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

// Database
//#include <mysql.h>

/* Constants */
#define STRING_SIZE 35
#define PORT 8888
#define BACKLOG 10
#define MAXSIZE 1024
#define END_OF_LINE_CHAR 1
#define RESULT_SIZE 20

// this will work until Sat, 20 Nov 2286 17:46:39 GMT
#define VALUES_TIME_LENGTH 10
#define VALUES_F_COUNT 5
#define VALUES_F_PRECISION 6
#define SPECIAL_CHARS 8

#define PARAMETER_BUFFERSIZE VALUES_TIME_LENGTH + SPECIAL_CHARS + VALUES_F_PRECISION * VALUES_F_COUNT + END_OF_LINE_CHAR

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
struct values results[RESULT_SIZE];
int r_index = 0;

int client_fd, socket_fd, num;

struct sockaddr_in server;
struct sockaddr_in dest;

socklen_t size;

char buffer[MAXSIZE];

int yes = 1;

/* Function declarations */
char *getMySQLValues(int i, char *ptr);
char *addMySQLParam(char *sql, int i);
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
	setup();
	loop();

	return 0;
}

void setup()
{
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "Socket Failure\n");
		exit(1);
	}

	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
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

			if ((num = recv(client_fd, buffer, MAXSIZE, 0)) == -1) 
			{
				perror("recv");
				exit(1);
			}
			else if(num == 0) {
				printf("Connection closed\n");
				break;
			}
			buffer[num] = '\0';
			addData();
			printf("Server: Message Received %s\n", buffer);
		}
		close(client_fd);
	}
}

#define AFTER_LAST_COMMA 1

int countNumbers()
{
	int i = 0;
	int j = 0;
	while(buffer[j] != '\0')
	{
		if(buffer[j] == ',')
		{
			i++;
		}
		j++;
	}
	return i + AFTER_LAST_COMMA;
}

#define FIRST_RESULT_INDEX 0

const int MAX_RESULT_INDEX = RESULT_SIZE - 1;

void addData()
{
	printf("Adding Data\n");
	int n = countNumbers();
	float *ptr = malloc(sizeof(float) * n);
	ptr = getNumbers(ptr);

	results[r_index].time = time(NULL);
	results[r_index].mean = getMean(ptr, n);
	results[r_index].sd = getSd(ptr, n);
	results[r_index].min = getMin(ptr, n);
	results[r_index].max = getMax(ptr, n);

	printf("r_index: %d\n", r_index);

	if(r_index == MAX_RESULT_INDEX)
	{
		sendData();
		r_index = FIRST_RESULT_INDEX;
		memset(results, 0, sizeof(results));
	} else {
		r_index++;
	}
}

char *substring(char *str, int start, int length)
{
	char *ptr = malloc(length + END_OF_LINE_CHAR);
	int c = 0;

	if(ptr == NULL)
	{
		printf("Unable to allocate memory\n");
		exit(1);
	}

	while ( c < start ) 
	{
		str++;
		c++;
	}
	
	c = 0;
	while ( c < length ) 
	{
		*(ptr + c) = *str;
		*str++;
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
	while(buffer[i] != '\0')
	{
		if(buffer[i] == ','){
			
			holder = substring(buffer, start, length);
			ptr[n++] = (float)atoi(holder);
			start = i + 1;
			length = 0;
		} else {
			length++;
		}
		i++;
	}
	holder = substring(buffer, start, length);
	ptr[n++] = (float)atoi(holder);

	return ptr;
}

const char *cmd = "python /usr/bin/StoreToDb.py";

const int MAX_SQL_QUERY_SIZE = PARAMETER_BUFFERSIZE * RESULT_SIZE + END_OF_LINE_CHAR;

void sendData()
{
	char *sql = calloc(MAX_SQL_QUERY_SIZE, sizeof(char));
	int i = 0;
	while( i < RESULT_SIZE )
	{
		sql = addMySQLParam(sql, i);
		i++;
	}
	printf("How query would look: \n%s \n", sql);
	
	char *syscmd = calloc((END_OF_LINE_CHAR + strlen(cmd) + strlen(sql)), sizeof(char));
	strcat(syscmd, strdup(cmd));
	strcat(syscmd, sql);
	
	printf("Calling python script!\n");
	printf("%s\n", syscmd);
	system(syscmd);

	printf("data sent to database\n");
	free(syscmd);
	printf("Freeing pointer\n");
}


char *addMySQLParam(char *sql, int i)
{
	printf("addMySQLParam\n");
	char *sensor_name = calloc(PARAMETER_BUFFERSIZE + END_OF_LINE_CHAR, sizeof(char));
	return strcat(sql, getMySQLValues(i, sensor_name));;
}

const int F_COUNT = VALUES_F_COUNT;
const int F_PRECISION = VALUES_F_PRECISION;
const int F_BUFFERSIZE = VALUES_F_PRECISION + END_OF_LINE_CHAR;
const int VALUES_BUFFERSIZE = sizeof(char) * PARAMETER_BUFFERSIZE;
const char *parameter_format = " %d '%s,%s,%s,%s' %s";

char *getMySQLValues(int i, char *ptr)
{
	char buffers[F_COUNT][F_PRECISION];
	memset(buffers, 0, sizeof(buffers));
	
	char values[VALUES_BUFFERSIZE];
	memset(values, 0, sizeof(values));
	
	snprintf(buffers[0], F_PRECISION, "%f", results[i].mean);
	snprintf(buffers[1], F_PRECISION, "%f", results[i].sd);
	snprintf(buffers[2], F_PRECISION, "%f", results[i].max);
	snprintf(buffers[3], F_PRECISION, "%f", results[i].min);
  
	if(snprintf(values, VALUES_BUFFERSIZE, parameter_format, results[i].time, buffers[0], buffers[1], buffers[2], buffers[3], buffers[0]) >= VALUES_BUFFERSIZE)
	{
	  printf("Buffer overflow in getMySQLValues");
	} 
	else 
	{
		strcat(ptr, values);
	}
	
	
	return ptr;
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
