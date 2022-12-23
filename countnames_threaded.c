/**
 * Description: This program uses multi-threaded programming and locking shared variables. 
 * Two threads update the same count of names in parallel to output how many times 
 * each name is repeated.
 * Author names: Manroop Gill, Grace Xiaoli Zheng
 * Author emails: manroop.gill@sjsu.edu, xiaoli.zheng@sjsu.edu  
 * Last modified date: 12/1/2022
 * Creation date: 11/28/2022
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <ctype.h>

/*****************************************
//CS149 Fall 2022
//Template for assignment 6
//San Jose State University
//originally prepared by Bill Andreopoulos
*****************************************/


//thread mutex lock for access to the log index
//TODO you need to use this mutexlock for mutual exclusion
//when you print log messages from each thread
pthread_mutex_t tlock1 = PTHREAD_MUTEX_INITIALIZER;


//thread mutex lock for critical sections of allocating THREADDATA
//TODO you need to use this mutexlock for mutual exclusion
pthread_mutex_t tlock2 = PTHREAD_MUTEX_INITIALIZER; 


//thread mutex lock for access to the name counts data structure
//TODO you need to use this mutexlock for mutual exclusion
pthread_mutex_t tlock3 = PTHREAD_MUTEX_INITIALIZER; 


void* thread_runner(void*);
pthread_t tid1, tid2;

//struct points to the thread that created the object. 
//This is useful for you to know which is thread1. Later thread1 will also deallocate.
struct THREADDATA_STRUCT
{
  pthread_t creator;
};
typedef struct THREADDATA_STRUCT THREADDATA;

THREADDATA* p=NULL;


//variable for indexing of messages by the logging function.
int logindex=0;
int *logip = &logindex;


//The name counts.
struct NAME_STRUCT {
	char name[30];
	int count;
};
typedef struct NAME_STRUCT THREAD_NAME;


//node with name_info for a linked list
struct NAME_NODE {
	THREAD_NAME name_count;
	struct NAME_NODE *next;
};

#define HASHSIZE 26
static struct NAME_NODE *hashTable[HASHSIZE]; 

/**
 * This function is to save the name from the first character.
 * assumption: every string has at least one character.
 * input parameter: char first
 * returns: integer
 **/
int hash(char first) {
  return first%26;
}

//to see if the name is found or not found
struct NAME_NODE *find(char* name) { 
	struct NAME_NODE *ptr;
	for (ptr = hashTable[hash(name[0])]; ptr != NULL; ptr = ptr->next)
		if (strcmp(name, ptr->name_count.name) == 0)
			return ptr; 
	return NULL; 
}

//to save the names and count in the structure
struct NAME_NODE *insert(char* name){
	struct NAME_NODE *ptr;
	if((ptr=find(name)) == NULL){ //if the name isnt in list
		ptr = (struct NAME_NODE *) malloc(sizeof(*ptr));
		if (ptr == NULL)
			return NULL;
		//add name and then increase count
		strcpy(ptr->name_count.name,name); 
		ptr->name_count.count = 1;
		int hashval = hash(name[0]);
		ptr->next = hashTable[hashval];
		hashTable[hashval] = ptr;
	} else {
		//add to count if the name is already there
		ptr->name_count.count = ptr->name_count.count + 1;
	}
	return ptr;
}

/**
 * This function is to free the nodes in the hashtable.
 * assumption: there is something that needs to be freed.
 * input parameter: no parameter
 * returns: nothing
 **/
void freeHashTable() {
	struct NAME_NODE *ptr;
	for(int i = 0; i < HASHSIZE; i++){
		while(hashTable[i]!= NULL){
		ptr = hashTable[i];
		hashTable[i] = ptr->next;
		free(ptr);
		}
	}
}

//main function
int main(int argc, char *argv[]) {
  
	//to make sure user enteres exactly two files
	if (argc != 3){
		printf("Please enter exactly two input files\n");
		return 0;
	} 

	printf("create first thread\n");
	pthread_create(&tid1,NULL,thread_runner,argv[1]);
  
	printf("create second thread\n");
	pthread_create(&tid2,NULL,thread_runner,argv[2]);
  
	printf("wait for first thread to exit\n");
	pthread_join(tid1,NULL);
	printf("first thread exited\n");

	printf("wait for second thread to exit\n");
	pthread_join(tid2,NULL);
	printf("second thread exited\n");

	//print all the names and the times they show up
	struct NAME_NODE *ptr;
	for(int i = 0; i < HASHSIZE; i++){
		ptr = hashTable[i];
		while(ptr != NULL){
			printf("%s: %d\n",ptr->name_count.name,ptr->name_count.count);
			ptr = ptr->next; //go to the next index
		}
	}

	freeHashTable(); //free the hash table
	exit(0);
}//end main

