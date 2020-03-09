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

#define MAX_BUF 64000;

//attempt to connect to socket, listen for encryption children, cypher text and return
bool printdebug = true;  //set false for turning in and proper formatting
bool listening = true;

//function declarations
int checkPort(int);
int listenPort(int, int, int, int, int, char*, char*);
bool encryptText(char*, char*);
bool isGoodText(char*);



int checkPort(int port) {
    //socket listening
    int initSocketFD;
    int listeningSocketFD;
    int keyLen;
    int textLen;
    int spawnPID;
    int dataSent;
    char* b1 = malloc(sizeof(char) * 64000);
    char* b2 = malloc(sizeof(char) * 64000);
    char* b3 = malloc(sizeof(char) * 64000);

    socklen_t clilen;
    struct sockaddr_in server;
    struct sockaddr_in client;
    if (printdebug)
        printf("Attempting to open port %i for encryption listening...\n", port);

    // validate port number
    if (port < 0 || port > 65535) {
        printf("Error: otp_enc_d was given an invalid port\n");
        return 2; //error break
    }

    // Set up the socket
    initSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (initSocketFD < 0) {
        fprintf(stderr, "CLIENT: ERROR opening socket");
    }

    // clear out the server address struct
    memset(&server, '\0', sizeof(server));
    // set up an address
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // bind socket to a port
    if (bind(initSocketFD, (struct sockaddr *) &server, sizeof(server)) < 0) {
        printf("Error: otp_enc_d was unable to bind socket to port %d\n", port);
        return 2; //error exit
    }

    if (printdebug) {
        fflush(stdout);
        printf("\notp_enc_d successfully bound socket on port %d\n", port);
    }

    // listen for connections
    if (listen(initSocketFD, 5) == -1) {
        printf("Error: otp_enc_d was unable to listen on port %d\n", port);
        return 2; //error exit
    }
    listening = true;
    if (printdebug) {
        printf("\notp_enc_d is now successfully listening on port %d\n", port);
    }
    clilen = sizeof(client);

    while (listening) {
        //only time listening == false is when connections fail
        listeningSocketFD = accept(initSocketFD, (struct sockaddr *)&client, &clilen); // Accept
        if (listeningSocketFD < 0) {
            printf("Error: opt_enc_d was unable to accept connection\n");
            continue;
        }
        //for every new connection, fork a child process to a new buffer
        spawnPID = fork();

        if (spawnPID < 0) {
            printf("Error: opt_enc_d was unable to fork!\n");
        } else if (spawnPID == 0) { //child process
            if (printdebug) {
                printf("otp_enc_d successfully established connection with client!\n");
            }

            //verify connection from client is infact otp_enc

            //listen for plaintext and key
            listenPort(listeningSocketFD, textLen, keyLen, dataSent, port, b1, b2);
            //encrypt text
            encryptText(b1, b2);

            //send new plaintext to client


            // clean up sockets
            close(listeningSocketFD);
            close(initSocketFD);

            return 0;
        } else { //parent process
            //no need to listen for client
            close(listeningSocketFD);
        }
    }
    return 0; //safe break
}

int listenPort(int listeningSocketFD, int textLen, int keyLen, int dataSent, int port, char* b1, char* b2) {
    //read plaintext from otp_enc
    textLen = read(listeningSocketFD, b1, 64000);
    if (textLen < 0) {
        printf("Error: otp_end_d was unable to read plaintext on port %d\n", port);
        return 2;
    }

    if (printdebug) {
        printf("otp_enc_d plaintext read: %d characters\n", textLen);
    }

    dataSent = write(listeningSocketFD, "!", 1);
    if (dataSent < 0) {
        printf("Error: otp_enc_d was unable to send acknowledgements to client\n");
        return 2;
    }

    if (printdebug) {
        printf("otp_enc_d successfully sent acknowledgements to client\n");
    }

    // zero out buffer
    memset(b2, 0, 64000);

    //read key from otp_enc
    keyLen = read(listeningSocketFD, b2, 64000);
    if (keyLen < 0) {
        printf("Error: otp_end_d could not read key on port %d\n", port);
        return 2;
    }

    if (printdebug) {
        printf("opt_enc_d: key read: %d characters. now processing\n", keyLen);
    }

    //validate legitimacy of text
    if (isGoodText(b1) || isGoodText(b2)) {
        if (printdebug) {
            printf("otp_enc_d received good text from otp_enc.\n");
        }
    }

    // compare length of plaintext to that of key
    if (keyLen < textLen) {
        printf("Error: the key otp_enc_d received is too short\n");
        return 1;
    }
    return 0;
}

bool encryptText(char* plaintext, char* key) {




    return false;
}

bool isGoodText(char* buffer) {
    int i;
    for (i = 0; i < sizeof(buffer); i++) {
        if ((int) buffer[i] > 90 || ((int) buffer[i] < 65) || (int) buffer[i] != 32) {
            //the char is not a capital letter or space, no go
            printf("Error: otp_enc_d received bad text input!\n");
            return false;
        }
    }
    return true;
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
