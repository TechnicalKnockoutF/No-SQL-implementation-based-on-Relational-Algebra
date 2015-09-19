/**
 * @susaname_assignment1
 * @author  Susana Dsa <susaname@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <sys/stat.h>
#include <ctype.h>

#include "../include/global.h"
#define inst_len 100
#define param_len 50
#define STDIN 0
#define SERVER_IP "XXX.XXX.XXX.XXX"
//#define char *myip;
#define MAXDATASIZE 1024

//Communication signals
char* SendServerMyClientDetails = "1000";
char* ReceiveFromServerUpdatedList = "2000";
//char* SendPeerMyDetails = "4000";
char *separator = ";";
char *endOfFile = "---xxx---";

char myPort[50];
char myIP[50];
int ServerSock; /* The socket file descriptor for our "listening" socket */
int ClientPeerListnerSock; /* The socket file descriptor for our "listening" socket */
int ClientToServerSock;
char *InstanceType;
char *InstancePort;
char *InstanceIP = "...";
int HighestSock;
int RegisteredWithServer;
int ClientCounter;
int ClientServerStarted;
int ServerListCount;
struct Client {
	int ID;
	char HostName[100];
	int sockfds;
	char IP[100];
	char Port[100];
	int UploadCounter;
	int DownloadCounter;
	unsigned long UploadBytes;
	unsigned long DownloadBytes;
	int flag;
};

struct Client ClientList[6];
struct Client PeerList[6];/* Array of connected sockets so we know who we are talking to */
fd_set master; /* Socket file descriptors we want to wake up for, using select() */
fd_set read_fds;
int highsock; /* Highest #'d file descriptor, needed for select() */
/* server address */
struct sockaddr_in serveraddr;
char *fnSelectCommandAction(char* inputCmd, char* portNumber);
void fnCreator();
void fnHelp();
void fnMyIP();
void fnMyPort(char* portNumber);
int fnRegister(char * ip, char *port);
int fnConnect(char * ip, char *port);
void fnTerminate(int sockId);
void fnExit();
void fnUpload(int receiverSockId, char *filename);
void fnDownload(int receiverSockId, char *filename);
void fnStatistics();
void fnErrorMsg();
char *ReadUserInput();
char *read_line(char *c);
void UpdateClientData(int k, char buf[4096]);
void UpdateDataFromServer(int k, char buf[4096]);
void UpdateClientHostname(int clientsockid, char *clientHostname);
void UpdateClientIP(int clientsockid, char *clientIP);
int UpdateClientPort(int clientsockid, char *clientPort);
int UpdateClientUploadCounter(int clientsockid, int count);
int UpdateClientDownloadCounter(int clientsockid, int count);
int UpdateClientUpdateByteCounter(int clientsockid, int count);
int UpdateClientDownloadByteCounter(int clientsockid, int count);
int FindClientInList(int clientsockid);
void ClientDataSender();
void UpdateListServer(int serversockid, char buf[4096]);
void AcceptUploadFile(int clientsockid, char buf[4096]);
void WriteFile(char *FileName, char *Data, int sockfd);
void UpdatePeerDetails(int clientsockid, char *clientIP);
int FindPeerInList(char* IP);
void FindHighestSock();
int RetrieveSocketID(int id);


/**
 * RetrieveSocketID function
 *
 *@param id client id
 *@return sockID socket id
 *
 */
int RetrieveSocketID(int id)
{
	int i;
	int sockID = -1;

	for(i=0; i<6;i++)
	{
		if(ClientList[i].ID == id)
		{
			return ClientList[i].sockfds;
		}
	}

	return sockID;
}

/**
 * UpdateListServer function
 *
 *@param serversockid socket id
 *@param buf data
 */
void UpdateListServer(int serversockid, char buf[4096]) {
	int i;

	const char s[4] = ";";
	const char endOfFile[10] = "---xxx---";
	char *token;
	char inputValue[10][100];
	char *tempToken;
	int count = -1;
	int countForTerms = 1;
	int j = 0;
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 100; j++) {
			inputValue[i][j] = '\0';
		}
	}

	//Ref: http://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
	//Modified code as per my need
	token = strtok(buf, s);

	if (strcmp(token, ReceiveFromServerUpdatedList) == 0) {
		/* walk through other tokens */
		while (token != NULL) {

			if ((count != -1) && (strcmp(token, endOfFile) != 0)) {
				ClientList[count].ID = count + 1;
				if (countForTerms == 1) {
					strcpy(ClientList[count].HostName, token);
				}

				if (countForTerms == 2) {
					strcpy(ClientList[count].IP, token);
				}

				if (countForTerms == 3) {
					strcpy(ClientList[count].Port, token);
					countForTerms = 1;
				}

				countForTerms++;
			}

			count++;
			token = strtok(NULL, s);
		}
	}

	ServerListCount = count;
}


/**
 * FindPeerInList function
 *
 *@param IP ip of the client
 *@return i client connection status
 */
int FindPeerInList(char* IP)
{
	int i = 0;
		//struct Client selectedClient;
		for (i = 0; i < 6; i++) {
			if (strcmp(ClientList[i].IP,IP)==0) {
				if(ClientList[i].flag != 0)
					return -1; //Client is already connected;
			}
		}
		return i;
}

/**
 * UpdateClientData function
 *
 *@param clientsockid socket id
 *@param  buf data
 */
void UpdateClientData(int clientsockid, char buf[4096]) {
//	struct Client selectedClient;
	int indexForStruct;
	indexForStruct = FindClientInList(clientsockid);

	//read buffer and extract information. call appropriate function from below to update respective fields.

	const char s[4] = ";";
	const char endOfFile[10] = "---xxx---";
	char *token;
	char inputValue[10][100];
	char *tempToken;
	int count = 0;
	int i = 0, j = 0;

	token = strtok(buf, s);
	if (strcmp(token, SendServerMyClientDetails) == 0) {
		/* walk through other tokens */
		while (token != NULL) {

			strcpy(inputValue[count], token);
			count++;
			token = strtok(NULL, s);
		}

		strcpy(ClientList[indexForStruct].HostName, inputValue[1]);
		strcpy(ClientList[indexForStruct].IP, inputValue[2]);
		strcpy(ClientList[indexForStruct].Port, inputValue[3]);
	}
}

