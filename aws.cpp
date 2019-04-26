#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

using namespace std;

#define AWS_TCP_PORT_FOR_CLIENT 24222
#define AWS_TCP_PORT_FOR_MONITOR 25222
#define AWS_UDP_PORT 23222
#define SERVER_A_PORT 21222
#define SERVER_B_PORT 22222
#define MAXLENGTH 1024 
#define LOCAL_IP_ADDRESS "127.0.0.1"
int buildUDPConnection(int serverPort, int localPort, string request, char returnVal[], string option, int destinationPort);
int intialTCPConnection(int localPort);
int main(){
	printf("The AWS is up and running\n");
	char buffer[MAXLENGTH];
	string request;
	string response;
	const string WRITE = "write";
	const string SEARCH = "search";
	const string COMPUTE = "compute";
	int socketDescriptorForClient = intialTCPConnection(AWS_TCP_PORT_FOR_CLIENT);
	int socketDescriptorForMonitor = intialTCPConnection(AWS_TCP_PORT_FOR_MONITOR);
	int childSocketForClient, childSocketForMonitor, requestCount;
	struct sockaddr_in childAddressForClient;
	struct sockaddr_in childAddressForMonitor;
	memset(&childAddressForClient, 0, sizeof(childAddressForClient));
	memset(&childAddressForMonitor, 0, sizeof(childAddressForMonitor));
	int lenMonitor = sizeof(childAddressForMonitor);
	if((childSocketForMonitor = accept(socketDescriptorForMonitor, (struct sockaddr *) &childAddressForMonitor, (socklen_t *) &lenMonitor)) < 0){
		exit(EXIT_FAILURE);
	}
	while(true){
		request.clear();
		response.clear();
		int lenClient = sizeof(childAddressForClient);
		if((childSocketForClient = accept(socketDescriptorForClient, (struct sockaddr *) &childAddressForClient, (socklen_t *) &lenClient)) < 0){
			exit(EXIT_FAILURE);
		}
		requestCount = read(childSocketForClient, buffer, MAXLENGTH);
		buffer[requestCount] = '\0';
		request = buffer;
		if(request.find(COMPUTE) != string::npos){
			printf("The AWS received operation <%s> from the client using TCP over port <%d>\n", COMPUTE.c_str(), AWS_TCP_PORT_FOR_CLIENT);
			send(childSocketForMonitor, buffer, MAXLENGTH, 0);
			printf("The AWS sent operation <%s> and arguments to the monitor using TCP over port <%d>\n", COMPUTE.c_str(), AWS_TCP_PORT_FOR_MONITOR);
			string requestA = SEARCH + request.substr(request.find(":"), request.find(" ") - request.find(":"));
			char returnVal[MAXLENGTH];
			buildUDPConnection(SERVER_A_PORT, AWS_UDP_PORT, requestA, returnVal, COMPUTE, SERVER_A_PORT);
			string responseA = returnVal;
			if(responseA.find("0") != string::npos && responseA.size() == 1){
				printf("Link ID not found\n");
				response = "Link ID not found";
				response[response.size()] = '\0';
			}else{
				printf("The AWS received link information from Backend-Server A using UDP over port <%d>\n", AWS_UDP_PORT);
				string requestB = request.substr(request.find(":") + 1) + " " + responseA.substr(responseA.find(" ") + 1);
				char returnVal[MAXLENGTH];
				buildUDPConnection(SERVER_B_PORT, AWS_UDP_PORT, requestB, returnVal, COMPUTE, SERVER_B_PORT);
				string responseB = returnVal;
				printf("The AWS received outputs from Backend-Server B using UDP over port <%d>\n", AWS_UDP_PORT);
				response = request.substr(request.find(":") + 1, request.find(" ") - request.find(":") - 1) + ":" + responseB;
			}
			send(childSocketForMonitor, response.c_str(), response.size(), 0);
			printf("The AWS sent compute results to the monitor using TCP over port <%d>\n", AWS_TCP_PORT_FOR_MONITOR);
			send(childSocketForClient, response.c_str(), response.size(), 0);
			printf("The AWS sent result to client for operation <%s> using TCP over port <%d>\n", COMPUTE.c_str(), AWS_TCP_PORT_FOR_CLIENT);
		}else{
			printf("The AWS received operation <%s> from the client using TCP over port <%d>\n", WRITE.c_str(), AWS_TCP_PORT_FOR_CLIENT);
			send(childSocketForMonitor, buffer, MAXLENGTH, 0);
			printf("The AWS sent operation <%s> and arguments to the monitor using TCP over port <%d>\n", WRITE.c_str(), AWS_TCP_PORT_FOR_MONITOR);
			char returnVal[MAXLENGTH];
			buildUDPConnection(SERVER_A_PORT, AWS_UDP_PORT, request, returnVal, WRITE.c_str(), SERVER_A_PORT);
			printf("The AWS received response from Backend-Server A for writing using UDP over port <%d>\n", AWS_UDP_PORT);
			response = "The write operation has been completed successfully";
			response[response.size()] = '\0';
			send(childSocketForMonitor, response.c_str(), response.size(), 0);
			printf("The AWS sent write response to the monitor using TCP over port <%d>\n", AWS_TCP_PORT_FOR_MONITOR);
			send(childSocketForClient, response.c_str(), response.size(), 0);
			printf("The AWS sent result to client for operation <%s> using TCP over port <%d>\n", WRITE.c_str(), AWS_TCP_PORT_FOR_CLIENT);
		}
		close(childSocketForClient);
	}
	return 0; 
}

