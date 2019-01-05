/*
* FILENAME: otp_enc.c
*	DESCRIPTION: otp_enc serves as a client to send messages and recieve encrpyted
* one
* Jared Tence
*/
#include "otp_tools.h"

// Error function used for reporting issues 21662
void formatSend(char *, char *, char *);

//sends message and receives encrpyted one
int main(int argc, char *argv[])
{
	//setups of variables
	int socketFD, portNumber, charsWritten, charsRead;
	char * port = argv[3];
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	//checks for a lack of arguments
	if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// Get input message from user
	char buffer[MAX_BUFFER];
	memset(buffer, '\0', MAX_BUFFER);
	formatSend(argv[1], argv[2], buffer);
	// Send message to server
	sendMessage(socketFD, buffer);
	// Get return encrpyted message from server
	memset(buffer, '\0', MAX_BUFFER);
	getInput(socketFD, buffer, MAX_BUFFER);
	//if return message is a empty message with a % terminator
	if(buffer[0] == '%'){
		fprintf(stderr, "Error: could not contact otp_enc_d on port %s\n", port);
		exit(2);
	}
	//removes message terminator
	buffer[strlen(buffer) - 1] = '\0';
	fprintf(stdout, "%s\n", buffer);

	close(socketFD); // Close the socket
	return 0;
}

//formatsSend takes a message file and a key file, checks for errors, and combines
//them into a single string and stores it in char * output
void formatSend(char * messageFile, char * keyFile, char * output){
	//sets up variables
	int mesSize, keySize;

	//reads from file one
	readFile(messageFile, output, MAX_BUFFER - 1);
	//gets the length of the message
	mesSize = strlen(output);
	//adds our character divider
	output[strlen(output)] = '&';
	//safegaurd to prevent character overflow
	if(mesSize + 1 > MAX_BUFFER/2){
		readFile(keyFile, &(output[strlen(output)]), MAX_BUFFER - 1 - mesSize);
	}else{
		readFile(keyFile, &(output[strlen(output)]), mesSize);
	}
	keySize = strlen(output) - mesSize - 1;
	//adds terminating chracter
	output[strlen(output)] = '@';
	output[strlen(output)] = '\0';
	//checks if the key doesn't have enough character
	if(mesSize > keySize){
		fprintf(stderr, "Error: key ‘%s’ is too short\n", keyFile);
		exit(1);
	}
}
