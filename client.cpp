#include <iostream>
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

#define MAXLENGTH 1024
#define AWS_TCP_PORT_FOR_CLIENT 24222
#define LOCAL_IP_ADDRESS "127.0.0.1"

int main(int argc, char const *argv[]) {
	printf("The client is up and running\n");
	const string WRITE = "write";
	const string COMPUTE = "compute";
	string request;
	string response;
	string bandwidth;
	string length;
	string velocity;
	string noisePower;
	string linkId;
	string size;
	string power;
	string option = argv[1];
	if(option.find(WRITE) != string::npos){
		bandwidth = argv[2];
		length = argv[3];
		velocity = argv[4];
		noisePower = argv[5];
		request = option + ":" + bandwidth + " " + length + " " + velocity + " " + noisePower;
	}else{
		linkId = argv[2];
		size = argv[3];
		power = argv[4];
		request = option + ":" + linkId + " " + size + " " + power;
	}
	int socketDescriptor;
	struct sockaddr_in awsTCPAddress;
	char buffer[MAXLENGTH];
	
	if((socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		return -1;
	}
	
	memset(&awsTCPAddress, 0, sizeof(awsTCPAddress));
	
	awsTCPAddress.sin_family = AF_INET;
	awsTCPAddress.sin_port = htons(AWS_TCP_PORT_FOR_CLIENT);
	if(inet_pton(AF_INET, LOCAL_IP_ADDRESS, &awsTCPAddress.sin_addr) <= 0){
		return -1;
	}
   	
	int len = sizeof(awsTCPAddress);
	if(connect(socketDescriptor, (struct sockaddr *) &awsTCPAddress, len) < 0){
		return -1;
	}
	send(socketDescriptor, request.c_str(), request.size(), 0);
	if(option.find(WRITE) != string::npos){
		printf("The client sent write operation to AWS\n");
	}else{
		cout << "The client sent ID=<" + linkId + ">, size=<" + size + ">, and power=<" + power + "> to AWS" << endl;
	}
	read(socketDescriptor, buffer, MAXLENGTH);
	response = buffer;
	if(response.find("successfully") != string::npos){
		cout << "The write operation has been completed successfully" << endl;
	}else if(response.find("not") != string::npos){
		cout << "Link ID not found" << endl;
	}else{
		string linkId = response.substr(0, response.find(":"));
		string delay = response.substr(response.find(":") + 1, response.find(" ") - response.find(":") - 1);
		cout << "The delay for link <" << linkId << "> is <" << delay << ">ms" << endl;
	}
	close(socketDescriptor);
	return 0;
}