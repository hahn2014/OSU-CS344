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

//function declarations
int checkPort(int);
int listenPort(int, int, int, int, int, char*, char*);
void encryptText(char*, char*, char*);
int getBufferSize(char*);


int checkPort(int port) {
    //socket listening
    int initSocketFD;
    int listeningSocketFD;
    int spawnPID;
    int dataSent;
    int textLen;
    int keyLen;
    char* plaintext = malloc(sizeof(char) * 64000);
    char* key       = malloc(sizeof(char) * 64000);
    char* result    = malloc(sizeof(char) * 64000);

    socklen_t clilen;
    struct sockaddr_in server;
    struct sockaddr_in client;
    if (printdebug)
        printf("DEBUG: Attempting to open port %i for encryption listening...\n", port);

    // validate port number
    if (port < 0 || port > 65535) {
        printf("ERROR: otp_enc_d was given an invalid port\n");
        return 2; //error break
    }

    // Set up the socket
    initSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (initSocketFD < 0) {
        fprintf(stderr, "ERROR: failed to open socket");
    }

    // clear out the server address struct
    memset(&server, '\0', sizeof(server));
    // set up an address
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // bind socket to a port
    if (bind(initSocketFD, (struct sockaddr *) &server, sizeof(server)) < 0) {
        printf("ERROR: otp_enc_d was unable to bind to socket on port %d\n", port);
        return 2; //error exit
    }

    if (printdebug) {
        fflush(stdout);
        printf("DEBUG: otp_enc_d successfully bound to socket on port %d\n", port);
    }

    // listen for connections
    if (listen(initSocketFD, 5) == -1) {
        printf("ERROR: otp_enc_d was unable to listen on port %d\n", port);
        return 2; //error exit
    }
    listening = true;
    if (printdebug) {
        printf("DEBUG: otp_enc_d is now successfully listening on port %d\n", port);
    }
    clilen = sizeof(client);

    while (listening) {
        //only time listening == false is when connections fail
        listeningSocketFD = accept(initSocketFD, (struct sockaddr *)&client, &clilen); // Accept
        if (listeningSocketFD < 0) {
            printf("ERROR: opt_enc_d was unable to accept connection\n");
            continue;
        }
        //for every new connection, fork a child process to a new buffer
        spawnPID = fork();

        if (spawnPID < 0) {
            printf("ERROR: opt_enc_d failed to fork!\n");
        } else if (spawnPID == 0) { //child process
            if (printdebug) {
                printf("DEBUG: otp_enc_d successfully established connection with client!\n");
            }

            //verify connection from client is infact otp_enc

            //listen for plaintext and key
            listenPort(listeningSocketFD, dataSent, port, textLen, keyLen, plaintext, key);
            //encrypt text
            encryptText(plaintext, key, result);
            //send new plaintext to client
            if (printdebug) {
                printf("DEBUG: otp_enc_d successfully encrypted plaintext, returning cipher to client.\n");
            }

            // send ciphertext to otp_enc
            dataSent = write(listeningSocketFD, result, sizeof(result));
            if (dataSent < sizeof(result)) {
                printf("ERROR: otp_enc_d failed to write to socket\n");
                return 2;
            }

            if (printdebug) {
                printf("DEBUG: opt_enc_d sent cipher esponse successfully.\n");
            }

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

int listenPort(int listeningSocketFD, int dataSent, int port, int textLen, int keyLen, char* plaintext, char* key) {
    //read plaintext from otp_enc
    textLen = read(listeningSocketFD, plaintext, 64000);
    if (textLen < 0) {
        printf("ERROR: otp_end_d was unable to read plaintext on port %d\n", port);
        return 2;
    }

    if (printdebug) {
        printf("DEBUG: otp_enc_d successfully read an %d character plaintext.\n", textLen);
    }

    dataSent = write(listeningSocketFD, "!", 1);
    if (dataSent < 0) {
        printf("ERROR: otp_enc_d was unable to ping client\n");
        return 2;
    }

    if (printdebug) {
        printf("DEBUG: otp_enc_d successfully pinged client\n");
    }

    // zero out buffer
    memset(key, 0, 64000);

    //read key from otp_enc
    keyLen = read(listeningSocketFD, key, 64000);
    if (keyLen < 0) {
        printf("ERROR: otp_end_d could not read key on port %d\n", port);
        return 2;
    }

    if (printdebug) {
        printf("DEBUG: opt_enc_d successfully read an %d character key.\nNow encrypting...\n", keyLen);
    }
    return 0;
}

void encryptText(char* plaintext, char* key, char* result) {
    int i;
    for (i = 0; i < sizeof(plaintext); i++) {
        // change spaces to open bracket (65-91)
        if (plaintext[i] == ' ') {
            plaintext[i] = '[';
        }
        if (key[i] == ' ') {
            key[i] = '[';
        }


        int ptChar = (int)plaintext[i];
        int kChar  = (int)key[i];

        // subtract 65 so that range is 0 - 26 (27 characters)
        ptChar = ptChar - 65;
        kChar  = kChar  - 65;

        // combine key and message using modular addition
        int encrypted = (ptChar + kChar) % 27;

        // add 64 back to that range is 64 - 90
        encrypted = encrypted + 65;

        // type conversion back to char
        result[i] = (char)encrypted + 0;

        // after encryption, change bracket to spaces
        if (result[i] == '[') {
            result[i] = ' ';
        }
    }
}

int getBufferSize(char* buff) {
    int i;
    for (i = 0; i < sizeof(buff); i++) {
        if (buff[i] == '\0') {
            break;
        }
    }
    return i;
}

int main(int argc, char** argv) {
    if (argc == 2) { // otp_enc_d listeningport
        return checkPort(atoi(argv[1])); //get listening port from command line arguments
    } else {
        fprintf(stderr, "ERROR: Improper arguments syntax! Syntax is as follows: ./otp_enc_d [listeningport] &\n");
        return -1; //fail break
    }
    return 1;
}
