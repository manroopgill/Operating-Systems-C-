/**
 * Description: This program stores the command lines and traces the memory usage and prints them too file named memtrace.out
 * Author names: Manroop Gill, Grace Xiaoli Zheng
 * Author emails: manroop.gill@sjsu.edu, xiaoli.zheng@sjsu.edu
 * Last modified date: 11/4/2022
 * Creation date: 10/25/2022
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>

/**
 *CS149 assignment#4 helper code is used.
**/


int ROW = 5;
int COL = 50;

/**
 * TRACE_NODE_STRUCT is a linked list of
 * pointers to function identifiers
 * TRACE_TOP is the head of the list is the top of the stack
**/
struct TRACE_NODE_STRUCT {
  char* functionid;                // ptr to function identifier (a function name)
  struct TRACE_NODE_STRUCT* next;  // ptr to next frama
};
typedef struct TRACE_NODE_STRUCT TRACE_NODE;
static TRACE_NODE* TRACE_TOP = NULL;       // ptr to the top of the stack


/* --------------------------------*/
/* function PUSH_TRACE */
/*
 * The purpose of this stack is to trace the sequence of function calls,
 * just like the stack in your computer would do.
 * The "global" string denotes the start of the function call trace.
 * The char *p parameter is the name of the new function that is added to the call trace.
 * See the examples of calling PUSH_TRACE and POP_TRACE below
 * in the main, make_extend_array, add_column functions.
**/
void PUSH_TRACE(char* p)          // push p on the stack
{
  TRACE_NODE* tnode;
  static char glob[]="global";

  if (TRACE_TOP==NULL) {

    // initialize the stack with "global" identifier
    TRACE_TOP=(TRACE_NODE*) malloc(sizeof(TRACE_NODE));

    // no recovery needed if allocation failed, this is only
    // used in debugging, not in production
    if (TRACE_TOP==NULL) {
      printf("PUSH_TRACE: memory allocation error\n");
      exit(1);
    }

    TRACE_TOP->functionid = glob;
    TRACE_TOP->next=NULL;
  }//if

  // create the node for p
  tnode = (TRACE_NODE*) malloc(sizeof(TRACE_NODE));

  // no recovery needed if allocation failed, this is only
  // used in debugging, not in production
  if (tnode==NULL) {
    printf("PUSH_TRACE: memory allocation error\n");
    exit(1);
  }//if

  tnode->functionid=p;
  tnode->next = TRACE_TOP;  // insert fnode as the first in the list
  TRACE_TOP=tnode;          // point TRACE_TOP to the first node

}/*end PUSH_TRACE*/

/* --------------------------------*/
/* function POP_TRACE */
/* Pop a function call from the stack */
void POP_TRACE()    // remove the top of the stack
{
  TRACE_NODE* tnode;
  tnode = TRACE_TOP;
  TRACE_TOP = tnode->next;
  free(tnode);

}/*end POP_TRACE*/



/* ---------------------------------------------- */
/* function PRINT_TRACE prints out the sequence of function calls that are on the stack at this instance */
char* PRINT_TRACE()
{
  int depth = 50; //A max of 50 levels in the stack will be combined in a string for printing out.
  int i, length, j;
  TRACE_NODE* tnode;
  static char buf[100];

  if (TRACE_TOP==NULL) {     // stack not initialized yet, so we are
    strcpy(buf,"global");   // still in the `global' area
    return buf;
  }

  /* peek at the depth(50) top entries on the stack, but do not
     go over 100 chars and do not go over the bottom of the
     stack */

  sprintf(buf,"%s",TRACE_TOP->functionid);
  length = strlen(buf);                  // length of the string so far
  for(i=1, tnode=TRACE_TOP->next;
                        tnode!=NULL && i < depth;
                                  i++,tnode=tnode->next) {
    j = strlen(tnode->functionid);             // length of what we want to add
    if (length+j+1 < 100) {              // total length is ok
      sprintf(buf+length,":%s",tnode->functionid);
      length += j+1;
    }else                                // it would be too long
      break;
  }
  return buf;
} /*end PRINT_TRACE*/

/**
 * This function performs realloc and should also print info about memory usage. It overwrites C library realloc.
 **/
void* REALLOC(void* p,int t,char* file,int line)
{
	PUSH_TRACE("REALLOC");
    p = realloc(p,t);
    printf("File %s, line=%d, function=%s reallocated the memory segment to address %p to a new size %d\n", file, line, PRINT_TRACE(), p, t);
    POP_TRACE();
    return p;
}


