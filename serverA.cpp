#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

#define SERVER_A_PORT 21222
#define MAXLENGTH 1024
#define LOCAL_IP_ADDRESS "127.0.0.1"

void write(string input);
string search(int linkId);
int getMaxLinkId();

int main() {
	const string WRITE = "write";
	const string SEARCH = "search";
	int socketDescriptor;
	char buffer[MAXLENGTH];
	struct sockaddr_in serverAAddress, awsAddress;
	string request;
	string response;
	  
	// Creating socket file descriptor 
	if((socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		exit(EXIT_FAILURE); 
	} 
	  
	memset(&serverAAddress, 0, sizeof(serverAAddress));
	memset(&awsAddress, 0, sizeof(awsAddress));
	   
	serverAAddress.sin_family = AF_INET;
	serverAAddress.sin_addr.s_addr = inet_addr(LOCAL_IP_ADDRESS); 
	serverAAddress.sin_port = htons(SERVER_A_PORT);

	if(bind(socketDescriptor, (const struct sockaddr *) &serverAAddress, sizeof(serverAAddress)) < 0){
		exit(EXIT_FAILURE); 
	}
	
	printf("The Server A is up and running using UDP on port <%d>\n", SERVER_A_PORT);
	int maxLinkId = getMaxLinkId();
	while(true){
		memset(buffer, 0, sizeof(buffer));
		request.clear();
		response.clear();
		unsigned int len = sizeof(awsAddress);
		int count;
		count = recvfrom(socketDescriptor, (char *) buffer, MAXLENGTH, MSG_WAITALL, (struct sockaddr *) &awsAddress, &len); 
		buffer[count] = '\0';
		request = buffer;
		if(request.find(WRITE) != string::npos){
			printf("The Server A received input for writing\n");
			maxLinkId++;
			stringstream ss;
			ss << maxLinkId;
			string input = ss.str();
			input += " " + request.substr(request.find(":") + 1);
			write(input);
			response = "1";
			sendto(socketDescriptor, response.c_str(), response.size(), 0, (const struct sockaddr *) &awsAddress, len);
			printf("The Server A wrote link <%d> to database\n", maxLinkId);
		}else{
			string linkId = request.substr(request.find(":") + 1);
			cout << "The Server A received input <" + linkId + "> for computing" << endl;
			if(atoi(linkId.c_str()) < 0 || atoi(linkId.c_str()) > maxLinkId){
				response = "0";
				sendto(socketDescriptor, response.c_str(), response.size(), 0, (const struct sockaddr *) &awsAddress, len);
				printf("Link ID not found\n");
			}else{
				response += "1:" + search(atoi(linkId.c_str()));
				sendto(socketDescriptor, response.c_str(), response.size(), 0, (const struct sockaddr *) &awsAddress, len);
				printf("The Server A finished sending the search result to AWS\n");
			}
		}
	}
	return 0;
}
void write(string input){
	ofstream outfile;
	outfile.open("database.txt", std::ios_base::app);
	outfile << input << endl;
	outfile.close();
}
string search(int linkId){
	string output;
	ifstream infile;
	infile.open("database.txt");
	infile.seekg(0, ios::beg);
	for(int i = 0; i < linkId; i++){
		getline(infile, output);
	}
	infile.close();
	return output;
}
int getMaxLinkId() { 
	int maxLinkId = 0;
	ifstream infile;
	infile.open("database.txt");
	infile.seekg(0, ios::beg);
	char line[MAXLENGTH];
	while(infile.getline(line, MAXLENGTH)) maxLinkId++;
	infile.close();
	return maxLinkId;
}