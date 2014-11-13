#include <stdio.h>
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
}

struct values results[5];

int client_fd, socket_fd;

struct sockaddr_in server;
struct sockaddr_in dest;

socklen_t size;

char buffer[MAXSIZE];

int yes = 1;

void setup();
void loop();

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
