/*
* FILENAME: otp_tools.h
*	DESCRIPTION: basic tools used by all otp
* Jared Tence
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

//some constents regarding buffer sizes and maximum byte send size
#define MAX_BUFFER 190000
#define MAX_SEND_SIZE 1000

//function definitions
void readFile(char *, char *, int);
void decrypt(char *, char *);
void encrypt(char *, char *);
void getInput(int, char *, int);

//error function
void error(const char *msg) { perror(msg); exit(1); }

//reads first line from file "file" and outputs it into a char array "output"
//max size prevents character overflow
void readFile(char * file, char * output, int max){
	//opens file for reading
	int fileDesc = open(file, O_RDONLY), numRead = 0;
	char buffer;
	//checks for bad open
	if(fileDesc < 0){
		fprintf(stderr, "ERROR cannot open message file\n" , 30);
		exit(1);
	}
	//reads through first line character by character
	while(read(fileDesc, &buffer, 1) > 0){
		//check if buffer is a new line or not between "A-B" or a ' '
		if(buffer == '\n'){
			break;
		}else if(buffer < 65 && buffer != 32 || buffer > 90){
			fprintf(stderr, "otp_enc error: %s contains bad characters\n", file, 30);
			exit(1);
		}
		//increases our number of characters read variable and adds character to output
    numRead++;
		output[strlen(output)] = buffer;
		//if characters red exceeds buffer max then we break
    if(max > 0 && numRead >= max){
      break;
    }
	}
	//closes file descriptor
  close(fileDesc);
}

//getInput reads from our input string provided by the socket
void getInput(int establishedConnectionFD, char * output, int max){
	//temporary buffer and # of chars read
  char buffer[1001];
  int charsRead;
	//reads character from socket until it reaches terminating character or error is detected
  while(!(strlen(output) > 0 && (output[strlen(output) - 1] == '@' || output[strlen(output) - 1] == '%'))){
    memset(buffer, '\0', 1001);
    charsRead = recv(establishedConnectionFD, buffer, 1000, 0); // Read the client's message from the socket
		//checks if it failed to read from socket
		if (charsRead < 0) error("ERROR reading from socket");
		//checks if file character read is exceding buffer
    if(strlen(output) + strlen(buffer) > max){
      break;
    }
    strcat(output, buffer);
  }
}

//sendMessage sends a message to the socket so that it can be transmitted
void sendMessage(int establishedConnectionFD, char * outGoing){
	//currentChar, charsRead, snedSize
	int currentChar = 0, charsRead = 0, sendSize = 0;
	//sends characters until all characters have been sent
	while(currentChar < strlen(outGoing) + 1){
		//if the characters left exceed the max send size then it only sends the max
		//send size
		if(strlen(outGoing) + 1 - currentChar > MAX_SEND_SIZE){
			sendSize = MAX_SEND_SIZE;
		}else{
			sendSize = strlen(outGoing) + 1 - currentChar;
		}
		//sends characters
		charsRead = send(establishedConnectionFD, &outGoing[currentChar], sendSize, 0); // Send success back
		//checks for sending errors
		if (charsRead < 0) error("ERROR writing to socket");
		//increases the current character send count
		currentChar += charsRead;
	}
}

//decrpts message using key
void decrypt(char * string, char * key){
	int i;
	//loops through "string" and converts character by character
	for(i = 0; i < strlen(string); i++){
		//converts string and key to values between 0 and 27
    char tempS = string[i] - 64;
    char tempK = key[i] - 64;
		//because ' ' is not inbetween "A-B" it needs to convert it
		if(tempS == -32){
			tempS = 27;
		}
		if(tempK == -32){
			tempK = 27;
		}
		//adds key character to string
    tempS += tempK;
    if(tempS > 27){
      tempS -= 27;
    }
		//if result is 27 we convert need to set string[i] to ' '
    if(tempS == 27){
      string[i] = ' ';
    }else{
      string[i] = tempS + 64;
    }
	}
}

//encrypt does what decrypt does but subtracts values instead of adds them
void encrypt(char * string, char * key){
	int i;
	for(i = 0; i < strlen(string); i++){
    char tempS = string[i] - 64;
    char tempK = key[i] - 64;
		if(tempS == -32){
			tempS = 27;
		}
		if(tempK == -32){
			tempK = 27;
		}
    tempS -= tempK;
    if(tempS < 1){
      tempS += 27;
    }
    if(tempS == 27){
      string[i] = ' ';
    }else{
      string[i] = tempS + 64;
    }
	}
}