/**
 * This function runs inside each thread
 * assumption: there are threads
 * input parameter: void* x
 * returns: void*
 **/
void* thread_runner(void* x) {
	pthread_t me;
	FILE *file;
	char* fileName = (char*) x;
	me = pthread_self();
  
	//put the timer.c file (from canvas) here to print the time information
	// variables to store date and time components
	int hours, minutes, seconds, day, month, year;
	// time_t is arithmetic time type
	time_t now;
	// Obtain current time
	// time() returns the current time of the system as a time_t value
	time(&now);
	
    char time [30]; 
    // localtime converts a time_t value to calendar time and 
	// returns a pointer to a tm structure with its members 
	// filled with the corresponding values
	struct tm *local = localtime(&now);

    for(int i = 0; i<30; i++){
      time[i] = 0;
    }

	hours = local->tm_hour;       // get hours since midnight (0-23)
    minutes = local->tm_min;      // get minutes passed after the hour (0-59)
    seconds = local->tm_sec;      // get seconds passed after minute (0-59)

    day = local->tm_mday;         // get day of month (1 to 31)
    month = local->tm_mon + 1;    // get month of year (0 to 11)
    year = local->tm_year + 1900; // get year since 1900

    
    // print local time
    if (hours < 12) // before midday
      sprintf(time,"%02d/%02d/%d %02d:%02d:%02d am", day, month, year, hours, minutes, seconds);
    else  // after midday
      sprintf(time,"%02d/%02d/%d %02d:%02d:%02d pm", day, month, year, hours - 12, minutes, seconds);
      
	logindex++;
	printf("Logindex %d, thread %ld, PID %d, %s: This is thread %ld (p=%p)\n", logindex,(long)me, getpid(), time, (long)me, p);
  
  
	pthread_mutex_lock(&tlock2); // critical section starts
	if (p==NULL) {
		p = (THREADDATA*) malloc(sizeof(THREADDATA));
		p->creator=me;
	}
	pthread_mutex_unlock(&tlock2);  // critical section ends

	if (p!=NULL && p->creator==me) {
		logindex++;
		printf("Logindex %d, thread %ld, PID %d, %s: This is thread %ld and I created THREADDATA %p\n", logindex,(long)me, getpid(), time, (long)me, p);
	} else {
		logindex++;
		printf("Logindex %d, thread %ld, PID %d, %s: This is thread %ld and I can access the THREADDATA %p\n", logindex,(long)me, getpid(), time, (long)me, p);
	}

	/**
	* //TODO implement any thread name counting functionality you need. 
	* //Make sure to use any mutex locks appropriately
	*/
	file = fopen(fileName, "r"); //open file
	//check if file does not exist
	if(file == NULL){
		fprintf(stderr, "range: cannot open file %s\n",fileName);
	} else {
		logindex++;
		printf("Logindex %d, thread %ld, PID %d, %s: opened file %s\n",logindex,(long)me,getpid(), time , fileName);

		char names[30];
		int idx = 0;
		
		//read through file
		while(fgets (names, 30, file) != NULL ) {
			idx++;
			//put warning if the line is empty
			if(names[0]=='\n'||names[0]==0){
				fprintf(stderr,"Warning - file %s line %d is empty.\n",fileName,idx);
				continue;
			}
			if(names[0]==' '){
				continue;
			}

			//remove last character \n
			names[strlen(names)-1] = 0;
			struct NAME_NODE *ptr = find(names);
		  
			pthread_mutex_lock(&tlock3);
			if(ptr==NULL){
				insert(names);
			}else {
				ptr->name_count.count = ptr->name_count.count + 1;
			}
			pthread_mutex_unlock(&tlock3); //critical section ends
		}

	}

	pthread_mutex_lock(&tlock1);
	// TODO use mutex to make this a start of a critical section 
	if (p!=NULL && p->creator==me) {
		logindex++;
		printf("Logindex %d, thread %ld, PID %d, %s: This is thread %ld and I can access the THREADDATA\n", logindex,(long)me, getpid(), time, (long)me);
		free(p);
	}else {
		logindex++;
		printf("Logindex %d, thread %ld, PID %d, %s: This is thread %ld and I delete the THREADDATA\n", logindex,(long)me, getpid(), time, (long)me);
	}
	// TODO critical section ends
	pthread_mutex_unlock(&tlock1);  

	pthread_exit(NULL);
	return NULL;
}//end thread_runner
