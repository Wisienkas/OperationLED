// General libraries
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

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

/* Structs */
struct values 
{
	float min;
	float max;
	float mean;
	float sd;
};

/* global variables */
struct values results[5];
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
			
			// Part to send to client
			//if ((send(client_fd, buffer, strlen(buffer), 0)) == -1)
			//{
			//	fprintf(stderr, "Failure Sending message\n");
			//	close(client_fd);
			//	break;
			//}

			//printf("Server: Message being sent: %s\nNumber of bytes sent: %lu\n",
			//		buffer, sizeof(buffer));
		}
		close(client_fd);
	}
}

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
	return i + 1;
}

void addData()
{
	printf("Adding Data\n");
	int n = countNumbers();
	float *ptr = malloc(sizeof(float) * n);
	ptr = getNumbers(ptr);

	results[r_index].mean = getMean(ptr, n);
	results[r_index].sd = getSd(ptr, n);
	results[r_index].min = getMin(ptr, n);
	results[r_index].max = getMax(ptr, n);

	printf("r_index: %d\n", r_index);
	//printf("Freeing ptr addData()\n");

	if(r_index == 4)
	{
		sendData();
		r_index = 0;
		memset(results, 0, sizeof(results));
	} else {
		r_index++;
	}
}

char *substring(char *str, int start, int length)
{
	char *ptr = malloc(length + 1);
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

void sendData()
{
	//MYSQL *conn;
	//MYSQL_RES *res;
	//MYSQL_ROW row;
	//MYSQL_STMT stmt;
	//MYSQL_BIND param[3], result[3];

	/*
	 *	REPLACE WITH HIS INFOMATIONS
	 *	server = jclarsen.dk
	 *	user = hwr
	 *	pass = hwr_e_14
	 *	database = hwr2014e_db
	 */	
	//char *server = "localhost";
	//char *user = "hwr";
	//char *pass = "12345678";
	//char *database = "hardware";
	
	//conn = mysql_init(NULL);
	/* Connect to database */
	//if (!mysql_real_connect(conn, server, user, pass, 
	//			database, 0, NULL, 0))
	//{
	//	fprintf(stderr, "%s\n", mysql_error(conn));
	//	exit(1);	
	//}
	char *sql = malloc(sizeof(char) * 1000);
	int i = 0;
	while( i < 5 )
	{
		sql = addMySQLParam(sql, i);
		i++;
	}
	printf("How query would look: \n%s \n", sql);
	
	char *syscmd = malloc(sizeof(char) * (1 + strlen(cmd) + strlen(sql)));
	strcat(syscmd, strdup(cmd));
	strcat(syscmd, sql);
	
	printf("Calling python script!\n");
	printf("%s\n", syscmd);
	system(syscmd);

	printf("data sent to database\n");
	free(syscmd);
	printf("Freeing pointer\n");
	//if (mysql_query(conn, sql))
	//{
	//	printf("Houston we have a problem");
	//	fprintf(stderr, "%s\n", mysql_error(conn));
	//	exit(1);
	//}

	/* closing connection */
	//mysql_free_result(res);
	//mysql_close(conn);

}

char *addMySQLParam(char *sql, int i)
{
	printf("addMySQLParam\n");
	char *sensor_name = calloc(1 + 45, sizeof(char));
	return strcat(sql, getMySQLValues(i, sensor_name));;
}

#define VALUES_F_COUNT 5
#define VALUES_F_PRECISION 6

const int F_COUNT = VALUES_F_COUNT;
const int F_PRECISION = VALUES_F_PRECISION;
const int F_BUFFERSIZE = VALUES_F_PRECISION + 1;
const int VALUES_BUFFERSIZE = sizeof(char) * (7 + VALUES_F_PRECISION * VALUES_F_COUNT + 1);
const char *parameter_format = " '%s,%s,%s,%s' %s";

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
  
	if(snprintf(values, VALUES_BUFFERSIZE, parameter_format, buffers[0], buffers[1], buffers[2], buffers[3], buffers[0]) >= VALUES_BUFFERSIZE)
	{
	  printf("Buffer overflow in getMySQLValues");
	}
	
	strcat(ptr, values);
	
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