/**
 * UpdateClientDataByServerSentDetails function
 *
 *@param clientsockid socket id
 *@param  buf data
 */
void UpdateClientDataByServerSentDetails(int clientsockid, char buf[4096]) {
//	struct Client selectedClient;
	int i;

	const char s[2] = ";";
	const char endOfFile[10] = "---xxx---";
	char *token;
	char inputValue[10][100];
	char *tempToken;
	int count = -1;
	int countForTerms = 1;
	int j = 0;

	token = strtok(buf, ";");

	if (strcmp(token, ReceiveFromServerUpdatedList) == 0) {
		/* walk through other tokens */
		while (token != NULL) {

			if (count == -1) {

			} else {
				if (strcmp(token, endOfFile) != 0) {

					if (countForTerms == 1) {
						ClientList[count].ID = atoi(token);

						printf(
								"\nClientList[count].ID : count = %d, value = %d\n",
								count, ClientList[count].ID);
						count--;
					}

					if (countForTerms == 2) {
						strcpy(ClientList[count].HostName, token);
						printf(
								"ClientList[count].HostName : count = %d, value = %s\n",
								count, ClientList[count].HostName);
						count--;
					}

					if (countForTerms == 3) {
						strcpy(ClientList[count].IP, token);
						printf(
								"ClientList[count].IP : count = %d, value = %s\n",
								count, ClientList[count].IP);
						count--;
					}

					if (countForTerms == 4) {
						strcpy(ClientList[count].Port, token);
						printf(
								"ClientList[count].Port : count = %d, value = %s\n",
								count, ClientList[count].Port);
						countForTerms = 1;
					}
				}
			}
			token = strtok(NULL, ";");
			count++;
			countForTerms++;
		}
	}
}

/**
 * AcceptUploadFile function
 *
 *@param clientsockid socket id
 *@param  buf data
 */
void AcceptUploadFile(int clientsockid, char buf[4096]) {
//http://stackoverflow.com/questions/26000251/large-file-transfer-error-in-socket-in-c
//	struct Client selectedClient;
	int indexForStruct;
	indexForStruct = 0;
	indexForStruct = FindClientInList(clientsockid);

	//read buffer and extract information. call appropriate function from below to update respective fields.

	const char s[4] = ";";
	const char endOfFile[10] = "---xxx---";
	char *token;
	char inputValue[10][100];
	char *tempToken;
	int count = 0;
	int i = 0, j = 0;
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 100; j++) {
			inputValue[i][j] = '\0';
		}
	}
	char *fileName;
	tempToken = strtok(buf, endOfFile);

	while (tempToken != NULL) {
		/* get the first token */
		token = strtok(tempToken, s);

		/* walk through other tokens */
		while (token != NULL) {
			strcpy(inputValue[count], token);
			if (count == 0) {

			}
			if (count == 1) {
				if (token != "") {
					fileName = token;
				}

			}
			if (count == 2) {
				if (atoi(token) == 0) {
					printf("Start Receiving %s from %d\n", fileName,
							clientsockid);
				}
			}
			if (count == 3) {
				WriteFile(fileName, token, clientsockid);
			}
			count++;
			token = strtok(NULL, s);
		}
		tempToken = strtok(NULL, endOfFile);
	}
}

/**
 * WriteFile function
 *
 *@param FileName filename to be written to
 *@param  Data data to be written
 *@param  sockfd socket id where data is to be sent
 */

void WriteFile(char *FileName, char *Data, int sockfd) {
	time_t WriteStart, ReceiveEnd;
	FILE *fr = fopen(FileName, "a");
	if (fr == NULL)
		printf("File %s Cannot be opened.\n", FileName);
	else {
		int fr_block_sz;
		char revbuf[512];
		bzero(revbuf, 512);
		fr_block_sz = 0;
		int readbytes;
		readbytes = 0;
		time(&WriteStart);
		int FileSize = 0;
		FileSize = atoi(Data);
		printf("Start Time %s\n", ctime(&WriteStart));
		while ((fr_block_sz = recv(sockfd, revbuf, 512, 0)) > 0) {
			readbytes = readbytes + fr_block_sz;
			int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
			if (write_sz < fr_block_sz) {
				error("File write failed.\n");
			}
			bzero(revbuf, 512);
			if (fr_block_sz == 0 || fr_block_sz != 512) {
				break;
			}
			if (readbytes == FileSize) {
				break;
			}
		}
		if (fr_block_sz < 0) {
			if (errno == EAGAIN) {
				printf("recv() timed out.\n");
			} else {
				fprintf(stderr, "recv() failed due to errno = %d\n", errno);
			}
		}
		time(&ReceiveEnd);
		printf("Received Time %s\n", ctime(&ReceiveEnd));
		double diff;
		diff = difftime(ReceiveEnd, WriteStart);
		printf("[PA01] File %s of %d bytes from Client was saved in %lf !\n",
				FileName, readbytes, diff);
		fclose(fr);
	}
}
/**
 * UpdateClientHostname function
 *
 *@param clientsockid holds socket id
 *@param  clientHostname Holds the host name to be updated
 */
void UpdateClientHostname(int clientsockid, char *clientHostname) {
	struct Client selectedClient;
	int i = 0;
	i = FindClientInList(clientsockid);

	strcpy(ClientList[i].HostName, clientHostname);
	printf("\nClient hostname is %s\n", ClientList[i].HostName);

}
/**
 * UpdateClientIP function
 *
 *@param clientsockid holds socket id
 *@param  clientIP Holds the IP to be updated
 */
void UpdateClientIP(int clientsockid, char *clientIP) {
	int i;
	i = ClientCounter;

	ClientList[i].sockfds = clientsockid;
	ClientList[i].ID = ClientCounter + 1;
	strcpy(ClientList[i].IP, clientIP);
	ClientList[i].UploadCounter = 0;
	ClientList[i].UploadBytes = 0;
	ClientList[i].DownloadCounter = 0;
	ClientList[i].DownloadBytes = 0;
}

/**
 * int FindClientUsingIP function
 *
 *
 *@param  clientIP Holds the IP to be updated
 *@return i returns the index for ClientList
 */
