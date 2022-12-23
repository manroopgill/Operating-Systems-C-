/**
 * Description: This program manages and execute many commands. Each command will write its stdout and stderr to log files. 
 * Author names: Manroop Gill, Grace Xiaoli Zheng
 * Author emails: manroop.gill@sjsu.edu, xiaoli.zheng@sjsu.edu
 * Last modified date: 10/16/2022
 * Creation date: 10/1/2022
 **/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define cmdNum 100 
#define cmdLen 30 

int main(int argc, char *argv[]){

    char commands[cmdNum][cmdLen]; //array for list of commands
    char command[cmdLen]; //array used to read command
    char outputFile[cmdLen]; //array for output file
    char errFile[cmdLen]; //array for error file
    int numOfCom = 0; //number of commands
    
    
	//read commands in file
    while(fgets(command, cmdLen, stdin) != NULL){
        //change last characer to null
        command[strlen(command) - 1] ='\0';
		
		//copy to commands array
		strcpy(commands[numOfCom++], command);  
    }
	
    for (int i = 0; i < numOfCom; i++) {
        pid_t pid1 = fork(); // call fork
        //child process
        if (pid1 == 0) { 
			char *cmdArr[cmdLen]; //array to hold input per line
			char *current = strtok(commands[i], " "); //seperating commands by space
			int line = 0; //lines of commands
			
			//go through all strings and save to cmdArr and go to next string
			while (current != NULL) {
				cmdArr[line++] = current;
				current = strtok(NULL,  "  ");
			}
			cmdArr[line]  = NULL; //add null to end of array for execvp
			
			
			//create the files that save output and error messages
			sprintf(outputFile, "%d.out", getpid());
			sprintf(errFile, "%d.err", getpid());
			
			//write to output and error files
			int output = open(outputFile, O_RDWR | O_CREAT | O_APPEND, 0777); //give permission to everyone
			dup2(output, 1); //1 is stdout
			int error = open(errFile, O_RDWR | O_CREAT | O_APPEND, 0777);
			dup2(error, 2); //2 is stderr
			
			printf("Starting command %d: child %d pid of parent %d\n", i + 1, getpid(), getppid());
			
			//flush stdout so  it doesn't repeat
			fflush(stdout);
			fflush(NULL);
			
			//execute command
			execvp(cmdArr[0], cmdArr);
			
			//see if there is problem with command
			fprintf(stderr, "Error: child %d couldn't execute command\n", getpid());
			exit(2);
		} else if (pid1 < 0) { 
			fprintf(stderr, "Error with forking\n");
			exit(1);
		}
	}
	
    
    int status, pid;
    //parent process
    while((pid = wait(&status)) >  0) { 
		// create files based on pid
		sprintf(outputFile, "%d.out", pid);
        sprintf(errFile, "%d.err", pid);
        
        //write status messages in file by appending
        FILE *output = fopen(outputFile, "a");
        FILE *error = fopen(errFile, "a");
        fprintf(output, "Finished child %d pid of parent %d\n",pid, getpid()); 
        //exit code for error file
        if (WIFEXITED(status)){ 
			fprintf(error, "Exited with exitcode = %d\n",WEXITSTATUS(status)); 
		}//code for signal for error file
		else if (WIFSIGNALED(status)){ 
			fprintf(error, "Killed with signal %d\n",WTERMSIG(status)); 
		}
		
		//close files
		fclose(output);
		fclose(error);

   }
   return 0;
}

