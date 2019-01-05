#include "otp_tools.h"

// Error function used for reporting issues
void formatSend(char *, char *, char *);

//exactlly like otp_enc but sends message to be decrypted instead of encrpyted
int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	char * port = argv[3];
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
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

	//printf("sending message: %s\n", buffer);
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) fprintf(stderr, "CLIENT: WARNING: Not all data written to socket!\n");
	// Get return message from server
	memset(buffer, '\0', MAX_BUFFER);
	getInput(socketFD, buffer, MAX_BUFFER);
	if(buffer[0] == '@'){
		fprintf(stderr, "Error: could not contact otp_dec_d on port %s\n", port);
		exit(2);
	}

	buffer[strlen(buffer) - 1] = '\0';
	fprintf(stdout, "%s\n", buffer);

	close(socketFD); // Close the socket
	return 0;
}

//exactlly like opt_enc formatSend instead of encrpyting it decrypts
void formatSend(char * messageFile, char * keyFile, char * output){
	int mesSize, keySize;

	readFile(messageFile, output, MAX_BUFFER - 1);
	mesSize = strlen(output);
	output[strlen(output)] = '&';
	if(mesSize + 1 > MAX_BUFFER/2){
		readFile(keyFile, &(output[strlen(output)]), MAX_BUFFER - 1 - mesSize);
	}else{
		readFile(keyFile, &(output[strlen(output)]), mesSize);
	}
	keySize = strlen(output) - mesSize - 1;
	output[strlen(output)] = '%';
	output[strlen(output)] = '\0';

	if(mesSize > keySize){
		fprintf(stderr, "Error: key ‘%s’ is too short\n", keyFile);
		exit(1);
	}
}
