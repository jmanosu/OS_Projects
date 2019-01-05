/*
* FILENAME: otp_dec_d.c
*	DESCRIPTION: exactlly like otp_enc_d execpt it decrpts instead of encrpyts
* Jared Tence
*/
#include "otp_tools.h"
// Error function used for reporting issues
void parseInput(char *, char *);

//exactlly like otp_enc_d main execpt decrpts instead of encrpyts and sends a
//terminating character '%'
int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr, "USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
	int con = 1;
	while(con){
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		int spawnPid = fork();
		switch (spawnPid) {
			case -1:
				fprintf(stderr, "ERROR creating fork\n");
			case 0:
				if (establishedConnectionFD < 0) error("ERROR on accept");

				// Get the message from the client
				char rawInput[MAX_BUFFER], decryptString[MAX_BUFFER];
				memset(rawInput, '\0', MAX_BUFFER);
				memset(decryptString, '\0', MAX_BUFFER);
				getInput(establishedConnectionFD, rawInput, MAX_BUFFER - 1);
				//printf("rawInput: %s\n", rawInput);
				if(rawInput[strlen(rawInput) - 1] == '@'){
					decryptString[0] = '%';
					decryptString[1] = '\0';
				}else{
					parseInput(rawInput, decryptString);
					decryptString[strlen(decryptString)] = '%';
					decryptString[strlen(decryptString)] = '\0';
				}
				//sends decrypted message
				sendMessage(establishedConnectionFD, decryptString);
				close(establishedConnectionFD); // Close the existing socket which is connected to the client
		}
		waitpid((pid_t)(-1), 0, WNOHANG);
	}
	close(listenSocketFD); // Close the listening socket
	return 0;
}

void parseInput(char * buffer, char * decryptString){
	strcpy(decryptString, strtok(buffer,"&"));
	char * key = strtok(NULL,"&");
	decrypt(decryptString, key);
}
