/**
 * Description: This program uses a hashtable data structure to save the commands. 
 * It also makes an .out and .err file for each command which records the time info and exit status.
 * This code resembles the system daemon that restarts processes that got killed.
 * Author names: Manroop Gill, Grace Xiaoli Zheng
 * Author emails: manroop.gill@sjsu.edu, xiaoli.zheng@sjsu.edu
 * Last modified date: 11/16/2022
 * Creation date: 11/13/2022
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
struct nlist
{
    struct nlist *next; /* next entry in chain */
    struct timespec starttime, finishtime; /* finishtime */
    int index; /* index // this is the line index in the input text file
*/
    pid_t pid; /* pid // the process id. you can use the pid result of
wait to lookup in the hashtable */
    char *command; /* char *command; // command. This is good to store for
when you decide to restart a command */
};
#define HASHSIZE 101
static struct nlist *hashtab[HASHSIZE]; /* pointer table */
/* This is the hash function: form hash value for string s */
/* TODO change to: unsigned hash(int pid) */
/* TODO modify to hash by pid . */
/* You can use a simple hash function: pid % HASHSIZE */

unsigned hash(int pid){
    return pid % HASHSIZE;
}

/* lookup: look for s in hashtab */
/* TODO change to lookup by pid: struct nlist *lookup(int pid) */
/* TODO modify to search by pid, you won't need strcmp anymore */
/*
This is traversing the linked list under a slot of the hash
table. The array position to look in is returned by the hash function
*/
struct nlist *lookup(int pid){
    struct nlist *np; //node pointer
    for (np = hashtab[hash(pid)]; np != NULL; np = np->next){
        if (np->pid == pid){
            return np; /* found */
        }
    }
    return NULL; /* not found */
}
/* insert: put (name, defn) in hashtab */
/* TODO: change this to insert in hash table the info for a
new pid and its command */
/* TODO: change signature to:
struct nlist *insert(char *command,int pid, int index). */
/* This insert returns a nlist node.
Thus when you call insert inyour main function */
/* you will save the returned nlist node in a variable (mynode). */
/* Then you can set the starttime and finishtime from your main function: */
/* mynode->starttime = starttime; mynode->finishtime = finishtime; */
//char *name, char *defn
struct nlist *insert(char *command, int pid, int index)
{
    struct nlist *np;
    unsigned hashval;
//TODO change to lookup by pid. There are 2 cases:
    if ((np = lookup(pid)) == NULL)
    {
/* case 1: the pid is not
found, so you have to create it with malloc.
Then you want to set the pid, command and index */
        np = (struct nlist *)malloc(sizeof(*np));
        if (np == NULL || ((np->command = strdup(command)) == NULL))
        {
            return NULL;
        }
        //set up next node
        np->pid = pid;
        np->index = index;
        hashval = hash(pid);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    }
    else
    {
		//free the previous command
        free((void *)np->command); 
        //set again the command and index 
        if ((np->command = strdup(command)) == NULL){
            return NULL;
        }
        np->index = index;
    }
    return np;
}
/** You might need to duplicate the command string to ensure
you don't overwrite the previous command each time a new line
is read from the input file. Or you might not need to duplicate it.
It depends on your implementation. **/
char *strduplicate(char *s) /* make a duplicate of s */
{
    char *p;
    p = (char *)malloc(strlen(s) + 1); /* +1 for \0 */
    if (p != NULL)
    {
        strcpy(p, s);
    }
    return p;
}

/**
 * This function read and parse the commands and creates the err and out files for each command. 
 * assumption: there are commands to be read
 * input parameter: char cmdArr[], int cmdNum, int restartStatus
 * returns: nothing (void)
 **/
