//File: buildrooms.c
//By: Jared Tence
//Date: 10/25/2016

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

//basic function declarations
void setupRooms();
void addConnection(char *, char *, int);
void addConnections();
void addRoomTypes();
int roomContains(char *, char *);
int validRooms();
void addTypes();

//room struct that will hold all created rooms and number of connections
struct room{
	char name[9];
	int connections;
};

//global vars that hold all rooms created
struct room rooms[7];
//directory of were we are storing room files
char directory[30];

//main function
void main(){
	//sets directory var to null pointers
	memset(directory, '\0', 30);
	//sets directory string to tencej.rooms.process ID/
	sprintf(directory, "tencej.rooms.%i/", getpid());
	//initalizes srand so it produces random numbers
	srand(time(NULL));
	//creates room files and adds name line
	setupRooms();
	//adds all connections to rooms
	addConnections();
	//adds room types
	addTypes();
}

//creates new directory and adds room files to ti
void setupRooms(){

	//number of new room files
	int new_file;

	//checks to see if directory already exists
	struct stat st = {0};
	if (stat(directory, &st) == -1) {
		//makes new directory
		int i = mkdir(directory, 0777);
	}else{
		printf("error directory already exists");
		exit(1);
	}

	//hard coded names
	char * names[10] = {"Bed","Study","Ball",
			"Bath","Kitchen","Basement",
			"Lounge","Attic","Dining","library"};
	//for loops until 7 new room files are created
	int i;
	for(i = 0; i < 7; i++){
		//picks a random name to create a new room file from
		int r = rand()%10;
		char * name = names[r];

		//creates new string that is the path to new file
		char path[50];
		memset(path, '\0', 50);
		sprintf(path,"%s%s_room",directory,name);

		//checks if room file already exists
		if(stat(path, &st) == -1){
			//opens new file to write to
			new_file = open(path, O_WRONLY | O_CREAT, 0600);
			//checks if there was an error in file creation
			if (new_file < 0){
      	fprintf(stderr, "Could not open %s\n", path);
      	perror("Error in setupRooms()");
      	exit(1);
		  }

			//creates buffer string
			char buffer[20];
			memset(buffer, '\0', 20);
			//sets buffer string to ROOM NAME: line
			sprintf(buffer, "ROOM NAME: %s\n",name);
			//writes buffer to file
			write(new_file, &buffer, strlen(buffer));
			//closes file
			close(new_file);
			//adds new room to rooms struct
			memset(rooms[i].name, '\0', 9);
			strcpy(rooms[i].name, name);
			rooms[i].connections = 0;
		}else{
			//decrements i because no room was created
			i--;
		}
	}
}

//adds connections to a single room give a room and number of # connection it is
void addConnection(char roomA[], char roomB[], int numC){
	//creates path to room
	char path[30];
	memset(path, '\0', 30);
	sprintf(path, "%s%s%s", directory, roomA, "_room");
	//opens file to write to
	int file_descriptor = open(path, O_WRONLY | O_APPEND);

	//creates buffer
	char buffer[30];
	memset(buffer, '\0', 30);

	//erros if file wasn't opened
	if (file_descriptor < 0){
		fprintf(stderr, "Could not open %s\n", path);
		perror("Error in addConnecion()");
		exit(1);
	}

	//adds string to buffer
	sprintf(buffer, "CONNECTION %i: %s\n", numC, roomB);

	//writes buffer to file
	write(file_descriptor, &buffer, strlen(buffer));

	//closes file
	close(file_descriptor);
}

//adds connections to all files
void addConnections(){
	//runs until all rooms are valid
	while(!validRooms()){
		//generates to random rooms to add a connection between the two
		int r1 = rand()%7;
		int r2 = rand()%7;
		//aslong as the connection doesn't exist and both rooms connections don't
		//exceed 6 then it adds the connection
		if(r1 != r2 &&
			rooms[r1].connections < 6 &&
			rooms[r2].connections < 6 &&
			roomContains(rooms[r1].name,rooms[r2].name) != 1){
			//increase the number of connections each room has
			rooms[r1].connections += 1;
			rooms[r2].connections += 1;
			//adds a connection between the two
			addConnection(rooms[r1].name, rooms[r2].name, rooms[r1].connections);
			addConnection(rooms[r2].name, rooms[r1].name, rooms[r2].connections);
		}
	}
}

//checks if a room contains a string
int roomContains(char * roomA, char * roomB){

	//creates path to room
	char path[30];
	memset(path, '\0', 30);
	sprintf(path, "%s%s%s", directory, roomA, "_room");
	//opens file to check if it containts a string
  int file_descriptor = open(path, O_RDONLY);
	//errors if file wasn't opened
  if (file_descriptor < 0){
      fprintf(stderr, "Could not open %s\n", path);
      perror("Error in roomContains()");
      exit(1);
  }

	//creates buffer
	char buffer = 'i';
	char word[25];
  memset(word, '\0', 25);

	//reads each word from file
	while(read(file_descriptor, &buffer, 1)){
		//if current character isn't a space it adds it to the buffer
		if(buffer != ' ' &&  buffer != '\n'){
			word[strlen(word)] = buffer;
		}else{
			//compares to see if string is the string we are looking fore
			if(strcmp(word, roomB) == 0){
				//returns 1 indicating string exists in file
				close(file_descriptor);
				return 1;
			}else{
				//resets buffer
				memset(word, '\0', 25);
			}
		}
	}
	close(file_descriptor);
	//if doesn't exist returns 0
	return 0;
}

//checks to see if rooms are valid
int validRooms(){
	int i;
	for(i = 0; i < 7; i++){
		//checks to see if each room has more than three connections
		if(rooms[i].connections < 3){
			return 0;
		}
	}
	return 1;
}

//adds types to all rooms
//I did encouter an weird bug that I haven't see again were it added all types
//except the Start room which was left blank
void addTypes(){
	int i;
	char * roomTypes[3] = {"START_ROOM","END_ROOM","MID_ROOM"};
	//loops through all rooms
	for(i = 0; i < 7; i++){
		//creates path to room
		char path[30];
		memset(path, '\0', 30);
		sprintf(path, "%s%s%s", directory, rooms[i].name, "_room");

		//opens file
  	int file_descriptor = open(path, O_WRONLY | O_APPEND);
		//erros if cannot open it
  	if (file_descriptor < 0){
    	fprintf(stderr, "Could not open %s\n", path);
    	perror("Error in addTypes()");
    	exit(1);
 		}
		//if index exceeds 2 then sets it back to 2 so it doesn't core dump
		int index = i;
		if(index > 2){
			index = 2;
		}

		//creates buffer
		char buffer[40];
		memset(buffer, '\0', 40);
		sprintf(buffer, "ROOM TYPE: %s\n", roomTypes[index]);
		//writes buffer
		write(file_descriptor, &buffer, strlen(buffer));
		//closes file
		close(file_descriptor);
	}
}
