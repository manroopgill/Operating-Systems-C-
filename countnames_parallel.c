/**
 * Description: This program creates several child processes to read the files at the same time. 
 * Then they send the information through a pipe to the parent.
 * Then the parent print output with names and how many times they show up.
 * Author names: Manroop Gill, Grace Xiaoli Zheng
 * Author emails: manroop.gill@sjsu.edu, xiaoli.zheng@sjsu.edu
 * Last modified date: 9/28/2022
 * Creation date: 9/21/2022
 **/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>


struct Info {
    char str[30];
    char namep[100];
    bool notCounted; //boolean to check if name exists already or not yet
    int repition;
};

int main(int argc, char *argv[]) {
	//create a pipe
    int fd[2];
    if(pipe(fd) == -1){ //check if pipe fails
        fprintf(stderr, "Pipe failed");
        return 1;
    }
    
    
    pipe(fd);
    char str[30];         
    char str2[100][30];
    int repeat[30];         //array to count 
    struct Info strN[100];  //used to read from pipe
    char namesEnd[100][30];// all the names and the times they show up in the end
    int namesTotal = 0;    //to store how many total number of names there are
    int finalValues[100];  //to store how many times a name appears
    
    //initialize str2 to make sure its all empty
    for(int i = 0; i<100; i++){
        for(int j = 0; j<30; j++){
            str2[i][j] = '\0';
        }
    }
    
   for(int i = 1; i < argc; i++){
        int childid = fork();
        if(childid == 0){
            
            struct Info namep[100];
            
            //open the file
            FILE *file = fopen(argv[i], "r");
            //for printing out the file name in warning statement
            char *fileName = argv[i];
            
            
            //if  the file is not valid
            if (file == NULL){
                printf("range: cannot open file");
                return 1;
            }
            
            
	
    
    int values[100];      //array for how many times a name is repeated
    int i = 0;
    int m = 1;            //integer to help count the empty line 

	//this for loop initialize each element in array to be -1
	for (int j = 0; j < 100; j++) {
        values[j] = -1;
    } 
    
	
     
    //this while loop uses fgets to read textfile
    while (fgets(str, sizeof(str), file)) {
		//read string until \n 
        char *ptr = strchr(str, '\n');
		if (ptr)
            *ptr  = '\0';
        //if the line isnt empty, save the name to string 2 without \n 
        if (strlen(str) != 0){
            strcpy(str2[i], str);
            i++;
        }
        else {
			fprintf(stderr, "Warning - file %s line %d is empty.\n", fileName, i+m);
			m++; //add a number when the line is empty so they are not ignored, helps for warning
        }
    }
    //prepare to send to pipe by copying using struct
    for(int k = 0; k < 100; k++){
		strcpy(namep[k].namep, str2[k]);
		namep[k].namep[strlen(namep[k].namep)] = '\0';
		namep[k].repition = repeat[k]+1;
		namep[k].notCounted = true;
	}
	
	//close the file
	fclose(file);
	//write to pipe
	write(fd[1], namep, sizeof(namep));
	close(fd[1]);
	exit(0); //exiting is important else child will continue to fork its own children
        }
   }
       //the parent waits for each child to finish
    while((wait(NULL)) > 0){
        //read from the pipeline, and store in strN
        read(fd[0], strN, sizeof(strN));
        int i = 0;
        char nameList[100][30]; //array to hold names
        int occurrence[100];    //array to hold how many times a name repeated
        while(strN[i].notCounted){
			//copy the things read into an array
            strcpy(nameList[i], strN[i].namep);
            occurrence[i] = strN[i].repition;
            i++;
        }

        int k = 0;
        //check boolean to see if its counted
        while(strN[k].notCounted)
        {
            int p = 0;
            
            //compare all the names, if they arent same then go to next name
            while(strcmp(nameList[k], namesEnd[p]) != 0 && namesTotal>p){
                p++;
            }
            
            //add one to counter if the name already exists
            if(strcmp(nameList[k], namesEnd[p]) == 0){
                finalValues[p]+=occurrence[k];
            }
                //save the name that hasnt showed up
            else{
                strcpy(namesEnd[p],nameList[k]);
                finalValues[p] = occurrence[k];
                namesTotal++;
            }
            k++;
        }
    }

    //print result by going through all the names
    for (int i = 0; i < namesTotal; i++) {
        printf("%s: %d\n",namesEnd[i],finalValues[i]);
    }
	//return 0 if successful
    return 0;
}
