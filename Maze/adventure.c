//File: adventure.c
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
#include <dirent.h>
#include <pthread.h>
#include <time.h>

//function initalizers
void loadRoom(char *);
void getNextWord(int, char *);
void initalizeRoom();
void displayRoom();
int getInput(char *);
void getFileDir();
void setupStart();
void* runGame();
void* calcTime();

//struct room contains all information of the room file
struct room{
  char name[9];
  char connections[6][9];
  int start, end, numCon;
};

//directory of the where the room files are located
char directory[30];

//file path for the current time file. it's a global var so that it bricks
//threads if the program wants to read and write at the same time.
char timePath[] = "currentTime.txt";

//current room the player is in
struct room currentRoom;

//thread for running the game component of the program
pthread_t gameThread;
//thread for running the caculation of the current time
pthread_t timeThread;
//counts the number of steps a person has taken
int counter;
//mutex lock
pthread_mutex_t lock;

void main(){
  //initalizes all threads and locks
  pthread_mutex_init(&lock, NULL);
  pthread_create(&gameThread, NULL, &runGame, NULL);
  pthread_create(&timeThread, NULL, &calcTime, NULL);
  pthread_join(gameThread, NULL);
  pthread_mutex_destroy(&lock);
}

//caculates the current time
void* calcTime(){
  //looks to avoid another process from opening the same file
  pthread_mutex_lock(&lock);
  //opens the time file to write to and clears it due to trunc
  int file_descriptor = open(timePath, O_WRONLY | O_TRUNC);
  //creates a temp that we will write to the file
  char temp[100];
  //gets the current time
  time_t t = time(0);
  //creates a tm struct to store the current time
  struct tm *tmp ;
  tmp = localtime( &t );
  //formates the tm struct tmp to be written to the file
  strftime(temp, 100, "%l:%M%P, %A, %B %d, %G", tmp);
  //writes the current time to the time file
  write(file_descriptor, temp, strlen(temp));
  //closes the time file
  close(file_descriptor);
  //unlocks mutex so otherthreads can used shared varables
  pthread_mutex_unlock(&lock);
}