/**
 * This function performs malloc and should also print info about memory usage. It overwrites C library malloc
 **/
void* MALLOC(int t,char* file,int line)
{
    PUSH_TRACE("MALLOC");
    void* p;
    p = malloc(t);
    printf("File %s, line=%d, function=%s allocated new memory segment to address %p to size %d\n", file, line, PRINT_TRACE(), p, t);
    POP_TRACE();
    return p;
}

/**
 * This function performs realloc and should also print info about memory usage. It overwrites C library free
 **/
void FREE(void* p,char* file,int line)
{
    PUSH_TRACE("FREE");
    printf("File %s, line=%d, function=%s deallocated the memory segment to address %p\n", file, line, PRINT_TRACE(), p);
    free(p);
    POP_TRACE();
}

#define realloc(a,b) REALLOC(a,b,__FILE__,__LINE__)
#define malloc(a) MALLOC(a,__FILE__,__LINE__)
#define free(a) FREE(a,__FILE__,__LINE__)

// Make struct to help linked list
struct CmdNode {
    char* command;
    int index;
    struct CmdNode *next;
};

/**
 * This function creates a new node.
 **/
void CreateCmdNode(struct CmdNode *thisNode, char cmd[COL], int ind, struct CmdNode *nextCmd) {
    PUSH_TRACE("CreateCmdNode");
    thisNode->command = (char*)malloc(sizeof(char) * COL);
    strcpy(thisNode->command, cmd);     
    thisNode->index = ind;   
    thisNode->next = nextCmd;
    POP_TRACE();
    return;
}

/**
 * Print all commands recursively out to a file
 **/
void PrintNodes(struct CmdNode *start) {
    if(start == NULL) {
        return;
    }
    printf("command[%d] =  %s\n", start->index-1, start->command);
    PrintNodes(start->next);
}

/**
 * This function is intended to demonstrate how memory usage tracing of malloc and free is done
 **/
void make_extend_array()
{
       PUSH_TRACE("make_extend_array");

	
	//make command array
	char **commandArr;
	commandArr = (char**) malloc(sizeof(char*) * ROW);
	for(int i = 0; i < ROW; i++) {
		commandArr[i] = (char*) malloc(sizeof(char) * COL);
	}
	
	// read the commands from the array
	int count = 0; //count the rows
	char command[COL];
	while (fgets(command, COL, stdin)) {
		//remove last character from command if it is \n
		if(command[strlen(command) - 1] =='\n')
			command[strlen(command) - 1] ='\0';
		strcpy(commandArr[count], command);

		count++;

		if (count >= ROW) {
			commandArr = (char**) realloc(commandArr, sizeof(char*) * (ROW + 1));
			commandArr[ROW] = (char*) malloc(sizeof(char) * COL);
			ROW += 1;
		}

    }
    
    // Command stored into linked list
    struct CmdNode *currentCmd = (struct CmdNode*)malloc(sizeof(struct CmdNode));
    CreateCmdNode(currentCmd, commandArr[0], 1, NULL);
    struct CmdNode *curr = currentCmd;
    for(int i = 1; i < count; i++) {
        struct CmdNode *nextCmd = (struct CmdNode*)malloc(sizeof(struct CmdNode));
        CreateCmdNode(nextCmd, commandArr[i], i+1, NULL);
        curr->next = nextCmd;
        curr = nextCmd;
    }
    
    // Deallocate the char**
    for(int i = 0; i < ROW; i++) {
        free((void*)commandArr[i]);
    }
    free((void*)commandArr);
    
    // Print the commands in the linked list
    PrintNodes(currentCmd);

    // Deallocate the linked list
    struct CmdNode *ptr = currentCmd;
    while(currentCmd != NULL) {
            ptr = currentCmd;
            currentCmd = currentCmd->next ;
            free(ptr->command);
            free(ptr);
    }


POP_TRACE();
         return;
}//end make_extend_array

// function main
int main()
{

		int outputFile = open("memtrace.out", O_CREAT|O_APPEND|O_WRONLY, 0777); //give permission to everyone
		dup2(outputFile, 1); //1 is stdout

        PUSH_TRACE("main");

		//call the make_extend_array function
		make_extend_array();

        POP_TRACE();

        close(outputFile);

        //free(TRACE_TOP);
        return(0);
}// end main