int FindClientUsingIP(char *clientIP) {
	int i = 0;

	for (i = 0; i < 6; i++) {
		if (strcmp(ClientList[i].IP,clientIP) ==0){
			break;
		}
	}

	return i;
}
/**
 * int UpdatePeerDetails function
 *
 *@param clientsockid socket ID
 *@param  clientIP Holds the IP to be updated
 *
 */
void UpdatePeerDetails(int clientsockid, char *clientIP) {
	int i = 0;

		for (i = 0; i < 6; i++) {
			if (strcmp(ClientList[i].IP,clientIP) ==0){
				ClientList[i].sockfds = clientsockid;
				ClientList[i].flag = 1;
				break;
			}
		}
}
/**
 * int UpdateClientPort function
 *
 *@param clientsockid socket ID
 *@param  clientPort Holds the port number to be updated
 *@return 0 on success
 */
int UpdateClientPort(int clientsockid, char *clientPort) {
//	struct Client selectedClient;
	int i = 0;
	i = FindClientInList(clientsockid);
	strcpy(ClientList[i].Port, clientPort);
	return 0;
}
/**
 * int UpdateClientUploadCounter function
 *
 *@param clientsockid socket ID
 *@param  count Holds the upload counter to be updated
 *@return 0 on success
 */
int UpdateClientUploadCounter(int clientsockid, int count) {
//	struct Client selectedClient;
	int i = 0;
	i = FindClientInList(clientsockid);
	ClientList[i].UploadCounter = count;
	return 0;
}
/**
 * int UpdateClientDownloadCounter function
 *
 *@param clientsockid socket ID
 *@param  count Holds the download counter to be updated
 *@return 0 on success
 */
int UpdateClientDownloadCounter(int clientsockid, int count) {
//	struct Client selectedClient;
	int i = 0;
	i = FindClientInList(clientsockid);
	ClientList[i].DownloadCounter = count;
	return 0;
}

/**
 * UpdateClientUpdateByteCounter function
 *
 *@param clientsockid socket ID
 *@param  count Holds the uploadbytes count to be updated
 *@return 0 on success
 */
int UpdateClientUpdateByteCounter(int clientsockid, int count) {
//	struct Client selectedClient;
	int i = 0;
	i = FindClientInList(clientsockid);
	ClientList[i].UploadBytes = count;
	return 0;
}

/**
 * UpdateClientDownloadByteCounter function
 *
 *@param clientsockid socket ID
 *@param  count Holds the downloadbytes count to be updated
 *@return 0 on success
 */

int UpdateClientDownloadByteCounter(int clientsockid, int count) {
//	struct Client selectedClient;
	int i = 0;
	i = FindClientInList(clientsockid);
	ClientList[i].DownloadBytes = count;
	return 0;
}
/**
 * FindClientInList function
 *
 *@param clientsockid socket ID
 *@return i returns the current index for ClientList
 */
int FindClientInList(int clientsockid) {
	int i = 0;
	//struct Client selectedClient;
	for (i = 0; i < 6; i++) {
		if (ClientList[i].sockfds == clientsockid) {
			return i;
		}
	}
	return i;
}
/**
 * RemoveClientFromList function
 *
 *@param clientsockid socket ID
 *@return 0 on success
 *Prevents the current socket ID from being retrieved
 */

int RemoveClientFromList(int clientsockid) {
	int i;
	for (i = 0; i < 6; i++) {
		if (ClientList[i].sockfds == clientsockid) {
			ClientList[i].flag = 0;
			ClientList[i].sockfds = 0;
		}
	}
	return 0;
}
/**
 * get_in_addr function
 *
 *@param sa sockaddr structure
 *
 */

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

/**
 * read_line function
 *
 *@param c contains the input data
 *Used to read data from console
 */

char *read_line(char *c) {
	int len = strlen(c);
	if (len > 0 && c[len - 1] == '\n')
		c[len - 1] = '\0';
	return c;
}
/**
 * fnCreator function
 *
 *
 *Get commands from console
 */


char *ReadUserInput() {
	char Input[100];
	char *Selc1;
//	printf("[PA1]$ ");
	Selc1 = read_line(fgets(Input, sizeof(Input), stdin));
	return Selc1; //fnSelectCommandAction(Selc1,4556);
}
/**
 * fnCreator function
 *
 *
 *Displays creator command details
 */

void fnCreator() {
	fflush(stdout);
	printf("\nName \t\t\t: Susana Melba D'sa\n");
	printf("UBIT Name \t\t: susaname\n");
	printf("UBIT Email address \t: susaname@buffalo.edu\n");
	printf(
			"I have read and understood the course academic integrity policy located at http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity\n");
	printf("\n[PA1]$");
//	getch();
}
/**
 * fnHelp function
 *
 *
 *Displays help command details
 */

void fnHelp() {
	printf(
			"CREATOR \t\t\t\t: Displays the author of the code and his/her details and declaration\n");
	printf("HELP \t\t\t\t\t: Displays all commands available to user\n");
	printf("MYIP \t\t\t\t\t: Displays IP address for this process \n");
	printf(
			"MYPORT \t\t\t\t\t: Displays port on which this process will listen\n");
	printf(
			"REGISTER <IP> <PORT_NO> \t\t: Registers \n\t<IP> \t\t\t\t: Provide IP of server \n\t<PORT_NO> \t\t\t: Port number of server\n");
	printf(
			"CONNECT <IP> <PORT_NO> \n\t<IP> \t\t\t\t: Provide IP of peer \n\t<PORT_NO> \t\t\t: Port number of peer\n");
	printf(
			"LIST \t\t\t\t\t: Displays a numbered list of all connections this process is part of\n");
	printf(
			"TERMINATE <connection ID> \t\t: Terminates the connection listed under the specified number\n");
	printf(
			"EXIT \t\t\t\t\t: Close all connections and terminate the process\n");
	printf(
			"UPLOAD <conection ID> <file name> \t:Uploads the specified file to peer connected on the connection ID\n");
	printf(
			"DOWNLOAD <connection ID> <filename> \t:Downloads the specified file from peer connected on the connection ID\n\t\t\t\t\t: This command can be used to download upto 3 files from 3 different peers\n");
	printf(
			"STATISTICS \t\t\t\t: Displays upload and download statistics for all clients\n");
	printf("\n[PA1]$ ");
}
/**
 * fnMyIP function
 *
 *
 *Provides my IP
 */

