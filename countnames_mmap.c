/**
 * Description: This program is an extension of assignment 2 and uses an mmap communicate between the files/processes.
 * It reads through the files and outputs how many times a name is repeated.
 * Author name: Manroop Gill
 * Author email: manroop.gill@sjsu.edu 
 * Last modified date: 11/30/2022
 * Creation date: 10/25/2022
 **/
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#define MAX_NAME 100
#define MAX_LENGTH 30

/**
 * This function is to see if the strings are equal.
 * assumption: the arrays are 30 or less characters
 * input parameter: char str[MAX_LENGTH], char str2[MAX_LENGTH]
 * returns: true or false 
 **/
bool IsEqual(char str[MAX_LENGTH], char str2[MAX_LENGTH]) {
    int i = 0;
    // This loop will terminate if we are about to have
    // an overflow or if the string is finished ('\0')
    while(i < MAX_LENGTH && str[i] != '\0') {
        if (str[i] != str2[i]) {
            return true;
        }
        i++;
    }
    return false;
}

struct Info {
    char name[MAX_LENGTH]; //single name with up to 30  chars
    int repition; //how many times a name is repeated
    bool counted; //the name exists
};

/**main function**/
int main (int argc, char *argv[]) {
    struct Info *mm = mmap(NULL, sizeof(struct Info)*100, PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    int size = 0; //size of name
    
    //everything initialized to null in the array
    for (int i = 0; i < MAX_NAME; i++){
        mm[i].name[0] = '\0'; //mm is the mmap
        mm[i].counted = false;
    }
    
	//go through each file
    for (int i = 1; i<argc; i++){
		
        char *fileName = argv[i]; //for warning, get the name of the files
        
        // call fork
        int childID = fork(); 
        if (childID == 0){
            char str[MAX_LENGTH];
            
            FILE *fp = fopen( fileName, "r");//open the files 

            // in case the file is null
            if(fp == NULL) {
                fprintf(stderr,"range: cannot open file %s \n",fileName);
                _Exit(2);
            }

            int j = 0; //variable to go through all of the lines through the file
            
            //go through the lines
            while(fgets (str, sizeof(str), fp) != NULL ) {
                j++;
                
                //check if there is an empty line and print this warning if there is 
                if (str[0] == '\n'){
                    fprintf(stderr,"Warning - file %s Line %d is empty.\n",fileName,j); //to stderr instead of stdout
                    continue;
                }
                
                if  (str[1] == '\n'){
                    continue;
                }
                
                int ind = 0;
				
				//check if the name is already in the array
                while (IsEqual(mm[ind].name,str) && mm[ind].counted){
                    ind++;
                }
                //If the name is not in the array tthen add it 
                if (!mm[ind].counted){
                    int i = 0;
                    while (str[i]!='\n'){
                        mm[ind].name[i]=str[i];
                        i++;
                    }
                    mm[ind].name[i]='\0';
                    mm[ind].repition = 1;
                    mm[ind].counted = true; //change it so it is counted
                    size++;
                } else mm[ind].repition++;
            }
            //close files and exit
            fclose(fp);
            exit(0);
        }
    }
    wait(NULL);
    
    int k = 0;
    //print output witth names and how many times repeated
    while (mm[k].counted && k<100) {
        printf("%s : %d\n", mm[k].name, mm[k].repition);
        k++;
    }
    return(0);
}



