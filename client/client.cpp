#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#define PORT 8885
#define CLIENTPORT 8000
using namespace std;

string clientIP = "127.0.0.2";
int loginStatus = 0;
string userId;

void *newAccount(void *usercred) {
	string signUpData = (char *)usercred;
	string uname, pass;

	// int pos = signUpData.find(" ");
	// pass = signUpData.substr(pos+1);
	// uname = signUpData.substr(0, pos);
	string sendData = "new_account " + signUpData;

	char buff[1024];
	memset(&buff[0], 0, sizeof(buff));
	strcpy(buff, sendData.c_str());

	//Connect to Tracker
	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	// char buffer[1024];

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Tracker.\n");

	int cnt = 0;
	//Send DATA to server
	send(clientSocket, buff, strlen(buff), 0);
	memset(&buff[0], 0, sizeof(buff));

	if(strcmp(buff, ":exit") == 0){
		close(clientSocket);
		printf("[-]Disconnected from Tracker.\n");
		exit(1);
	}
	bzero(buff, 1024);
	// memset(&buff[0], 0, sizeof(buff));
	if(recv(clientSocket, buff, 1024, 0) < 0){
		printf("[-]Error in receiving data.\n");
	}else{
		printf("Tracker side: \t%s\n", buff);
	}
	pthread_exit(NULL);
	return NULL;
}

void *userLogin(void *usercred) {
	string signUpData = (char *)usercred;
	string uname, pass;

	int pos = signUpData.find(" ");
	// pass = signUpData.substr(pos+1);
	// userId = signUpData.substr(0, pos);
	string sendData = "login " + signUpData;

	char buff[1024];
	memset(&buff[0], 0, sizeof(buff));
	strcpy(buff, sendData.c_str());

	//Connect to Tracker
	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	// char buffer[1024];

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Tracker.\n");

	int cnt = 0;
	//Send DATA to server
	send(clientSocket, buff, strlen(buff), 0);
	memset(&buff[0], 0, sizeof(buff));

	// bzero(buff, 1024);
	// memset(&buff[0], 0, sizeof(buff));
	if(recv(clientSocket, buff, 1024, 0) < 0){
		printf("[-]Error in receiving data.\n");
	}else{
		printf("Tracker side: %s\n", buff);
		if(strcmp(buff,"User Logged In Successfully!!") == 0) {
			loginStatus = 1;
			string clientPort = to_string(CLIENTPORT);
			// cout << "CLPORT is " << CLIENTPORT;
			string clientInfo = clientPort + " " + clientIP;
			memset(&buff[0], 0, sizeof(buff));
			strcpy(buff, clientInfo.c_str());
			// printf("%s", buff);
			send(clientSocket, buff, 1024, 0);
			userId = signUpData.substr(0, pos);
		} else {
			loginStatus = 0;
		}
	}

	pthread_exit(NULL);
	return NULL;
}



void *createGrp(void *grpData) {
	string Data = (char *)grpData;

	string sendData = "newgrp " + Data;

	char buff[1024];
	memset(&buff[0], 0, sizeof(buff));
	strcpy(buff, sendData.c_str());

	//Connect to Tracker
	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	// char buffer[1024];

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Tracker.\n");

	int cnt = 0;
	//Send DATA to server
	send(clientSocket, buff, strlen(buff), 0);
	memset(&buff[0], 0, sizeof(buff));

	// bzero(buff, 1024);
	// memset(&buff[0], 0, sizeof(buff));
	if(recv(clientSocket, buff, 1024, 0) < 0){
		printf("[-]Error in receiving data.\n");
	}else{
		printf("Tracker response: %s\n", buff);
	}

	pthread_exit(NULL);
	return NULL;
}


void *joinGrp(void *grpData) {
	string Data = (char *)grpData;

	// int pos = Data.find(" ");
	// pass = signUpData.substr(pos+1);
	// string grpId = signUpData.substr(0, pos);

	string sendData = "joingrp " + Data;

	char buff[1024];
	memset(&buff[0], 0, sizeof(buff));
	strcpy(buff, sendData.c_str());

	//Connect to Tracker
	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	// char buffer[1024];

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Tracker.\n");

	int cnt = 0;
	//Send DATA to server
	send(clientSocket, buff, strlen(buff), 0);
	memset(&buff[0], 0, sizeof(buff));

	// bzero(buff, 1024);
	// memset(&buff[0], 0, sizeof(buff));
	if(recv(clientSocket, buff, 1024, 0) < 0){
		printf("[-]Error in receiving data.\n");
	}else{
		printf("Tracker response: %s\n", buff);
	}

	pthread_exit(NULL);
	return NULL;
}

void *listGrpJoinRequest(void *grpData) {
	string Data = (char *)grpData;

	// int pos = Data.find(" ");
	// pass = signUpData.substr(pos+1);
	// string grpId = signUpData.substr(0, pos);

	string sendData = "listrequest " + Data;

	char buff[1024];
	memset(&buff[0], 0, sizeof(buff));
	strcpy(buff, sendData.c_str());

	//Connect to Tracker
	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	// char buffer[1024];

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Tracker.\n");

	int cnt = 0;
	//Send DATA to server
	send(clientSocket, buff, strlen(buff), 0);
	memset(&buff[0], 0, sizeof(buff));

	// bzero(buff, 1024);
	// memset(&buff[0], 0, sizeof(buff));
	if(recv(clientSocket, buff, 1024, 0) < 0){
		printf("[-]Error in receiving data.\n");
	}else{
		printf("Tracker response: %s\n", buff);
	}

	pthread_exit(NULL);
	return NULL;
}


void getUserInput() {
	cout << "\nEnter your command: " << endl;
	string userinput;
	// cin >> cmd;
	getline(cin,userinput);
	pthread_t inputThread;
	char buff[1024];
	memset(&buff[0], 0, sizeof(buff));
	int pos = userinput.find(" ");
	if(userinput.substr(0, pos) == "create_account") {
		string str = userinput.substr(pos+1);
		strcpy(buff, str.c_str());

		pthread_create(&inputThread, NULL, newAccount, buff);
		usleep(1000);
	} else if(userinput.substr(0, pos) == "login") {

		string str = userinput.substr(pos+1);
		strcpy(buff, str.c_str());

		pthread_create(&inputThread, NULL, userLogin, buff);
		usleep(1000);
	} else if(userinput.substr(0, pos) == "create_group") {
		if(loginStatus) {
			string grpid = userinput.substr(pos+1);
			string str = grpid + " " + userId;
			strcpy(buff, str.c_str());
			pthread_create(&inputThread, NULL, createGrp, buff);
			usleep(1000);
		} else {
			cout << "Login first!!" << endl;
		}
	} else if(userinput.substr(0, pos) == "join_group") {
		if(loginStatus) {
			string grpid = userinput.substr(pos+1);
			string str = grpid + " " + userId;
			strcpy(buff, str.c_str());
			pthread_create(&inputThread, NULL, joinGrp, buff);
			usleep(1000);
		} else {
			cout << "Login first!!" << endl;
		}
	} else if(userinput.substr(0, pos) == "list_requests") {
		if(loginStatus) {
			string grpid = userinput.substr(pos+1);
			string str = grpid + " " + userId;
			strcpy(buff, str.c_str());
			pthread_create(&inputThread, NULL, listGrpJoinRequest, buff);
			usleep(1000);
		} else {
			cout << "Login first!!" << endl;
		}
	}
}

int main(){

	// landingPage();
	// callTracker();
	while(1) {
		getUserInput();
	}

	return 0;
}