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
#define AWS_TCP_PORT_FOR_MONITOR 25222
#define LOCAL_IP_ADDRESS "127.0.0.1"
#define WRITE_OPTION "write"
#define COMPUTE_OPTION "compute"

int main(int argc, char const *argv[]) {
	printf("The monitor is up and running\n");
	int socketDescriptor;
	struct sockaddr_in awsTCPAddress;
	const char* message = "";
	char buffer[MAXLENGTH];
	
	if((socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		return -1;
	}
	
	memset(&awsTCPAddress, 0, sizeof(awsTCPAddress));
	
	awsTCPAddress.sin_family = AF_INET;
	awsTCPAddress.sin_port = htons(AWS_TCP_PORT_FOR_MONITOR);
	
	if(inet_pton(AF_INET, LOCAL_IP_ADDRESS, &awsTCPAddress.sin_addr) <= 0){
		return -1;
	}
   	
	int len = sizeof(awsTCPAddress);
	if(connect(socketDescriptor, (struct sockaddr *) &awsTCPAddress, len) < 0){
		return -1;
	}
	
	send(socketDescriptor, message, strlen(message), 0);
	while(1){
		memset(buffer, 0, sizeof(buffer));
		read(socketDescriptor, buffer, MAXLENGTH);
		string temp = buffer;
		if(temp.find("compute:") != string::npos){
			int index = 0;
			string option = temp.substr(index, temp.find(":"));
			index = temp.find(":") + 1;
			string linkId = temp.substr(index, temp.find(" ", index) - index);
			index = temp.find(" ", index) + 1;
			string size = temp.substr(index, temp.find(" ", index) - index);
			index = temp.find(" ", index) + 1;
			string power = temp.substr(index, temp.size() - index);
			cout << "The monitor received link ID=<" + linkId + ">, size=<" + size + ">, and power=<" + power + "> from the AWS" << endl;
		}else if(temp.find("write:") != string::npos){
			int index = 0;
			string option = temp.substr(index, temp.find(":"));
			index = temp.find(":") + 1;
			string bandwidth = temp.substr(index, temp.find(" ", index) - index);
			index = temp.find(" ", index) + 1;
			string length = temp.substr(index, temp.find(" ", index) - index);
			index = temp.find(" ", index) + 1;
			string velocity = temp.substr(index, temp.find(" ", index) - index);
			index = temp.find(" ", index) + 1;
			string noisePower = temp.substr(index, temp.size() - index);
			cout << "The monitor received BW=<" + bandwidth + ">, L=<" + length + ">, V=<" + velocity + "> and P=<" + noisePower + "> from the AWS" << endl;
		}else if(temp.find("successfully") != string::npos){
			cout << "The write operation has been completed successfully" << endl;
		}else if(temp.find("not") != string::npos){
			cout << "Link ID not found" << endl;
		}else if(temp.size() > 0){
			int index = 0;
			string linkId = temp.substr(index, temp.find(":"));
			index = temp.find(":") + 1;
			string endToEndDelay = temp.substr(index, temp.find(" ", index) - index);
			index = temp.find(" ", index) + 1;
			string propagationDelay = temp.substr(index, temp.find(" ", index) - index);
			index = temp.find(" ", index) + 1;
			string transmissionDelay = temp.substr(index, temp.size() - index);
			cout << "The result for link <" + linkId + ">: Tt = <" + transmissionDelay + ">ms, Tp = <" + propagationDelay + ">ms, Delay = <" + endToEndDelay + ">ms" << endl;
		}
		temp.clear();
	}
	return 0;
}