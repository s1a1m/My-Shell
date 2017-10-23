#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <signal.h>

#define MAX_SIZE 513

//Trims input on both sides (Used in parse)
char* prepare(char *array){
	char* end;
	end = strlen(array) + array - 1;
	while(end > array && isspace(*end)){
		end--;
	}
while(isspace(*array)){
		array++;
	}
	//Replaces Null char
	*(end + 1) = '\0';
	return array;
}

int pycheck(char * array){
	char * ptr = array;
	int length = 0;
	if(strstr(ptr, ".py") !=NULL){
		ptr = strstr(ptr, ".py");
		ptr = prepare(ptr);
		while(*(ptr++) != '\0'){
			length++;
		}
		if(length == 3){
			printf("returning 1\n");
			return 1;
		}
		else{
			return 0;
		}
	}
	else{
		return 0;
	}
}

int ampCheck(char* array){//Returns 1 if ampersand at end of string
	char* ptr = array;
	int length = 0; 
	if(strstr(ptr, "&") != NULL){
		//printf("Command contains an ampersand!\n");
		ptr = strstr(ptr, "&"); //Cuts right before &
		ptr = prepare(ptr); 
		
		//Finds Length
		while(*(ptr++) != '\0'){
			length++;
		}
		//printf("Length: %d\n", length);
		if(length == 1){
			//printf("Ampersand found at end of command!\n");
			return 1;
		}else{
			//printf("Ampersand not at end of string...\n");
			return 0;
		}		
	}else{
		//printf("No ampersand found in command...\n");
		return 0;
	}
}

//prints current directory
void pwd(int background){

	char cwd[513];
	pid_t pid = fork();

	if(pid == 0){
		pid = getpid();
		if(getcwd(cwd, sizeof(cwd)) != NULL){
			printf("%s\n",cwd);
		}
		//TODO: Make sure this is not an error we need to check for
		else{
			printf("Error: cannot access current directory.\n");
		}
		kill(pid,SIGTERM);
	}
	else{
		//STILL NEED TO FIGURE OUT HOW TO USE &
		if (background == 0){
			wait(0);
		}
	}
}

//handles the cd command
void cd(char * token, int background){

	if (token == NULL){
		if (chdir(getenv("HOME"))!=0){
			printf("Error: Could not return to HOME directory.\n");
		}			
	}
	else {

		if (chdir(token) != 0){
			printf("Error: Could not find directory %s\n",token);
		}
	}
}

void parse_line(char * line, char * command){
	
	char * token;	
	static char * args[513];
	char * word;
	char * path = "/bin/";
	char arg[513];
	
	int count = 0;
	int background = 0;

		
	token = strtok(line," \n");
	
	args[count] = token;
	count++; 
	
	while(token != NULL) {
		token = strtok(NULL," \n");
		args[count] = token;
		count++;
	}
	
	background = ampCheck(command);
	
	if (background == 1){
		count--;
	}

	args[count] = NULL;
	
	strcpy(arg, args[0]);

	for (int i = 0; i < strlen(arg); i++){
		if (arg[i] == '\n'){
			arg[i] = '\0';
		}
	}
	
	token = strtok(command," \n");
	

	//while (token != NULL){

	if (strcmp(args[0],"exit") == 0){
					
		exit(0);

	}
	//checks for pwd
	else if(strcmp(args[0],"pwd") == 0){	
		
		pwd(background);
		
	}
	//checks for cd
	else if(strcmp(args[0],"cd") == 0){	
		
		token = strtok(NULL, " \n");
		cd(token, background);//call the cd fuction	

	}

	else {//NON BUILT IN COMMANDS
		

		pid_t pid = fork();
		if (pid < 0){
			printf("Error: fork unsucessful");
		}
		//if child process
		if ( pid == 0){
			//check for a python file as the first argument
			//execute accordingly
			if(pycheck(command) == 1){

				strcpy(arg,"python");
				int counter = 0;
				int counter2 = 0;
				while (args[counter]!= '\0'){
					counter++;
				}
				counter2=counter;
				counter--;

				while (counter >= 0){
					
					args[counter2] = args[counter];
					counter--;
					counter2--;
				}
				args[0] = arg;
			}
			pid = getpid();
			if(execvp(arg,args) == -1){
				printf("Error: Could not execute command %s\n",token);
				kill(pid,SIGTERM);
			}

		}
		else{
			if (background == 0){
				wait(0);
			}
		}
	}			
}

//This loop repeatedly prompts user for input
//Sends the input to parse_line()
void batch_loop(char ** batchArray){
	 
		int i = 0;
		char line[MAX_SIZE];
		char command[MAX_SIZE];

     while (batchArray[i] != '\0'){
			strcpy(line, batchArray[i]);
			strcpy(command, batchArray[i]);
			parse_line(line,command);
			i++;
			printf("***********************\n");
		}
}

//loops through the commands entered from the user and sends them to parse line
void shell_loop(){
	
	char line[MAX_SIZE];
	char command[MAX_SIZE];  

    do {
		printf("mysh > ");
		fgets(line, MAX_SIZE, stdin); 
		strcpy(command, line);
		if(strcmp(line,"\n") == 0){
			continue;
		}
		parse_line(line, command);		
	} while (strcmp(line,"exit\n")!=0);
}

int main ( int argc, char *argv[] ){
	
	//common method for reading batch file in from the command line prompt
	if(argc == 2){
  		unsigned char buffer[513]; 
  		size_t offset = 0; 
  		size_t bytes; 
  		
		int i; 

 		char transfer[513];
 		char* ptr; 
  		int fd = open (argv[1], O_RDONLY); 
 
  		char *array[513];
  		do {
    			bytes = read (fd, buffer, sizeof (buffer));
    			
    			for (int i = 0; i < sizeof(buffer); ++i){
    				transfer[i] = buffer[i];
    			}
    			int count = 0;
    			char *token = strtok(transfer, "\n");
    			
    			while(token != NULL){
    				array[count] = token;
    				token = strtok(NULL, "\n");
    				count++;
    			}
    			array[count] = NULL;

    			for (i = 0; i < bytes; i++){
					offset += bytes; 
				}
  		} while (bytes == sizeof (buffer)); 
   		close (fd); 
		batch_loop(array);
	}
	
	//call the shell loop
	shell_loop();

	return 0;

}
