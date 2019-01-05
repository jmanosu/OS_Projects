/*
* small shell
* Jared Tence
* 11/14/18
*/


#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>

//command struct that contains the params file in and out
struct command{
	char* params[514];
	char file_in[200];
	char file_out[200];
	int  background;
	int	 numParams;
};

//function definitions
void input(char * cInput);
void getCommand(char *, struct command *);
void toggleForeground();
void initCommand();
void checkStatus(int *, int *);
void setupFileIO(struct command *);
void addArg(char *, char *);
int getNextWord(char *, char *, int);

//foreground only boolean that will determine if the shell is in foreground
//only mode
int foregroundOnly;

//main function
int main(){
//defines ^C (SIGINT) signal to be ignored by parent and sibling shells
	struct sigaction SIGINT_action = {0};
	SIGINT_action.sa_handler = SIG_IGN;
	sigaction(SIGINT, &SIGINT_action, NULL);

//defines ^Z (SIGTSTP) signl to call toggleForeground
	struct sigaction SIGTSTP_action = {0};
  SIGTSTP_action.sa_handler = toggleForeground;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = SA_RESTART;
  sigaction(SIGTSTP, &SIGTSTP_action, NULL);

//creating input strings and childPIDS integers
	char * cInput = (char *)malloc(2050 * sizeof(char));
	int * childPIDS = (int *)malloc(20 * sizeof(int));
//creating cPIDSsize that holds the number of child processes
	int cPIDSsize = 0;
//struct command which will store the current command inputted
	struct command com;
//initalize command to set all memory for strings
	initCommand(&com);
//finished value will store wether the state is to continue
	int finished = 0;
//exitStatus is the last status of the command executed in the foreground
	int exitStatus = 0;
//initalizes foreground only variable to 0
	foregroundOnly = 0;
//loop until user wants to exit
	while(!finished){
//sets values in character array to null terminates to avoid array
		memset(cInput, '\0', 2050 * sizeof(char));
//flushes stdout before checking status and taking input
		fflush(stdout);
//checks status of child processes
		checkStatus(childPIDS, &cPIDSsize);
//gets input from user
		input(cInput);
//checks if input is a new line null terminator or comment
		if(cInput[0] != '\n' && cInput[0] != '\0' && cInput[0] != '#'){
//gets the command from the current input
			getCommand(cInput, &com);
//checks if command is exit, status, cd, or other command
// exit, status, and cd were handled seprately because chdir only changes
// the directory of the current process and status can be called by a
// terminated child
			if(strcmp(com.params[0], "exit") == 0){
//if exit setus finished boolean to 1
				finished = 1;
//checks if exitStatus
			}else if(strcmp(com.params[0], "status") == 0){
//prints a different statement depending on the exit status
				if(exitStatus == 256){
					printf("exit value 1\n", exitStatus);
				}else if(exitStatus == 2){
					printf("terminated by signal 2\n");
				}else{
					printf("exit value %i\n", exitStatus);
				}
//checks if command is a change directory command
			}else if(strcmp(com.params[0], "cd") == 0){
//checks if cd has no arguments so it changes directory to home directory or to
// the entered directory
				if(com.params[1] == NULL){
					chdir(getenv("HOME"));
				}else if(chdir(com.params[1]) == -1){
					printf("%s: no such file or directory\n", com.params[1]);
				}
			}else{
//creats a spawnPid that will hold the return of the fork which repesents what
// the current process is if 0 it is the child else if a positive number it is
// the parent
				pid_t spawnPid = -5;
				spawnPid = fork();
//switch statment depending on the state of the spawnPid it will exec
				switch (spawnPid) {
//if case -1 then fork didn't work and things are about to go to shit
					case -1:
						exit(1);
						break;
					case 0:
//sets sa_handler to default if it's a foreground process
						if(!com.background){
							SIGINT_action.sa_handler = SIG_DFL;
							SIGINT_action.sa_flags = 0;
							sigaction(SIGINT, &SIGINT_action, NULL);
						}
//setups file IO to redirect stdin and stdout
						setupFileIO(&com);
						if(execvp(com.params[0], com.params) == -1){
							printf("%s: no such file or directory\n", com.params[0]);
						}
						return 1;
					default:
//if the called execp is not a background process then it waits for it to return
						if(!com.background){
							exitStatus = -5;
							fflush(stdout);
							waitpid(spawnPid, &exitStatus, 0);
							if(exitStatus == 2){
								printf("terminated by signal 2\n");
							}
						}else{
//else prints background pid and adds it to the stack
							printf("background pid is %i\n", spawnPid);
							childPIDS[cPIDSsize] = spawnPid;
							cPIDSsize++;
						}
						break;
				}
			}
		}
	}

//frees allocated memory
	free(cInput);
	free(childPIDS);
	int i;
	for(i = 0; i < 514; i++){
		if(com.params[i] != NULL){
			free(com.params[i]);
		}
	}
	return 0;
}