void exec(char cmdArr[], int cmdNum, int restartStatus) {
	//create out and err file names string array
    int pid = getpid();
    char out_pid[10];
    char err_pid[10];
    
	//add pid number to array
    sprintf(out_pid, "%d", pid);
    sprintf(err_pid, "%d", pid);
    
	//add .out and .err to end 
    char adding[] = ".out";
    strcat(out_pid, adding);
    char add[] = ".err";
    strcat(err_pid, add);
    
    //create files
    int f_out = open(out_pid, O_RDWR | O_CREAT | O_TRUNC, 0777);
    int f_err = open(err_pid, O_RDWR | O_CREAT | O_TRUNC, 0777);
		
	//use dup2 to copy of file descriptor
    dup2(f_out, fileno(stdout));
    dup2(f_err, fileno(stderr));
    
    //print restarting if command need to be restarted
    if(restartStatus){
        fprintf(stdout, "RESTARTING\n");
        fprintf(stderr, "RESTARTING\n");
    }
    fprintf(stdout, "Starting command %d: child %d pid of parent %d\n",
            cmdNum, getpid(), getppid());
            
	//flush stdout and stderr
    fflush(stdout);
    fflush(stderr);
    
    char *input[strlen(cmdArr)];
    char *token = strtok(cmdArr, " \n");
    int index = 0;
	//parse strings for execvp
    while (token != NULL) {
        input[index] = token;
        index++;
        token = strtok(NULL, " \n");
    }
    input[index] = NULL;
    
    //flush stdout and stderr, and close files
    fflush(stdout);
    fflush(stderr);
    close(f_out);
    
	//call execvp
    if (execvp(input[0], input) == -1) {
 
		//if execvp fails print failure 
        dup2(f_err, STDOUT_FILENO);
        dup2(f_err, STDERR_FILENO);
        perror("Execvp failed: \n");
        fflush(stdout);
        fflush(stderr);
        close(f_err);
    }
}

int main(int argc, char *argv[]){
 
    char cmd[50]; 
    int cmdIdx = 0; //index of command
    struct timespec start, finish;
    
    //read the commands
    while (fgets(cmd, 50, stdin) != NULL){ 
        cmdIdx++;
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        //call fork
        pid_t pid = fork(); 
        //if fork fails
        if (pid < 0) {
            fprintf(stderr, "fork failed\n"); 
            exit(1);
        }
        //child process
        else if (pid == 0) {
            exec(cmd, cmdIdx, 0);
            exit(2); //2 means failed
        }
        else {
            struct nlist* np = insert(cmd, pid, cmdIdx);
            np->starttime = start;
        }
    }

    int status; 
    int cpid; //child pid 
    int ppid = getpid(); 
    
    //parent process
    while ((cpid = wait(&status)) > 0) {
        clock_gettime(CLOCK_MONOTONIC, &finish);
        struct nlist* np = lookup(cpid);
        np->finishtime = finish;
        double elapsedtime = ((double)finish.tv_sec + 1.0e-9*finish.tv_nsec) -
                          ((double)start.tv_sec + 1.0e-9*start.tv_nsec);; 
                          
        char fileBuff[20]; //create buffer array to save name of file
        sprintf(fileBuff, "%d.err", cpid); // create name for pid.err file
        //open files to append to 
        int f_err = open(fileBuff, O_RDWR | O_CREAT | O_APPEND, 0777); 
        sprintf(fileBuff, "%d.out", cpid);
        int f_out = open(fileBuff, O_RDWR | O_CREAT | O_APPEND, 0777); 
        //print finished message and time
        dprintf(f_out, "Finished child %d of parent %d\n", cpid, ppid);
        dprintf(f_out, "Finished time at %ld. Runtime duration %f\n", np->finishtime.tv_sec, elapsedtime);
        
        //exit statuses
        if (WIFEXITED(status)) {
            dprintf(f_err, "Exited with exitcode = %d\n", WEXITSTATUS(status));
            if(elapsedtime <= 2) {
                dprintf(f_err, "spawning too fast\n");
            }
        }
        //if status is killed
        else if (WIFSIGNALED(status)) {
            dprintf(f_err, "Killed with signal %d\n", WTERMSIG(status));//print kill signal message
            if(elapsedtime <= 2) {
                dprintf(f_err, "spawning too fast\n");
            }
        }
		//if over 2 sec then restart
        if (elapsedtime > 2) {
            clock_gettime(CLOCK_MONOTONIC, &start);
            pid_t pid = fork(); 
            //check for fork failure
            if (pid < 0){
                fprintf(stderr, "fork failed\n"); 
                exit(1);
            }
            //child process
            else if (pid == 0) {
                exec(np->command, np->index, 1); //execute command again
                exit(2); 
            }
            else {
                struct nlist* np1 = insert(np->command, pid, np->index);
                np1->starttime = start;
            }
        }
    }
    return 0;
}