void fnMyIP() {
	//Ref:http://jhshi.me/2013/11/02/how-to-get-hosts-ip-address/
	/* get my hostname */
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) < 0) {
		perror("gethostname");
	}

	// Google's DNS server IP
	char* target_name = "www.google.com";	//"8.8.8.8";
	// DNS port
	char* target_port = "80"; //53

	/* get peer server */
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo* info;
	int ret = 0;
	if ((ret = getaddrinfo(target_name, target_port, &hints, &info)) != 0) {
		printf("[ERROR]: getaddrinfo error: %s\n", gai_strerror(ret));
	}

	if (info->ai_family == AF_INET6) {
		printf("[ERROR]: do not support IPv6 yet.\n");
	}

	/* create socket */
	int sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (sock <= 0) {
		perror("socket");
	}

	/* connect to server */
	if (connect(sock, info->ai_addr, info->ai_addrlen) < 0) {
		perror("connect");
		close(sock);
	}

	/* get local socket info */
	struct sockaddr_in local_addr;
	socklen_t addr_len = sizeof(local_addr);
	if (getsockname(sock, (struct sockaddr*) &local_addr, &addr_len) < 0) {
		perror("getsockname");
		close(sock);
	}

	/* get peer ip addr */
	char myip[INET6_ADDRSTRLEN];
	if (inet_ntop(local_addr.sin_family, &(local_addr.sin_addr), myip,
			sizeof(myip)) == NULL) {
		perror("inet_ntop");
	}

	strcpy(myIP, myip);

}

/**
 * fnMyPort function
 *
 *@param portNumber Process port number
 *
 *Displays process port number
 */
void fnMyPort(char* portNumber) {
	printf("Port Number:  %d\n", atoi(portNumber));
	printf("\n[PA1]$ ");
}

/**
 * ClientDataListener function
 *
 *@param IP Server IP
 *@param portNumber Server port number
 *return 0 on success
 */
int fnRegister(char *IP, char * portNumber) {

	int sockfd, numbytes;
	char buf[1024];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (RegisteredWithServer != 1) {
		if ((rv = getaddrinfo(IP, portNumber, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}

		// loop through all the results and connect to the first we can
		for (p = servinfo; p != NULL; p = p->ai_next) {
			if ((ClientToServerSock = socket(p->ai_family, p->ai_socktype,
					p->ai_protocol)) == -1) {
				perror("client: socket");
				continue;
			}
			if (connect(ClientToServerSock, p->ai_addr, p->ai_addrlen) == -1) {
				close(ClientToServerSock);
				perror("client: connect");
				continue;
			}
			break;
		}
		if (p == NULL) {
			fprintf(stderr, "client: failed to connect\n\n[PA1]$ ");
			return 2;
		}

		inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr), s,
				sizeof s);
		if (sockfd > highsock) { /* keep track of the maximum */
			highsock = sockfd;
		}
	if (strcmp(IP, myIP) != 0) {
		UpdateClientIP(ClientToServerSock, s);
		fnStartClientServer(myPort);
		printf("client data sent\n");
		freeaddrinfo(servinfo); // all done with this structure
	}

	else
		printf("Self connection not allowed\n[PA1]$ ");
	}

	else {
		printf("[PA1]$ Duplicate connections not allowed\n");
	}
	return 0;
}


/**
 * ClientDataListener function
 *
 *@param data Data received by client
 * sends client data to server
 */
int ClientDataListener(char *data) {
	int numbytes;
	//char buf[1024];
	if ((numbytes = recv(ClientToServerSock, data, MAXDATASIZE - 1, 0)) == -1) {
		perror("recv");
		exit(1);
	}
	data[numbytes] = '\0';
	printf("client: received '%s'\n", data);
	return 0;
}


/**
 * ClientDataSender function
 *
 * sends client data to server
 */
void ClientDataSender() {
	char ClientInfoBuffer[MAXDATASIZE];

	/* get my hostname */
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) < 0) {
		perror("gethostname");
	}

	strcpy(ClientInfoBuffer, "1000");
	strcat(ClientInfoBuffer, separator);
	strcat(ClientInfoBuffer, hostname);
	strcat(ClientInfoBuffer, separator);
	strcat(ClientInfoBuffer, myIP);
	strcat(ClientInfoBuffer, separator);
	strcat(ClientInfoBuffer, myPort);
	strcat(ClientInfoBuffer, separator);
	strcat(ClientInfoBuffer, endOfFile);

	int NoOfBytes;
	NoOfBytes = send(ClientToServerSock, ClientInfoBuffer,
			sizeof(ClientInfoBuffer), 0);
}

/**
 * fnConnect function
 *
 * @param  PeerIP IP of peer connection
 * @param  PeerPortNumber port number of peer
 * @return 0 on success
 */