//setsup of file in and out for redirect depending if it's a background and
// foreground process
void setupFileIO(struct command * com) {
//file points for target file in and and target file out, and result of open
//function
	int targetFI, targetFO, result;
	if(com->file_in[0] != '\0'){
		targetFI = open(com->file_in, O_RDONLY);
//errors if open fails
		if(targetFI == -1){
			printf("cannot open %s for input\n", com->file_in);
			exit(1);
		}
//redirects std in to file target
		result = dup2(targetFI, 0);
//exits if dup2 didn't work
		if(result == -1){exit(1);}
	}else if(com->background){
//if it's a background process then it opens dev/null to read from when not
// given a file redirect for input
		targetFI = open("/dev/null", O_RDONLY);
		if(targetFI == -1){
			printf("cannot open /dev/null for input\n");
			exit(1);
		}
		result = dup2(targetFI, 0);
		if(result == -1){exit(1);}
	}
//does the samething as done for input but for std out
	if(com->file_out[0] != '\0'){
		targetFO = open(com->file_out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if(targetFO == -1){
			printf("cannot open %s for output\n", com->file_out);
			exit(1);
		}
		result = dup2(targetFO, 1);
		if(result == -1){exit(1);}
	}else if(com->background){
		targetFO = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if(targetFO == -1){
			printf("cannot open /dev/null for output\n");
			exit(1);
		}
		result = dup2(targetFO, 1);
		if(result == -1){exit(1);}
	}
}

//checks the status of current child processes running and if terminated prints
// the pid of the child aswell as the exit status
void checkStatus(int * childPIDS, int * size){
	int	childExitMethod;
	int i;
//loops through current child processes array
	for(i = 0; i < *size; i++){
//gets waitpid returns 1 if child has terminated WNOHANG makes sure parent
// doesn't wait for child to terminate
		if(waitpid(childPIDS[i], &childExitMethod, WNOHANG)){
//prints differnt messages for differnt exit statuses
			if(childExitMethod == 256){
				printf("background pid %i is done: exit value 1\n", childPIDS[i]);
			}else if(childExitMethod == 15){
				printf("background pid %i is done: terminated by signal 15\n", childPIDS[i]);
			}else{
				printf("background pid %i is done: exit value %i\n", childPIDS[i], childExitMethod);
			}
//removes terminated child
			if(*size > 0){
				childPIDS[i] = childPIDS[*size - 1];
				*size = *size - 1;
			}
		}
	}
}

//toggles foregroundOnly variable, this function is called by the ^Z signal
void toggleForeground(){
//switches foregroundOnly to 0 -> 1 and vic versa and prints a message
	if(foregroundOnly){
		printf("\nExiting foreground-only mode\n: ");
		fflush(stdout);
		foregroundOnly = 0;
	}else{
		printf("\nEntering foreground-only mode (& is now ignored)\n: ");
		fflush(stdout);
		foregroundOnly = 1;
	}
}

//gets input from user and stores it into variable cInput
void input(char * cInput){
	printf(": ");
	size_t size = 2048;
	getline(&cInput, &size, stdin);
}

//initalizes com struct so that it's variables are allocated memory
void initCommand(struct command * com){
	int i;
	for(i = 0; i < 514; i++){
		com->params[i] = (char *)malloc(200 * sizeof(char));
	}
	com->numParams = 0;
	com->background= 0;
}

//translates and argument, mainly used to convert $$ to the current processes
//	ID within an argument
void addArg(char * arg, char * temp){
	int i;
	int j = 0;
//sets arg to null terminaters
	memset(arg, '\0',200 * sizeof(char));
//goes through temp string and translates it to an argument
	for(i = 0; i < strlen(temp); i++){
//checks if there is $$ in the argument
		if(i < strlen(temp) - 1 && temp[i] == '$' && temp[i+1] == '$'){
//inserts the process id to the argument
			char buffer[300];
			memset(buffer, '\0', 300 * sizeof(char));
			sprintf(buffer,"%i", (int)getpid());
			strcat(arg, buffer);
			j += strlen(buffer);
			i++;
		}else{
			arg[j] = temp[i];
			j++;
		}
	}
}

//gets the command from the input string cInput and sets file_in and file_out
// and sets background flag
void getCommand(char * cInput, struct command * com){
	int i;

//because the last string of the array of strings that is passed into execvp
// is null we need to rallocate that memory
	if(!com->params[com->numParams]){
		com->params[com->numParams] = (char *)malloc(200 * sizeof(char));
	}

//We have to loop through the params and reset them to null pointers to avoid
// the params containing old characters
	for(i = 0;i < 514; i++){
			memset(com->params[i], '\0', sizeof(com->params[i]));
	}
	memset(com->file_in, '\0', sizeof(com->file_in));
	memset(com->file_out, '\0', sizeof(com->file_out));
	com->background = 0;
	com->numParams  = 0;
	if(strlen(cInput) < 1){
		printf("returning from getCommand\n");
		return;
	}
//creats a buffer we will store the parsed cInput string into
	char * buffer = (char *)malloc(strlen(cInput)+1 * sizeof(char));
	i = 0;
//goes through the cInput string and adds arguments to command struct and names
// of th efile input and output redirects
	while(i < strlen(cInput)){
		i += getNextWord(cInput, buffer, i);
		if(strcmp(buffer, "<") == 0){
			i += getNextWord(cInput, buffer, i);
			addArg(com->file_in, buffer);
		}else if(strcmp(buffer, ">") == 0){
			i += getNextWord(cInput, buffer, i);
			addArg(com->file_out, buffer);
		}else if(strcmp(buffer, "&") == 0){
			if(foregroundOnly == 0){
				com->background = 1;
			}
		}else{
			addArg(com->params[com->numParams], buffer);
			com->numParams++;
		}
	}
//frees allocated buffer
	free(buffer);
	free(com->params[com->numParams]);
//sets last param to null so it can be passed into execp
	com->params[com->numParams] = NULL;
}

//gets the next word from a string
int getNextWord(char * input, char * output, int start){
//sets output string to null pointers
	memset(output, '\0', (strlen(input) + 1) * sizeof(char));
	int i;
//loops through input string and adds characters to output string
	for(i = 0; i < strlen(input); i++){
		//if current character is a space it breaks from loop
		if(input[i + start] == ' '|| input[i + start] == '\n' || input[i + start] == '\0'){
			break;
		}
		output[i] = input[i + start];
	}
	return i + 1;
}
