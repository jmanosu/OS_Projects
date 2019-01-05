/*
* FILENAME: otp_enc_d.c
*	DESCRIPTION: otp_enc_d serves as a sever to encrpyt messages
* Jared Tence
*/
#include "otp_tools.h"
// Error function used for reporting issues
void parseInput(char *, char *);

//runs the server
int main(int argc, char *argv[])
{
	//initalizes values later to be used
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;

	//checks if there are not enough arguments
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
	//runs forever
	int con = 1;
	while(con){
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		//when an established connection is made it forks the process to deal with it
		int spawnPid = fork();
		switch (spawnPid) {
			//breaks if fork errors
			case -1:
				fprintf(stderr, "ERROR creating fork\n");
				exit(1);
			case 0:
				if (establishedConnectionFD < 0) error("ERROR on accept");

				// Get the message from the client
				char rawInput[MAX_BUFFER], encryptString[MAX_BUFFER];
				memset(rawInput, '\0', MAX_BUFFER);
				memset(encryptString, '\0', MAX_BUFFER);
				getInput(establishedConnectionFD, rawInput, MAX_BUFFER - 1);
				//set values so it has correct terminates and if  it recieves a vlue
				//from otp_dec then it sends back a blank message
				if(rawInput[strlen(rawInput) - 1] == '%'){
					encryptString[0] = '@';
					encryptString[1] = '\0';
				}else{
					parseInput(rawInput, encryptString);
					encryptString[strlen(encryptString)] = '@';
					encryptString[strlen(encryptString)] = '\0';
				}

				// Send a encrpyted message back to the client
				sendMessage(establishedConnectionFD, encryptString);
				close(establishedConnectionFD); // Close the existing socket which is connected to the client
		}
		//KILL THOSE ZOMBIE CHILDREN
		waitpid((pid_t)(-1), 0, WNOHANG);
	}
	close(listenSocketFD); // Close the listening socket
	return 0;
}

//parses raw input and encrpyts to be sent out
void parseInput(char * rawInput, char * encryptString){
	strcpy(encryptString, strtok(rawInput,"&"));
	char * key = strtok(NULL,"&");
	encrypt(encryptString, key);
}
