#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[]){
   if(argc!=2)	exit(1);		//not right amount of arguments
   
   int length=atoi(argv[1]), i, rand;	//init all variables 
   char randomLetter;
   char *string = malloc(sizeof(char)*length+1);
   //sets to all newlines so last char is a newline
   memset(string, '\n', length+1);

   srand(time(NULL));

   for(i=0; i<length; i++)  string[i]="ABCDEFGHIJKLMNOPQRSTUVWXYZ "[random () % 27];      //random char from string
   printf("%s", string);   //goes to stdout

   return 0;
}
