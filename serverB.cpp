#include <iostream>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <fstream>
#include <math.h>
#include <sstream>

using namespace std;

#define SERVER_B_PORT 22222
#define MAXLENGTH 1024
#define LOCAL_IP_ADDRESS "127.0.0.1"

double getBitRate(double signalPower, double noisePower, double bandWidth);
double getPower(double power);
string format(double input);

int main(int argc, char *argv[]) {
	int socketDescriptor;
	char request[MAXLENGTH];
	string response;
	struct sockaddr_in serverBAddress, awsAddress;

	if((socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		exit(EXIT_FAILURE); 
	} 
		  
	memset(&serverBAddress, 0, sizeof(serverBAddress)); 
	memset(&awsAddress, 0, sizeof(awsAddress)); 

	serverBAddress.sin_family = AF_INET; // IPv4
	serverBAddress.sin_addr.s_addr = inet_addr(LOCAL_IP_ADDRESS);
	serverBAddress.sin_port = htons(SERVER_B_PORT);

	if(bind(socketDescriptor, (const struct sockaddr *) &serverBAddress, sizeof(serverBAddress)) < 0){
		exit(EXIT_FAILURE); 
	}
		
	printf("The Server B is up and running using UDP on port <%d>.\n", SERVER_B_PORT);
	while(true){
		memset(request, 0, sizeof(request));
		response.clear();
		unsigned int len = sizeof(awsAddress);
		int count;
		count = recvfrom(socketDescriptor, (char *) request, MAXLENGTH, MSG_WAITALL, (struct sockaddr *) &awsAddress, &len);
		request[count] = '\0';
		char delimiters[] = " ";
		char *linkId = strtok(request, delimiters);
		char *fileSizeStr = strtok(NULL, delimiters);
		double fileSize = atof(fileSizeStr); // bits
		char *signalPowerStr = strtok(NULL, delimiters);
		double signalPower = atof(signalPowerStr); // dBm
		double bandWidth = atof(strtok(NULL, delimiters)); // MHz
		double linkLength = atof(strtok(NULL, delimiters)); // km
		double velocity = atof(strtok(NULL, delimiters)); // km/s
		double noisePower = atof(strtok(NULL, delimiters)); // dBm
		printf("The Server B received link information: link <%s>, file size <%s>, and signal power <%s>\n", linkId, fileSizeStr, signalPowerStr);
		double bitRate = getBitRate(signalPower, noisePower, bandWidth);
		double transmissionDelay = (fileSize * 1000) / bitRate;
		double propagationDelay = (linkLength * 1000) / velocity;
		double endToEndDelay = (fileSize * 1000) / bitRate + (linkLength * 1000) / velocity;
		printf("The Server B finished the calculation for link <%s>\n", linkId);
		response = format(endToEndDelay) + " " + format(propagationDelay) + " " + format(transmissionDelay);
		sendto(socketDescriptor, response.c_str(), response.size(), 0, (const struct sockaddr *) &awsAddress, len);
		printf("The Server B finished sending the output to AWS\n");
	}
	return 0;
}
double getBitRate(double signalPower, double noisePower, double bandWidth){
	double tempSignalPower = getPower(signalPower);
	double tempNoisePower = getPower(noisePower);
	return bandWidth * pow(10, 6) * log2(1 + tempSignalPower / tempNoisePower);
}
double getPower(double power){
	return (pow(10, power / 10) / 1000);
}
string format(double input){
	double temp1 = input * 100 + 0.5;
	int temp2 = (int) temp1;
	double temp3 = (double) temp2;
	double temp4 = temp3 / 100;
	stringstream ss;
	ss << temp4;
	string result = ss.str();
	if(result.find(".") == string::npos){
		result += ".00";
	}else if(result.find(".") != string::npos && result.find(".") == result.size() - 2){
		result += "0";
	}
	ss.clear();
	return result;
}