int buildUDPConnection(int serverPort, int localPort, string request, char returnVal[], string option, int destinationPort){
	int socketDescriptor;
	struct sockaddr_in serverAddress;
	struct sockaddr_in awsUDPAddress;
	if((socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		exit(EXIT_FAILURE); 
	}
	memset(&serverAddress, 0, sizeof(serverAddress));
	memset(&awsUDPAddress, 0, sizeof(awsUDPAddress));
	
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	serverAddress.sin_addr.s_addr = inet_addr(LOCAL_IP_ADDRESS);
		
	awsUDPAddress.sin_family = AF_INET;
	awsUDPAddress.sin_port = htons(localPort);
	awsUDPAddress.sin_addr.s_addr = inet_addr(LOCAL_IP_ADDRESS);
	if(bind(socketDescriptor, (struct sockaddr *) &awsUDPAddress, sizeof(awsUDPAddress)) < 0){
		exit(EXIT_FAILURE);
	}
	
	unsigned int len = sizeof(serverAddress);
	const char* message = request.c_str();
	sendto(socketDescriptor, message, strlen(message), 0, (const struct sockaddr *) &serverAddress, len);
	if(destinationPort == SERVER_A_PORT){
		printf("The AWS sent operation <%s> to Backend-Server A using UDP over port <%d>\n", option.c_str(), AWS_UDP_PORT);
	}else{
		char delimiters[] = " ";
		char *linkId = strtok((char *) message, delimiters);
		char *size = strtok(NULL, delimiters);
		char *power = strtok(NULL, delimiters);
		printf("The AWS sent link ID=<%s>, size=<%s>, power=<%s>, and link information to Backend-Server B using UDP over port <%d>\n", linkId, size, power, AWS_UDP_PORT);
	}
	int count = recvfrom(socketDescriptor, (char *) returnVal, MAXLENGTH, MSG_WAITALL, (struct sockaddr *) &serverAddress, &len);
	
	returnVal[count] = '\0';
	close(socketDescriptor);
	return count;
}

int intialTCPConnection(int localPort){
	int socketDescriptor;
	struct sockaddr_in awsTCPAddress;

	if((socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		exit(EXIT_FAILURE); 
	}

	memset(&awsTCPAddress, 0, sizeof(awsTCPAddress));
	
	awsTCPAddress.sin_family = AF_INET;
	awsTCPAddress.sin_port = htons(localPort);
	awsTCPAddress.sin_addr.s_addr = inet_addr(LOCAL_IP_ADDRESS);
	
	if(bind(socketDescriptor, (struct sockaddr *) &awsTCPAddress, sizeof(awsTCPAddress)) < 0){
		exit(EXIT_FAILURE); 
	} 
	if(listen(socketDescriptor, 1) < 0){ 
		exit(EXIT_FAILURE); 
	}
	return socketDescriptor;
}