int fnConnect(char *PeerIP, char * PeerPortNumber) {

	int sockfd, numbytes;
	char buf[1024];
	struct addrinfo hints, *servinfo, *p;
	char s[INET6_ADDRSTRLEN];
	int rv;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if(strcmp(PeerIP,myIP)==0)
	{
		printf("Self connection not allowed");
	}
	else
	if(FindPeerInList(PeerIP) == -1)
	{
		printf("Duplicate connection with peer not allowed");
	}
	else
	{
	if ((rv = getaddrinfo(PeerIP, PeerPortNumber, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1) {
			perror("client: socket");
			continue;
		}
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect\n");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr), s,
			sizeof s);
	printf("client: connected to %s\n\n[PA1]$ ", s);
	FD_SET(sockfd, &master);
	if (sockfd > highsock) { /* keep track of the maximum */
		highsock = sockfd;
	}
//	UpdatePeerDetailsTemp(sockfd, s, PeerPortNumber);
	UpdatePeerDetails(sockfd, s);
	++ClientCounter;
	freeaddrinfo(servinfo); // all done with this structure
	}
	return 0;
}

/**
 * fnList function
 *
 * Displays list of all peer connections
 */
void fnList() {
	int j;
	printf("\n");
	printf("%-5s%-35s%-20s%-8s\n", "ID", "HostName", "IP", "Port Number");

	for (j = 0; j < 6; j++) {
		if ((ClientList[j].ID == 1)||(ClientList[j].flag == 1)) {
			printf("%-5d%-35s%-20s%-8d\n", ClientList[j].ID,
					ClientList[j].HostName, ClientList[j].IP,
					atoi(ClientList[j].Port));

		}
	}

	printf("\n[PA1]$ ");

}

/**
 * fnTerminate function
 *
 * @param  id ID associated with the peer
 * @return 0 on success
 */
void fnTerminate(int id) {
	int i = 0;
	int sockId;
	int ClientFoundInList = 0;
	for (i = 0; i < 25; i++) {
		if (ClientList[i].sockfds > 0 && ClientList[i].ID == id) {
			sockId = ClientList[i].sockfds;
			ClientList[i].flag = 0;
			close(ClientList[i].sockfds);
			ClientList[i].sockfds = 0;
			ClientFoundInList++;
			--ClientCounter;
			FD_CLR(sockId, &master);
			break;
		}
	}
	if (ClientFoundInList > 0) {
//		RemoveClientFromList(sockId);
		printf("connection with peer at IP %s was terminated\n \n[PA1]$ ", ClientList[i].IP);
	} else {
		printf("no connection with ID %d\n", id);
	}
}


/**
 * fnExit function
 *
 *Terminates all active connections
 */

void fnExit() {
	printf("ARE YOU SURE YOU WANT TO EXIT FROM THE APPLICATION? ENTER Y/N\n");
	char *input;
	scanf("%c", input);
	if (strcasecmp(input, "Y") == 0) {
		//terminate all active connections
		exit(0);
	}
}

/**
 * fnUpload function
 *
 * @param  receiverSockId Socket ID of receiving end
 * @param filename Filename to be uplaoded
 */
void fnUpload(int receiverID, char *filename) {
	//pass peer ip/hostname and check if that hostname/ip is not of server.
	//http://stackoverflow.com/questions/26000251/large-file-transfer-error-in-socket-in-c
	FILE *fs;
	char *mode = "r";
	char sdbuf[1024];
	time_t readStart, sendEnd;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	long milisec;
	milisec = tv.tv_usec / 1000;
	int receiverSockId;

	//strcpy(filename, "test.txt");

	receiverSockId = RetrieveSocketID(receiverID);

	if(receiverSockId != -1)
	{
	printf("[PA1]$ Sending %s to the Server...\n", filename);
	fs = fopen(filename, "r");
	if(!fs) {
		printf("ERROR: File %s not found.\n", filename);
		exit(1);
	}
	bzero(sdbuf, 1024);
	int fs_block_sz = 0;
	int readbytes;
	readbytes = 0;
	char UploadMsg[1024];
	strcpy(UploadMsg, "3000;");
	strcat(UploadMsg, filename);
	strcat(UploadMsg, ";0;Sending File;---xxx---");
	printf("Sending Upload init msg %s",UploadMsg);
	//UploadMsg = "3000;"+filename+";0;Sending File;---xxx---;";
	//send(receiverSockId, UploadMsg, sizeof(UploadMsg), 0);
	if (send(receiverSockId, UploadMsg, sizeof(UploadMsg), 0) < 0) {
		fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n",
				filename, errno);
	} else {
		time(&readStart);
		printf("Start Time %s\n", ctime(&readStart));
		while ((fs_block_sz = fread(sdbuf, sizeof(char), 1024, fs)) > 0) 		{
			//printf("Reading from file...\n");
			readbytes = readbytes + fs_block_sz;
			if (send(receiverSockId, sdbuf, fs_block_sz, 0) < 0) {
				fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n",
						filename, errno);
				break;
			}
			//printf("Total Sent bits %d\n", readbytes);
			bzero(sdbuf, 1024);
		}
		time(&sendEnd);
		printf("End Time %s\n", ctime(&sendEnd));
		double diff;
		diff = difftime(sendEnd, readStart);
		printf("[PA01] File %s of %d bytes from Client was Sent in %lf !\n",
				filename, readbytes, diff);
		int indexForStruct;
		indexForStruct = 0;
		indexForStruct = FindClientInList(receiverSockId);
		ClientList[indexForStruct].UploadCounter++;
		ClientList[indexForStruct].UploadBytes =
		ClientList[indexForStruct].UploadBytes + readbytes;

	}
	}
	else
		printf("Invalid connection ID\nConnection with ID %d does not exist\n[PA1] $", receiverID);
}

/**
 * fnDownload function
 *
 * @param  sockfd Socket ID for auto download
 *
 */
void fnDownload(int sockfd, char *fr_name) {
	/* Receive File from Server */
	//http://stackoverflow.com/questions/26000251/large-file-transfer-error-in-socket-in-c
	printf("[Client] Receiveing file and saving it...");
	FILE *fr = fopen(fr_name, "a");
	int fr_block_sz;
	char revbuf[512];
	bzero(revbuf, 512);
	fr_block_sz = 0;
	while ((fr_block_sz = recv(sockfd, revbuf, 512, 0)) > 0) {
		int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
		if (write_sz < fr_block_sz) {
			error("File write failed.\n");
		}
		bzero(revbuf, 512);
		if (fr_block_sz == 0 || fr_block_sz != 512) {
			break;
		}
	}
	if (fr_block_sz < 0) {
		if (errno == EAGAIN) {
			printf("recv() timed out.\n");
		} else {
			fprintf(stderr, "recv() failed due to errno = %d\n", errno);
		}
	}
	printf("Ok received from server!\n");
	fclose(fr);
	//}
}
/**
 * fnStatistics function
 *
 * Provides Statistics command output
 */
void fnStatistics() {
	int j;
	printf("%-5s%-20s%-35s%-20s%-35s%-20s\n", "ID", "HostName", "Total Uploads",
			"Average Upload Speed(bps)", "Total Downloads",
			"Average Download Speed(bps)");

	for (j = 0; j < ClientCounter; j++) {
		if (ClientList[j].ID > 0) {
			printf("%-5d%-35s%-5d%-20lu%-5d%-20lu\n", ClientList[j].ID,
					ClientList[j].HostName, ClientList[j].UploadCounter,
					ClientList[j].UploadBytes, ClientList[j].DownloadCounter,
					ClientList[j].DownloadBytes);

		}
	}
}
/**
 * fnErrorMsg function
 *
 *Gives generic error message
 */

