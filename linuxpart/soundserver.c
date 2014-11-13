#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <x86_64-linux-gnu/sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <x86_64-linux-gnu/sys/types.h>
#include <netdb.h>

#define PORT 8888
#define BACKLOG 10
#define MAXSIZE 256

struct values 
{
	float min;
	float max;
	float mean;
	float sd;
};

struct values results[5];
int r_index = 0;

int client_fd, socket_fd, num;

struct sockaddr_in server;
struct sockaddr_in dest;

socklen_t size;

char buffer[MAXSIZE];

int yes = 1;

char *substring(char *str, int start, int length);
void setup();
void loop();
void addData();
int countNumbers();
void sendData();
void getNumbers(float *numbers);
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
		fprintf(stderr, "Socket Failure");
		exit(1);
	}

	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("setsockopt");
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
	float *ptr;///////////////////
	printf("Getting numbers\n");
	getNumbers(ptr);
	printf("Counting observations\n");
	int n = countNumbers();

	printf("Setting results\n");

	results[r_index].mean = getMean(ptr, n);
	results[r_index].sd = getSd(ptr, n);
	results[r_index].min = getMin(ptr, n);
	results[r_index].max = getMax(ptr, n);

	printf("Freeing pointer\n");
	free(ptr);

	if(r_index == 5)
	{
		sendData();
		r_index = 0;
	} else {
		r_index++;
	}
}

char *substring(char *str, int start, int length)
{
	char *ptr;
	int c;

	ptr = malloc(length + 1);

	if(ptr == NULL)
	{
		printf("Unable to allocate memory\n");
		exit(1);
	}

	c = 0;
	while ( c < start - 1 ) 
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

void getNumbers(float *numbers){
	printf("Allocates memory for pointer\n");
	numbers = malloc(sizeof(float) * countNumbers());
	
	printf("Memory calculated\n");
	int i = 0;
	int n = 0;
	int start = 0;
	int length = 0;
	char *holder;
	while(buffer[i] != '\0')
	{
		if(buffer[i] == ','){
			printf("getting sub string\n");
			holder = substring(buffer, start, length);
			numbers[n++] = (float)atoi(holder);
		}
		i++;
	}
}

void sendData(float num[], int n)
{
	printf("Sending data to mysql server!\n");
		
}

float getMean(float *num, int nu)
{
	printf("Getting mean\n");
	if(nu == 0)
	{
		return nu;
	}
	int i = 0;
	float result = 0;
	printf("Adding up\n");
	while( i < nu )
	{
		exit(1);
		printf("adding index: %d with num %f\n", i, num[i]);
		result = result + num[i];
		i++;
	}
	printf("mean returning...\n");
	return result / (float) nu;
}

float getSd(float num[], int n)
{
	printf("Getting SD\n");
	if(n == 0)
	{
		return 0;
	}
	float mean = getMean(num, n);
	float deviation = 0;
	
	int i = 0;
	while( i < n )
	{
		deviation += (num[i] - mean) * 	(num[i] - mean);
	}
	
	return sqrt( deviation / n);
}

float getMin(float num[], int n)
{
	printf("Getting min\n");
	int i = 0;
	float result = 0;
	while( i < n )
	{
		result = num[i] < result ? num[i] : result;
	}
	return result;
	
}

float getMax(float num[], int n)
{
	printf("Getting max\n");
	int i = 0;
	if(n == 0)
	{
		return 0;
	}
	float result = num[i];
	while( i < n )
	{
		result = num[i] > result ? num[i] : result;
	}
	return result;
}




