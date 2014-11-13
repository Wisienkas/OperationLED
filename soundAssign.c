#include<stdio.h>
#include<stdlib.h>
#include<x86_64-linux-gnu/sys/socket.h>
#include<x86_64-linux-gnu/sys/types.h>
#include<netinet/in.h>
#include<string.h>

#define PORT 8888
#define MAXSIZE 256


int main(int argc, char *argv[])
{
	int clientSocket, num;
	char buffer[MAXSIZE];
	struct sockaddr_in serverAddr;
	socklen_t addr_size;

	/* Create the socket. The three arguments are
	 *  1) Internet domain
	 *  2) Stream socket
	 *  3) default protocol (TCP in this case 
	 */
	clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	/* Configure settings of the server address struct
	 * Address family = internet */
	serverAddr.sin_family = AF_INET;
	/* Set port number using htons function to use proper byte order */
	serverAddr.sin_port = htons(8888);
	/* Set IP address to localhost */
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	/* Set all the bits of the padding fields to 0 */
	memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

	/* Connect the socket to the server using the address struct */
	addr_size = sizeof(serverAddr);
	connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);
	int i = 1;
	while(i < argc)
	{
		memcpy(buffer, argv[i], sizeof(argv[i]));
		if ((send(clientSocket, buffer, strlen(buffer),0)) == -1)
		{
			fprintf(stderr, "Failure sending message\n");
			close(clientSocket);
			exit(1);
		} else 
		{
			printf("Client: Message being sent: %s\n", buffer);
			num = recv(clientSocket, buffer, sizeof(buffer), 0);
			if(num <= 0)
			{
				printf("Either Connection closed or Error\n");
				break;
			}
			printf("Client: Message received from Server - %s\n", buffer);
			
		}
		i++;
	}
	close(clientSocket);
	return 0;
}

