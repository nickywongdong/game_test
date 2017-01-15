#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int readFiles(FILE *key, FILE *plaintext, char **myKey, char **myText){			//checks the length of key generated
   char ch;
   int i=0, j=0, bufferLength;
   //get wc of both files
   while (1) {
      ch = fgetc(key);
      if (ch == EOF)	break;
      ++i;
   }

   while (1) {
      ch = fgetc(plaintext);
      if (ch == EOF)	break;
      ++j;
   }
   bufferLength=i+j+2;	//+2 for extra newline characters

   //if they lengths do not equal, exit
   if(i<j){
      printf("Keygen length: %d, Plaintext length: %d, key too short\n", i, j);
      fclose(key);
      fclose(plaintext);
      exit(1);
   }
   //rewind file pointers to store string in pointers
   rewind(key);
   rewind(plaintext);

   char *keyTemp=malloc(sizeof(char)*i), *textTemp=malloc(sizeof(char)*j);
   *myKey = malloc(sizeof(char)*i);
   *myText = malloc(sizeof(char)*j);

   i=0;
   j=0;
   while(1){
      ch=fgetc(key);
      if(ch==EOF)	break;
      keyTemp[i]=ch;
      ++i;
   }
   while(1){
      ch=fgetc(plaintext);
      if(ch==EOF)	break;
      textTemp[j]=ch;
      j++;
   }

   //copy temp into actual pointers from main
   sprintf(*myKey, keyTemp);
   sprintf(*myText, textTemp);
   //clean up
   free(keyTemp);
   free(textTemp);
   fclose(key);
   fclose(plaintext);
   return bufferLength;
}

int main(int argc, char *argv[])
{
   int socketFD, portNumber, charsWritten, charsRead;
   struct sockaddr_in serverAddress;
   struct hostent* serverHostInfo;
   //char buffer[256];

   if (argc < 4) { fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); exit(EXIT_FAILURE); } // Check usage & args

   // Set up the server address struct
   memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
   portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
   serverAddress.sin_family = AF_INET; // Create a network-capable socket
   serverAddress.sin_port = htons(portNumber); // Store the port number
   serverHostInfo = gethostbyname("localhost");	//host localhost by default
   //serverHostInfo = gethostbyname(argv[1]); // Convert the machine name into a special form of address
   if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(2); }
   memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

   // Set up the socket
   socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
   if (socketFD < 0) error("CLIENT: ERROR opening socket");

   // Connect to server
   if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
      error("CLIENT: ERROR connecting");

   // Sending data to server (or otp_end_d.c in this case)
   char *keyFile = argv[2];
   char *textFile = argv[1];		//setting filenames
   char *myKey, *myText;
   //*buffer;
   char buffer[500000];


   //send in the name (encryption) for verification
   char name[]="1";
   charsWritten=send(socketFD, name, strlen(name), 0);
   if(charsWritten<strlen(name)){
      printf("Warning, not completely written to server\n");
   }
   sleep(1);

   //open plaintext & key
   FILE *key = fopen(keyFile, "r");
   FILE *plaintext = fopen(textFile, "r");
   if(key==NULL || plaintext == NULL)	printf("error in opening file...\n");
   //parse files
   int bufferLength = readFiles(key, plaintext, &myKey, &myText);
   //buffer = malloc(sizeof(char)*bufferLength);
   snprintf(buffer, bufferLength, "%s%s", myKey, myText);		//concatenate key and text to buffer

   // Send message to server
   charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
   if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
   if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

   // Get return message from server
   memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
   //testing

   //fflush(stdout);
   charsRead = recv(socketFD, buffer, sizeof(buffer), 0);
   fflush(stdout);
   printf("");

   if (charsRead < 0) error("CLIENT: ERROR reading from socket");

   fprintf(stdout, "%s\n", buffer);

   close(socketFD); // Close the socket
   return 0;
}