void fnErrorMsg() {
	printf(
			"[PA1]$Invalid command or arguments\nTry HELP to get a list of valid commands\n\n[PA1]$");
}

/**
 * fnStartServer function
 *
 * @param  portNumber Process port number
 * @return 0 on success
 */
int fnStartServer(char *PortNumber) {
	//http://www.tenouk.com/Module41.html
	char *PORT;
	PORT = PortNumber;
	char *EnteredInput;
	char sendBuf[MAXDATASIZE];
	/* master file descriptor list */
	//fd_set master;
	//fd_set readmaster;
	/* temp file descriptor list for select() */
	fd_set read_fds; //, input_read_fds

	/* client address */
	struct sockaddr_in clientaddr;
	/* maximum file descriptor number */
	int fdmax;
	/* listening socket descriptor */
	int listener;
	/* inputer reader descriptor */
	int reader;
	/* newly accept()ed socket descriptor */
	int newfd;
	/* buffer for client data */
	char buf[4096];
	int nbytes;
	/* for setsockopt() SO_REUSEADDR, below */
	int yes = 1;
	int addrlen;
	int i, j, k;
	int indexForStruct;
	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 500000;
	///////////////////////////
	struct stat statbuf;
	struct addrinfo hints, *res;
	int sockfd;
	// first, load up address structs with getaddrinfo():
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	// fill in my IP for me
	getaddrinfo(NULL, PORT, &hints, &res);
	/* clear the master and temp sets */
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	//socket(AF_INET, SOCK_STREAM, 0)
	/* get the listener */
	if ((ServerSock = socket(res->ai_family, res->ai_socktype, res->ai_protocol))
			== -1) {
		perror("Server-socket() error lol!");
		/*just exit lol!*/
		exit(1);
	}
	/* bind */
	if (bind(ServerSock, res->ai_addr, res->ai_addrlen) == -1) {
		perror("Server-bind() error lol!");
		exit(1);
	}

	/* listen */
	if (listen(ServerSock, 5) == -1) {
		perror("Server-listen() error lol!");
		exit(1);
	}
	reader = 10;
	/* add the listener to the master set */

	FD_SET(0, &master);
	FD_SET(ServerSock, &master);
	char hostname[128];
	gethostname(hostname, sizeof(hostname));

	ClientCounter = 0;

	ClientList[ClientCounter].ID = ClientCounter + 1;
	strcpy(ClientList[ClientCounter].HostName, hostname);
	strcpy(ClientList[ClientCounter].IP, myIP);
	strcpy(ClientList[ClientCounter].Port, InstancePort);
	ClientList[ClientCounter].sockfds = 0;

	++ClientCounter;

	printf("[PA1]$ ");
	fflush(stdout);
	/* keep track of the biggest file descriptor */
	highsock = ServerSock; /* so far, it's this one*/
	/* loop */
	while (1) {
		/* copy it */
		read_fds = master;
		if (select(highsock + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("[PA1]$ Server-select() error!");
			exit(1);
		}

		if (FD_ISSET(0, &read_fds)) {
			char* userSelection = ReadUserInput(); //read_line(fgets(GetUserInput,sizeof(GetUserInput),stdin));
			userSelection = fnSelectCommandAction(userSelection, PortNumber);
		}

		else {

			for (i = 0; i <= highsock; i++) {
				if (FD_ISSET(i, &read_fds)) {
					if ((i == ServerSock)) {
						/* handle new connections */
						addrlen = sizeof(clientaddr);
						if ((newfd = accept(ServerSock,
								(struct sockaddr *) &clientaddr, &addrlen))
								== -1) {
							perror("[PA1]$ Server-accept() error lol!");
						} else {
							FD_SET(newfd, &master); /* add to master set */
							if (newfd > highsock) { /* keep track of the maximum */
								highsock = newfd;
							}
							printf(
									"[PA1]$ New client connection from %s on socket %d\n",
									inet_ntoa(clientaddr.sin_addr), newfd);
							UpdateClientIP(newfd,
									inet_ntoa(clientaddr.sin_addr));
							++ClientCounter;
						}
					} else {
						/* handle data from a client */
						if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
							/* got error or connection closed by client */
							if (nbytes == 0)
								/* connection closed */
								printf(
										"[PA1]$ client with socket ID %d hung up\n",
										i); //remove from global list.
							else {

								perror("recv() error lol!\n");

							}
							/* close it... */
							close(i);
							/* remove from master set */
							FD_CLR(i, &master);
						} else {
							/* we got some data from a client*/
							//read client data and add it to global client data list.
							UpdateClientData(i, buf);
							char tempToHoldIntToCharValue[100];
							strcpy(sendBuf, ReceiveFromServerUpdatedList);
							strcat(sendBuf, separator);

							for (k = 0; k < ClientCounter; k++) {
								sprintf(tempToHoldIntToCharValue, "%d",
										ClientList[k].ID);
								strcat(sendBuf, tempToHoldIntToCharValue);

								strcat(sendBuf, separator);

								strcat(sendBuf, ClientList[k].HostName);

								strcat(sendBuf, separator);

								strcat(sendBuf, ClientList[k].IP);

								strcat(sendBuf, separator);

								strcat(sendBuf, ClientList[k].Port);

								strcat(sendBuf, separator);
							}
							strcat(sendBuf, endOfFile);

							int bytes;

							for (j = 1; j <= ClientCounter; j++) {
								/* send to everyone! */
								bytes = send(ClientList[j].sockfds, sendBuf,
										sizeof(sendBuf), 0);
							}
						}
					}
				}
			}

			fflush(stdout);

		}

//		printf("[PA1]$\n");
	}
//	printf("[PA1]$ came out of while loop, will return 0 and close.\n");
	return 0;
}
/**
 * fnStartClientServer function
 *
 * @param  portNumber Process port number
 * @return 0 on success

 */