//runs the adventure game
void* runGame(){

  //initalizes the time file and closes it
  close(open(timePath, O_WRONLY | O_CREAT, 0600));

  //sets the directory path to null terminators
  memset(directory, '\0', 30);
  //gets the directory were the room files are located at
  getFileDir();

  //creates a string to hold the path the player traveled through to get to the
  //end of the maze
  int currentPathSize = 100;
  char * path = (char*)malloc(currentPathSize * sizeof(char));
  memset(path, '\0', currentPathSize);

  //sets up the game so we start in the starting room
  setupStart();

  //displays the starting room
  displayRoom();

  //creats the steps variable that stores the # steps the player took
  int steps = 0;
  //while loops unless player is currently in the end room
  while(!currentRoom.end){
    //increases the player path if the current path can't fit in original var
    //this will happen rarely as the person has to take 70 or more steps
    if(strlen(path) + 15 > currentPathSize){
      char temp[currentPathSize];
      memset(path, '\0', currentPathSize);
      strcpy(temp, path);
      free(path);
      currentPathSize = 2 * currentPathSize;
      char * path = (char*)malloc(currentPathSize * sizeof(char));
      strcpy(path, temp);
    }
    //gets the input of the user and increases the steps if the input was valid
    if(getInput(path)){
      steps++;
    }
  }
  //prints ending text
  printf("\n\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
  printf("YOU TOOK %i STEPS. YOUR PATH TO VICTORY WAS:\n%s", steps, path);
}

//sets up the currentRoom so the player is in the starting room
void setupStart(){
  //array of all possible room names
  char * names[10] = {"Bed","Study","Ball",
			"Bath","Kitchen","Basement",
			"Lounge","Attic","Dining","library"};

  int i;
  //loops through the names array and checks to see if it's the starting room
  for(i = 0; i < 10; i++){
    //loads the room, if it doesn't exist it returns
    loadRoom(names[i]);
    if(currentRoom.start){
      return;
    }
  }
  //if no room is the starting room it prints an error message
  printf("Error setupStart\n");
}

//get the directory that holds the rooms
void getFileDir(){
  //creates a stat struct to get information of a file
  struct stat dir;
  //creates a dirent struct to hold a file
  struct dirent *file;
  //opens the current directory
  DIR *curDr = opendir(".");

  int lastmodified = 0;
  //loops through all files in the current directory
  while((file = readdir(curDr)) != NULL){
      //gets the information on the current file
      stat(file->d_name, &dir);
      //checks if current file is a directory and is not . dir or .. dir
      if(S_ISDIR(dir.st_mode) &&
          dir.st_mtime > lastmodified &&
          strcmp(file->d_name, ".") != 0 &&
          strcmp(file->d_name, "..") != 0){

        //sets the last modified int to the current directorys modified time
        lastmodified = dir.st_mtime;
        //copys that directory into our directory variable
        strcpy(directory, file->d_name);
        //adds the /
        strcat(directory, "/");
      }
  }
  //closes the current directory
  closedir(curDr);
}

//gets the input from a user and interpets it
int getInput(char * path){
  //runs the timeThread to caculate the current time
  pthread_join(timeThread, NULL);
  //creates our input variable
  char * input;
  size_t bufferSize = 30;
  input = (char*) malloc(bufferSize * sizeof(char));;
  //gets input
  getline(&input, &bufferSize, stdin);
  //removes the new line character and replaces it with a null terminator
  input[strlen(input)-1] = '\0';
  //compares input to see if user wants the current time
  if(strcmp(input, "time") == 0){
    //opens time file
    int file_descriptor = open(timePath, O_RDONLY);
    char buffer = 'i';
    char word[100];
    memset(word, '\0', 100);
    //reads all characeters in time file
    while(read(file_descriptor, &buffer, 1)){
      word[strlen(word)] = buffer;
    }
    //prints out time file
    printf("\n%s\n", word);
    //displays the current room
    displayRoom();
    return 0;
  }

  //loops through to see if input is any connecting room
  int i;
  for(i = 0; i < currentRoom.numCon; i++){
    //compares to current rooms connections
    if(strcmp(input, currentRoom.connections[i]) == 0){
      //reinitalizes current room
      initalizeRoom();
      //loads current room
      loadRoom(input);
      //adds input to players path
      strcat(input, "\n");
      strcat(path, input);
      //displays new room if not the end
      if(currentRoom.end != 1){
        displayRoom();
      }
      //returns 1 to indicate valid input
      return 1;
    }
  }
  //prints error message if input is not recognizable
  printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
  //redisplays current room
  displayRoom();
  //returns 1 to indicate invalid input
  return 0;
}

//displays current room
void displayRoom(){
  //prints the current location
  printf("\nCURRENT LOCATION: %s\n", currentRoom.name);
  int i;
  printf("POSSIBLE CONNECTIONS: ");
  //prints all connections in current room execpt last
  for(i = 0; i < currentRoom.numCon - 1; i++){
    printf("%s, " , currentRoom.connections[i]);
  }
  //prints last connection so formated correctly
  printf("%s." , currentRoom.connections[currentRoom.numCon - 1]);
  printf("\nWHERE TO? >");
}

//loads the room given by the char input
void loadRoom(char * roomA){
  //clears current room
  initalizeRoom();
  //gets the path to the room we are loading
  char path[30];
	memset(path, '\0', 30);
	sprintf(path,"%s%s%s", directory, roomA, "_room");

  //opens the file we are loading
  int file_descriptor = open(path, O_RDONLY);
  //if not an actual room it returns
  if (file_descriptor < 0){
      return;
  }

  //buffer for reading
	char buffer = 'i';
	char word[25];
  memset(word, '\0', 25);

  //reads until end of file
	while(read(file_descriptor, &buffer, 1)){
    //only loads charcters into the buffer not spaces or new lines
		if(buffer != ' ' &&  buffer != '\n'){
			word[strlen(word)] = buffer;
		}else{
      //if there is a space or a new line checks if it isa keyword we are
      //looking for
			if(strcmp(word, "CONNECTION") == 0){
        //loads a connection into the new room adds 1 to the connection
        getNextWord(file_descriptor, word);
        getNextWord(file_descriptor, word);
        strcpy(currentRoom.connections[currentRoom.numCon], word);
        currentRoom.numCon += 1;
			}else if(strcmp(word, "NAME:") == 0){
        //loads the name of the room into the current room
        getNextWord(file_descriptor, word);
        strcpy(currentRoom.name, word);
      }else if(strcmp(word, "TYPE:") == 0){
        //sets type flags if it is a starting room or an end room
        getNextWord(file_descriptor, word);
        if(strcmp(word, "START_ROOM") == 0){
          currentRoom.start = 1;
        }else if(strcmp(word, "END_ROOM") == 0){
          currentRoom.end = 1;
        }
      }
      //resets word if not a keyword
      memset(word, '\0', 25);
		}
	}
  //closes file
  close(file_descriptor);
}

//gets the next word in a currently opened file
void getNextWord(int file_descriptor, char * output){
  //creation of the buffer
  char buffer = 'i';
  char word[25];
  memset(word, '\0', 25);
  memset(output, '\0', 25);

  //reads until it hits a new line space or end of file
  while(read(file_descriptor, &buffer, 1)){
    if(buffer == '\n' || buffer == ' '){
      break;
    }
    //loads character into buffer
    word[strlen(word)] = buffer;
    //errors if buffer overflows
    if(strlen(word) > 24){
          printf("ERROR getNextWord Out of space");
          break;
    }
  }
  //coppys the next word into the output
  strcpy(output, word);
  //we don't return the next word because it would be deleted once the function
  //pop off the stack
}

//initalizes the room setting all values to 0 and name and connections to
//nul terminators
void initalizeRoom(){
  currentRoom.start = 0;
  currentRoom.end = 0;
  currentRoom.numCon = 0;
  int i;
  for(i = 0; i < 7; i++){
    memset(currentRoom.connections[i], '\0', 9);
  }
  memset(currentRoom.name, '\0', 9);
}
