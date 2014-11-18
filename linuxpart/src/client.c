#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<string.h>

#define PORT 8888
#define MAXSIZE 1024

int main(int argc, char *argv[])
{
	int clientSocket;
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

	printf("Received %d data points\n", argc - 1);
	int i = 1;
	int j = 0;
	char comma = ',';
	while(i < argc)
	{
		strncpy(buffer + j, argv[i], strlen(argv[i]));
		j += strlen(argv[i]);
		if(i + 1 < argc)
		{
			strncpy(buffer + j, &comma, 1);
			j++;
		}
		i++;
	}
	buffer[j] = '\0';
	printf("Buffered Elements: %s\n", buffer);
	printf("Data points added to buffer\n");

	printf("Sending Data points to Server\n");
	if ((send(clientSocket, buffer, strlen(buffer),0)) == -1)
	{
		fprintf(stderr, "Failure sending message\n");
		close(clientSocket);
		exit(1);
	} else {
		printf("Succesfully send Data points to Server!\n");
	}
	close(clientSocket);
	
	return 0;
}