int fnStartClientServer(char *PortNumber) {
	//http://www.tenouk.com/Module41.html
	char *PORT;
	PORT = PortNumber;
	char *EnteredInput;
	/* master file descriptor list */
	/* temp file descriptor list for select() */
	fd_set read_fds; //, input_read_fds

	/* client address */
	struct sockaddr_in clientaddr;
	/* maximum file descriptor number */
	int fdmax;
	/* listening socket descriptor */
	int listener;
	/* inputer reader descriptor */
	int reader;
	/* newly accept()ed socket descriptor */
	int newfd;
	/* buffer for client data */
	char buf[4096];
	int nbytes;
	/* for setsockopt() SO_REUSEADDR, below */
	int yes = 1;
	int addrlen;
	int i, j;
	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 500000;
	///////////////////////////
	struct stat statbuf;
	struct addrinfo hints, *res;
	int sockfd;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, PORT, &hints, &res);
	/* clear the master and temp sets */
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	//socket(AF_INET, SOCK_STREAM, 0)
	/* get the listener */
	if ((ServerSock = socket(res->ai_family, res->ai_socktype, res->ai_protocol))
			== -1) {
		perror("Client-socket() error!");
		/*just exit lol!*/
		exit(1);
	}

	if (bind(ServerSock, res->ai_addr, res->ai_addrlen) == -1) {
		perror("Client-Listner-bind() error lol!");
		exit(1);
	}
	/* listen */

	if (listen(ServerSock, 5) == -1) {
		perror("Client-Listner-listen() error lol!");
		exit(1);
	}
	ClientServerStarted = 1;
	reader = 10;
	/* add the listener to the master set */
	//FD_SET(STDIN, &master);
	FD_SET(0, &master);
	FD_SET(ServerSock, &master);
	FD_SET(ClientToServerSock, &master);
	ClientDataSender();
	ClientCounter = 1;
	RegisteredWithServer = 1;
	/* keep track of the biggest file descriptor */
	highsock = ServerSock; /* so far, it's this one*/
	HighestSock = highsock;
	/* loop */
	while (1) {
		/* copy it */
		highsock= HighestSock;
		read_fds = master;
		if (select(highsock + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("[PA1]$ Client-Listner-select() error!");
			exit(1);
		}

		fflush(stdout);

		if (FD_ISSET(0, &read_fds)) {
			char* userSelection = ReadUserInput(); //read_line(fgets(GetUserInput,sizeof(GetUserInput),stdin));

			userSelection = fnSelectCommandAction(userSelection, PortNumber);
		}

		else {
			for (i = 0; i <= highsock; i++) {
				if (FD_ISSET(i, &read_fds)) {
					if ((i == ServerSock)) {
						/* handle new connections */
						addrlen = sizeof(clientaddr);
						if ((newfd = accept(ServerSock,
								(struct sockaddr *) &clientaddr, &addrlen))
								== -1) {
							perror("[PA1]$ Client-Listner-accept() error lol!");
						} else {

							FD_SET(newfd, &master); /* add to master set */
							if (newfd > highsock) { /* keep track of the maximum */
								highsock = newfd;
								HighestSock = highsock;
							}
							UpdatePeerDetails(newfd,inet_ntoa(clientaddr.sin_addr));
							printf(
									"Peer connection with host at IP %s established\n\n[PA1]$ ",
									inet_ntoa(clientaddr.sin_addr));
//							++ClientCounter;
							fflush(stdout);
						}
					} else //if(S_ISSOCK(statbuf.st_mode))
					{

						/* handle data from a client */
						if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
							/* got error or connection closed by client */
							if (nbytes == 0)
							{
								/* connection closed */
								printf("[PA1]$ Peer initiated connection terminate\n"); //remove from global list.
								int index = FindClientInList(i);
								RemoveClientFromList(i);
								printf("Connection with peer at IP %s has been terminated\n\n[PA1]$ ", ClientList[index].IP);
								fflush(stdout);
							}
							else {

								perror("recv() error lol!\n");

							}
							/* close it... */
							close(i);
							/* remove from master set */
							FD_CLR(i, &master);
						} else {
							/* we got some data from a client*/
							//read client data and add it to global client data list.
							//printf("Received msg %s",buf);
							char *token;
							int count = -1;
							int countForTerms = 0;
							int flag = 1;
							token = strtok(buf, ";");
							printf("token %s\n",token);
							/* walk through other tokens */						if (strcmp(token, "3000") == 0) {AcceptUploadFile(i, buf);}
							while (token != NULL) {

							if (count == -1) {

							if (strcmp(token,
											ReceiveFromServerUpdatedList)
											== 0) {
										char* temp;
										strcpy(temp, token);
										flag = 0;
									} else if (strcmp(token, "3000") == 0) {
										AcceptUploadFile(i, buf);
									}
								}
								if ((flag == 0) && (count > -1)) {
									if (strcmp(token, endOfFile) != 0) {

										if (countForTerms == 1) {
											ClientList[count].ID = atoi(token);
											count--;
										}

										if (countForTerms == 2) {
											strcpy(ClientList[count].HostName,
													token);
											count--;
										}

										if (countForTerms == 3) {
											strcpy(ClientList[count].IP, token);
											count--;
										}

										if (countForTerms == 4) {
											strcpy(ClientList[count].Port,
													token);
											countForTerms = 0;
											ClientList[count].flag = 0;
										}
									}
								}
								token = strtok(NULL, ";");
								count++;
								countForTerms++;
							}

							fflush(stdout);

							printf("\n");
							printf("%-5s%-35s%-20s%-8s\n", "ID", "HostName",
									"IP", "Port Number");

							for (j = 0; j < count; j++) {
								if (ClientList[j].ID > 0) {
									printf("%-5d%-35s%-20s%-8d\n",
											ClientList[j].ID,
											ClientList[j].HostName,
											ClientList[j].IP,
											atoi(ClientList[j].Port));

								}
							}

							printf("\n[PA1]$ ");
							fflush(stdout);
						}
					}
				}
			}
		}
	}
	return 0;
}

/**
 * fnSelectCommandAction function
 *
 * @param  inputCmd Command line input
 * @param  portNumber Process port number
 * @return char* Input Typed by User

 */


