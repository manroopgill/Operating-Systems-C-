/**
 * Description: This program counts how many times each name is repeated in a file.
 * Author names: Manroop Gill, Grace Xiaoli Zheng
 * Author emails: manroop.gill@sjsu.edu, xiaoli.zheng@sjsu.edu
 * Last modified date: 9/12/2022
 * Creation date: 9/7/2022
 **/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char **argv) {
	
	//create file pointer and open file specified by user
	FILE *file = fopen(argv[1], "r");
    char *str = (char *)calloc(30, sizeof(char));         //string array to read the names from the file
    char **str2 = (char **)malloc(100*sizeof(char *));   //string array to save the names
    int *values = (int *)malloc(100*sizeof(int));       //array for how many times a name is repeated
    int current_array_length = 100;
    int i = 0;
    int m = 1;            //integer to help count the empty line 

	//this for loop initialize each element in array to be -1
	for (int j = 0; j < 100; j++) {
        values[j] = -1;
    } 
    
	//print error message if unable to open file
    if (file == NULL) {
        printf("Error: cannot open file\n");
        return 1;
    }
     
    //this while loop uses fgets to read textfile
    while (fgets(str, 30, file)) {
        //if the line isnt empty, save the name to string 2 without \n
        if (strlen(str) != 0){
            //searches string until \n , then replace with NULL
            char *ptr = strchr(str, '\n');
            if (ptr)
                *ptr  = '\0';

            //strcpy(str2[i], str);
            str2[i] = strdup(str);
            i++;
        }
        else {
			fprintf(stderr, "Warning - Line %d is empty\n", i+m);
			m++; //add a number when the line is empty so they are not ignored, helps for warning
        }
        if (i >= current_array_length) { //allocate more memory when needed
			current_array_length += 100; str2 = (char **) realloc(str2, current_array_length*sizeof(char *)); values = (int *) realloc(values, current_array_length*sizeof(int)); }
    }
    //this for loop compares all the name strings and count how many times they appear
    for (int j = 0; j < i; j++) {
        int count  = 1;  //iniitialize every name to appear one time
        for (int k = j + 1; k < i; k ++) {
			//use strcmp to compare two names to see if they are the same
            if (strcmp(str2[j], str2[k]) == 0) {
                count++;  
                values[k] = 0;
            }
        }
        //if the name is repeated, update value to be count
        if (values[j] != 0) {
            values[j] = count;
        }
        //print output with all the names and times they appeared
        if (values[j] != 0){
            printf("%s: %d\n", str2[j], values[j]);
		}
        
    }
    
    //close file
    fclose(file);
    return 0; 
}
