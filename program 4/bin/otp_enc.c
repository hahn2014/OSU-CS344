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

bool printdebug = true;  //set false for turning in and proper formatting


int openPort(int outputPort, char* key, char* plaintext) {
    if (printdebug)
        printf("Attempting to open port %i to encrypt %s with the key %s.\n", outputPort, key, plaintext);

    



    return 0;
}





int main(int argc, char** argv) {
    if (argc == 4) { //otp_enc plaintext key port
        return openPort(atoi(argv[3]), argv[2], argv[1]);
    } else {
        fprintf(stderr, "Error, improper arguments syntax! Syntax is as follows: ./otp_enc [plaintext] [key] [outputport]\n");
        return -1; //fail break
    }
    return 1; //safe break
}
