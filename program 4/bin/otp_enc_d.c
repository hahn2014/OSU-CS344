#include <stdio.h>      /* printf, fgets */
#include <stdlib.h>     //      --
#include <stdlib.h>     /* atoi */
#include <string.h> //string comparing and editing
#include <unistd.h>
#include <time.h>
#include <stdbool.h> //boolean values
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//attempt to connect to socket, listen for encryption children, cypher text and return
bool printdebug = true;  //set false for turning in and proper formatting
bool listening = true;

//socket listening
int socketFD;


int checkPort(int port) {
    if (printdebug)
        printf("Attempting to open port %i for encryption listening...\n", port);

    while (listening) {
        //only time listening == false is when connections fail

        // Set up the socket
    	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    	if (socketFD < 0) {
            fprintf(stderr, "CLIENT: ERROR opening socket");
        }




    }
    return 0; //safe break
}




void debug(char* message) {
    if (printdebug)
        printf("[DEBUG] - %s", message);
}

int main(int argc, char** argv) {
    if (argc == 2) { // otp_enc_d listeningport
        return checkPort(atoi(argv[1])); //get listening port from command line arguments
    } else {
        fprintf(stderr, "Error, improper arguments syntax! Syntax is as follows: ./otp_enc_d [listeningport]\n");
        return -1; //fail break
    }
    return 1;
}
