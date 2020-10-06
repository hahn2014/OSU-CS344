/*******************************************************************************
 *                                                                             *
 *                           OTP Decryption Deamon                             *
 *  Filename      : otp_dec_d.c                                                *
 *  Author        : Bryce Hahn                                                 *
 *  Last Modified : 3/13/20                                                    *
 *                                                                             *
 *  Listens for otp_dec child process to decrypt a ciphertext message, and     *
 *      returns the converted plaintext.                                       *
 *******************************************************************************/
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

int max_size = 100000;

//attempt to connect to socket, listen for encryption children, cypher text and return
bool printdebug = false;  //set false for turning in and proper formatting
bool listening = true;

//function declarations
int checkPort(int);
int listenPort(int, int, int, char*, char*);
char* decryptText(char*, char*, int);

/*******************************************************************************
 *                                Check Port                                   *
 * The checkPort function initiates a server, and listens to otp_dec child     *
 *      processes to read from. Once found, it reads from client then decrypts *
 *      provided ciphertext and returns the resulting plaintext.               *
 *                                                                             *
 * Arguments: port as the connection port to listen for files                  *
 * Returns  : int 0 if safe break, 2 fail break                                *
 *******************************************************************************/
int checkPort(int port) {
    //socket listening
    int initSocketFD;
    int listeningSocketFD;
    int spawnPID;
    int dataSent;
    int textLen;
    char* ciphertext = malloc(sizeof(char) * max_size);
    char* key        = malloc(sizeof(char) * max_size);
    char* result;

    socklen_t clilen;
    struct sockaddr_in server;
    struct sockaddr_in client;
    if (printdebug)
        printf("DEBUG: Attempting to open port %i for deciphering listening...\n", port);

    // validate port number
    if (port < 0 || port > 65535) {
        fprintf(stderr, "ERROR: otp_dec_d was given an invalid port\n");
        return 2; //error break
    }

    // Set up the socket
    initSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (initSocketFD < 0) {
        fprintf(stderr, "ERROR: failed to open socket\n");
    }

    // clear out the server address struct
    memset(&server, '\0', sizeof(server));
    // set up an address
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // bind socket to a port
    if (bind(initSocketFD, (struct sockaddr *) &server, sizeof(server)) < 0) {
        fprintf(stderr, "ERROR: otp_dec_d was unable to bind to socket on port %d\n", port);
        return 2; //error exit
    }

    if (printdebug) {
        fflush(stdout);
        printf("DEBUG: otp_dec_d successfully bound to socket on port %d\n", port);
    }

    // listen for connections
    if (listen(initSocketFD, 5) == -1) {
        fprintf(stderr, "ERROR: otp_dec_d was unable to listen on port %d\n", port);
        return 2; //error exit
    }
    listening = true;
    if (printdebug) {
        printf("DEBUG: otp_dec_d is now successfully listening on port %d\n", port);
    }
    clilen = sizeof(client);

    while (listening) {
        //only time listening == false is when connections fail
        listeningSocketFD = accept(initSocketFD, (struct sockaddr *)&client, &clilen); // Accept
        if (listeningSocketFD < 0) {
            fprintf(stderr, "ERROR: opt_enc_d was unable to accept connection\n");
            continue;
        }
        //for every new connection, fork a child process to a new buffer
        spawnPID = fork();

        if (spawnPID < 0) {
            fprintf(stderr, "ERROR: opt_enc_d failed to fork!\n");
        } else if (spawnPID == 0) { //child process
            if (printdebug) {
                printf("DEBUG: otp_dec_d successfully established connection with client!\n");
            }

            //verify connection from client is infact otp_dec

            //listen for plaintext and key
            textLen = listenPort(listeningSocketFD, dataSent, port, ciphertext, key);
            //encrypt text
            result = decryptText(ciphertext, key, textLen);
            //send new plaintext to client
            if (printdebug) {
                printf("DEBUG: otp_dec_d successfully decrypted cipher, returning plaintext to client.\n");
                printf("DEBUG: otp_dec_d plaintext ---> %s\n", result);
            }

            // send plaintext to otp_dec
            dataSent = write(listeningSocketFD, result, textLen);
            if (dataSent < textLen) {
                fprintf(stderr, "ERROR: otp_dec_d failed to write to socket\n");
                return 2;
            }

            if (printdebug) {
                printf("DEBUG: opt_enc_d sent plaintext response successfully.\n");
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

    free(ciphertext);
    free(key);
    free(result);
    return 0; //safe break
}


/*******************************************************************************
 *                                 Listen Port                                 *
 * The listPort function first verifies that the connection made was to a      *
 *      otp_dec process, then listens for plaintext and key from clients to    *
 *      return for encryption.                                                 *
 *                                                                             *
 * Arguments: listeningSocketFD as socket connection, dataSent as a means to   *
 *              get text size, port as the sending port (mostly debug),        *
 *              ciphertext as the char* of the original ciphertext, key as the *
 *              char* of the key for decryption                                *
 * Returns  : int 0 if safe break                                              *
 *******************************************************************************/
int listenPort(int listeningSocketFD, int dataSent, int port, char* ciphertext, char* key) {
    int keyLen;
    int textLen;
    int dataReceived;
    // make sure only otp_dec is connected to us
    char* veriPing = malloc(sizeof(char));
    // receive ping from otp_dec
    dataReceived = read(listeningSocketFD, veriPing, 1);

    if (dataReceived < 0) {
       fprintf(stderr, "ERROR: failed to receive verification response from otp_dec\n");
       exit(2);
    }

    if (printdebug) {
        printf("DEBUG: otp_dec_d received verification response\n");
        printf("DEBUG: otp_dec_d verification response ---> %c\n", veriPing[0]);
    }

    if (veriPing[0] == 'd') { //verification received from otp_enc
        if (printdebug)
            printf("DEBUG: otp_dec_d received verification response from otp_dec, continuing...\n");
    } else { //connected to something other than otp_enc_d
        fprintf(stderr, "ERROR: otp_dec_d is not connected to a otp_dec process\n");
        exit (2);
    }

    free(veriPing);

    dataSent = write(listeningSocketFD, "d", 1); //tell otp_enc we are otp_enc_d
    if (dataSent < 0) {
        fprintf(stderr, "ERROR: could not verify connection on port %d\n", port);
        return 2;
    }


    //read ciphertext from otp_dec
    textLen = read(listeningSocketFD, ciphertext, max_size);
    if (textLen < 0) {
        fprintf(stderr, "ERROR: otp_end_d was unable to read cipher on port %d\n", port);
        return 2;
    }

    if (printdebug) {
        printf("DEBUG: otp_dec_d successfully read an %d character cipher.\n", textLen);
        printf("DEBUG: otp_dec_d cipher ---> %s\n", ciphertext);
    }

    dataSent = write(listeningSocketFD, "!", 1);
    if (dataSent < 0) {
        fprintf(stderr, "ERROR: otp_dec_d was unable to ping client\n");
        return 2;
    }

    if (printdebug) {
        printf("DEBUG: otp_dec_d successfully pinged client\n");
    }

    // zero out buffer
    memset(key, 0, max_size);

    //read key from otp_dec
    keyLen = read(listeningSocketFD, key, max_size);
    if (keyLen < 0) {
        fprintf(stderr, "ERROR: otp_end_d could not read key on port %d\n", port);
        return 2;
    }

    if (printdebug) {
        printf("DEBUG: opt_enc_d successfully read an %d character key.\n", keyLen);
        printf("DEBUG: otp_dec_d key ---> %s\n", key);
    }
    return textLen;
}

/*******************************************************************************
 *                                Decrypt Text                                 *
 * The decryptText function decrypts a provided ciphertext with the provided   *
 *      key and returns the calculated plaintext.                              *
 *                                                                             *
 * Arguments: ciphertext as the char* to the ciphertext, key as the char* to   *
 *              then key, textLen as the length of the original plaintext.     *
 * Returns  : char* to the new resultant plaintext                            *
 *******************************************************************************/
char* decryptText(char* ciphertext, char* key, int textLen) {
    int i;
    int ptChar;
    int kChar;
    int decrypted;
    char* result = malloc(sizeof(char) * textLen);

    if (printdebug)
        printf("DEBUG: otp_dec_d textLen -> %i\n", textLen);

    for (i = 0; i < textLen; i++) {
        // change spaces to open bracket (65-91)
        if (ciphertext[i] == ' ') {
            ciphertext[i] = '[';
        }
        if (key[i] == ' ') {
            key[i] = '[';
        }

        ptChar = (int)ciphertext[i]; // convert char to ascii integer value
        ptChar = ptChar - 65;       // subtract 65 to range from 0-26
        kChar  = (int)key[i];       // convert char to ascii integer value
        kChar  = kChar  - 65;       // subtract 65 to range from 0-26

        // remove key from message using modular addition
        decrypted = (ptChar - kChar) % 27;

        if (decrypted < 0) { //make sure we stay in the 0-26 range
            decrypted = decrypted + 27;
        }
        decrypted = decrypted + 65;
        // add 64 back to that range is 64 - 90
        // type conversion back to char
        result[i] = (char)decrypted + 0;

        //replace bracket with spaces
        if (result[i] == '[') {
            result[i] = ' ';
        }
    }

    //debuging plaintext value
    if (printdebug) {
        printf("DEBUG: otp_dec_d plaintext text: ");
        for (i = 0; i < textLen; i++) {
            printf("%c", result[i]);
        }
        printf("\n");
    }
    return result;
}


int main(int argc, char** argv) {
    if (argc == 2) { // otp_dec_d listeningport
        return checkPort(atoi(argv[1])); //get listening port from command line arguments
    } else {
        fprintf(stderr, "ERROR: Improper arguments syntax! Syntax is as follows: ./otp_dec_d [listeningport] &\n");
        return -1; //fail break
    }
    return 1;
}