char *fnSelectCommandAction(char* inputCmd, char* portNumber) {
	int count = 0;
	char *token;
	char outputCmd1[50];
	char outputCmd2[50];
	char outputCmd3[50];
	char outputCmd4[50];
	char outputCmd5[50];
	char outputCmd6[50];
	char outputCmd7[50];

	token = strtok(inputCmd, " ");

	while (token != NULL) {

		if (count < 7) {

			if (count == 0) {
				strcpy(outputCmd1, token);
			}

			else if (count == 1) {
				strcpy(outputCmd2, token);

			}

			else if (count == 2) {
				strcpy(outputCmd3, token);

			}

			else if (count == 3) {
				strcpy(outputCmd4, token);

			}

			else if (count == 4) {
				strcpy(outputCmd5, token);

			}

			else if (count == 5) {
				strcpy(outputCmd6, token);

			}

			else if (count == 6) {
				strcpy(outputCmd7, token);

			}

		} else
			printf("Invalid input");

		token = strtok(NULL, " ");

		count += 1;
	}

	if (count == 1) {
		if (strcasecmp(outputCmd1, "CREATOR") == 0) {
			fnCreator();
		} else if (strcasecmp(outputCmd1, "HELP") == 0) {
			fnHelp();
		} else if (strcasecmp(outputCmd1, "MYIP") == 0) {
			char myip[20];
			fnMyIP();

			strcpy(myip, myIP);
			strcat(myip, "\0");
			printf("IP address: %s\n", myip);
			printf("\n[PA1]$ ");
		} else if (strcasecmp(outputCmd1, "MYPORT") == 0) {
			fnMyPort(portNumber);
		} else if (strcasecmp(outputCmd1, "LIST") == 0) {
			fnList();
		} else if (strcasecmp(outputCmd1, "EXIT") == 0) {
			fnExit();
		} else if (strcasecmp(outputCmd1, "STATISTICS") == 0) {
			fnStatistics();
		} else {
			fnErrorMsg();
		}
	}

	else if (count == 2) {
		if (strcasecmp(outputCmd1, "TERMINATE") == 0) {
			fnTerminate(atoi(outputCmd2));
		} else {
			fnErrorMsg();
		}
	}

	else if (count == 3) {
		if (strcasecmp(outputCmd1, "REGISTER") == 0) {
			if(strcmp(InstanceType,"s")==0)
			{
				printf("Server cannot register with itself or with any other client\n\n[PA1]$");
			}
			else
			{
			int ClientSocketDescptr;
			ClientSocketDescptr = fnRegister(outputCmd2, outputCmd3);
			}
		} else if (strcasecmp(outputCmd1, "CONNECT") == 0) {
			int peerConnectionSocketDescptr;
			peerConnectionSocketDescptr = fnConnect(outputCmd2, outputCmd3);
		} else if (strcasecmp(outputCmd1, "UPLOAD") == 0) {
			if (InstanceType == "c") {
				char *filename;
				fnUpload(atoi(outputCmd2), outputCmd3);
			} else {
				printf("[PA1]$ Server can't upload any files to clients.\n");
			}
		} else if (strcasecmp(outputCmd1, "DOWNLOAD") == 0) {
			fnDownload(atoi(outputCmd2), outputCmd3);
		} else {
			fnErrorMsg();
		}
	} else if (count == 5) {
		if (strcasecmp(outputCmd1, "DOWNLOAD") == 0) {
			fnDownload(atoi(outputCmd2), outputCmd3);
		} else {
			fnErrorMsg();
		}
	}

	else if (count == 7) {
		if (strcasecmp(outputCmd1, "DOWNLOAD") == 0) {
			fnDownload(atoi(outputCmd2), outputCmd3);
		} else {
			fnErrorMsg();
		}
	} else {
		fnErrorMsg();
	}

	fflush(stdout);
	char NextInput[100];
	if (strcasecmp(InstanceType, "c") == 0)
	{
		if(RegisteredWithServer == 0)
		{
		inputCmd = read_line(fgets(NextInput, sizeof(NextInput), stdin));
		}
	}
	return inputCmd;
}

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS

 */

int main(int argc, char **argv) {
	/*Start Here*/
	char inst[inst_len], param[param_len];
	int strlength;
	int portNumber;
	char *server = "s";
	char *client = "c";
	char GetUserInput[100];
	ClientCounter = 0;
	RegisteredWithServer = 0;
	char *userSelection;
	int i;
	int flag = 0;
	ClientServerStarted = 0;
	printf("\n");
	if (argc == 3) {

		if (strcasecmp(argv[1], server) == 0) {
			InstanceType = "s";

			for (i = 0; i < strlen(argv[2]); i++) {
				if (isdigit(argv[2][i])) {
				}

				else {
					flag = 1;
				}
			}
			if (flag == 0) {
				InstancePort = argv[2];
				strcpy(myPort, argv[2]);

				fnMyIP();
				fnStartServer(myPort);
			} else
				printf(
						"[PA1]$ Invalid arguments\nArgument 2 can only be numeric\n");
		}
//end of if (strcmp(argv[1], server) == 0)
		else if (strcasecmp(argv[1], client) == 0) {
			InstanceType = "c";

			for (i = 0; i < strlen(argv[2]); i++) {
				if (isdigit(argv[2][i])) {
				}

				else {
					flag = 1;
				}
			}
			if (flag == 0) {
				InstancePort = argv[2];
				strcpy(myPort, argv[2]);
				fnMyIP();

				printf("[PA1]$ ");

				userSelection = ReadUserInput(); //read_line(fgets(GetUserInput,sizeof(GetUserInput),stdin));
				while (strcasecmp(userSelection, "EXIT") != 0) {
					if (ClientServerStarted == 0)
						userSelection = fnSelectCommandAction(userSelection,
								argv[2]);
				}

			} else
				printf(
						"[PA1]$ Invalid arguments\nArgument 2 can only be numeric\n");
		} //end of (strcmp(argv[1], client)==0)
		else {
			printf(
					"[PA1]$ Invalid arguments\n Argument 1 can only be S/s for server or C/c for client\n");
		}
	} else {
		printf(
				"[PA1]$ Invalid Arguments\nInput takes 2 arguments: <Project or Object name> <s or c> <port number>\n");
	}
	return 0;
}
