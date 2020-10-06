/*******************************************************************************
 *                                                                             *
 *                                 OTP Decryption                              *
 *  Filename      : otp_dec.c                                                  *
 *  Author        : Bryce Hahn                                                 *
 *  Last Modified : 3/13/20                                                    *
 *                                                                             *
 *  Attempts to connect to otp_dec_d, vefifying connection is valid, then      *
 *      sends ciphertext and key to otp_dec_d to be decrypted. Listening for a *
 *      response plaintext from otp_dec_d.                                     *
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

bool printdebug = false;  //set false for turning in and proper formatting
int charSize = 0;
int max_size = 100000;

int openPort(int, char*, char*);
char* readFile(char*);
bool isGoodText(char*);
int sendFiles(int, int, int, int, int, int, char*, char*, char*);

/*******************************************************************************
 *                                 Open Port                                   *
 * The openPort function initiates a client to connect to a decryption deamon  *
 *      and verifies that the connected process is infact a otp_dec_d process  *
 *      then waits for a responce from otp_dec_d for the resulting plaintext.  *
 *                                                                             *
 * Arguments: port as the connection port to send files, keyFile as the file   *
 *              name for the key text, ciphertextFile as the filename for the  *
 *              ciphertext file.                                               *
 * Returns  : int 0 if safe break, 2 fail break                                *
 *******************************************************************************/
int openPort(int port, char* keyFile, char* ciphertextFile) {
    int i;
    int socketfd;
    int dataReceived;
    int dataSent;
    int textLen;
    int keyLen;
    char* ciphertext;
    char* key;
    char* ping;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (printdebug)
        printf("DEBUG: Attempting to open port %i to decrypt %s with the key %s.\n", port, ciphertextFile, keyFile);

    ciphertext = readFile(ciphertextFile);
    textLen    = charSize;
    key        = readFile(keyFile);
    keyLen     = charSize;
    ping       = malloc(sizeof(char));

    if (printdebug)
        printf("DEBUG: ciphertext [%i] - %s\nDEBUG: key [%i] - %s\n", textLen, ciphertext, keyLen, key);

    //make sure key is bigger than ciphertext
    if (textLen > keyLen) {
        fprintf(stderr, "ERROR: the key given to otp_dec is too small!\n");
        return 1; //fail break
    }

    //validate legitimacy of text
    if (!isGoodText(ciphertext)) {
        fprintf(stderr, "ERROR: otp_dec received bad ciphertext input! -> %s\n", ciphertextFile);
    } else {
        if (printdebug)
            printf("DEBUG: otp_dec received a good ciphertext.\n");
    }
    if (!isGoodText(key)) {
        fprintf(stderr, "ERROR: otp_dec received bad key input! -> %s\n", keyFile);
    } else {
        if (printdebug)
            printf("DEBUG: otp_dec received a good key.\n");
    }

    //attempt to connect to server otp_dec_d
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        fprintf(stderr, "ERROR: could not contact otp_dec_d on port %d\n", port);
        exit(2);
    }

    // zero out the IP address memory space
    memset(&serv_addr, '\0', sizeof(serv_addr));

    server = gethostbyname("localhost"); //server will always be hosted on localhost
    if (server == NULL) {
        fprintf(stderr, "ERROR: could not connect to otp_dec_d\n");
        exit(2);
    }

    serv_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

    serv_addr.sin_port = htons(port);

    // connect to otp_dec_d
    if (connect(socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "ERROR: could not connect to otp_dec_d on port %d\n", port);
        exit(2);
    }

    // make sure were not attempting to connect to otp_dec_d

    if (printdebug) {
        printf("DEBUG: otp_dec successfully connected to server\n");
    }

    //send files to server for encryption
    if (sendFiles(dataSent, dataReceived, socketfd, port, textLen, keyLen, ciphertext, key, ping) != 0) {
        fprintf(stderr, "ERROR: failed to send files to server\n");
        return 2;
    } else {
        if (printdebug)
            printf("DEBUG: otp_dec successfully sent files to otp_dec_d server\n");
    }

    // zero out response buffer
    memset(ciphertext, 0, textLen);

    //do {
        // receive plaintext from otp_dec_d
        dataReceived = read(socketfd, ciphertext, textLen);
    //} while (dataReceived > 0);

    if (dataReceived < 0) {
       fprintf(stderr, "ERROR: failed to receive plaintext from otp_dec_d\n");
       exit(2);
    }

    if (printdebug) {
        printf("DEBUG: otp_dec received plaintext response\n");
        printf("DEBUG: otp_dec plaintext ---> %s\n", ciphertext);
    }

    // output plaintext to stdout
    for (i = 0; i < textLen; i++) {
        printf("%c", ciphertext[i]);
    }

    // add newline to plaintext ouput
    printf("\n");

    // cleanup
    close(socketfd);
    free(ciphertext);
    free(key);
    return 0;
}

/*******************************************************************************
 *                                 Send Files                                  *
 * The sendFiles function first verifies the connected process is a otp_dec_d  *
 *      process, then sends the ciphertext and key to be decrypted.            *
 *                                                                             *
 * Arguments: dataSent as the number of bytes sent to server, dataReceived as  *
 *              the bytes received from server, port as the send/receive port, *
 *              textLen as the size of the sent ciphertext, keyLen as the size *
 *              of the sent key, ciphertext as the char* to the ciphertext,    *
 *              key as the char* to the key, ping as the char* to the ping mem *
 * Returns  : int 0 if safe break                                              *
 *******************************************************************************/
int sendFiles(int dataSent, int dataReceived, int socketfd, int port, int textLen, int keyLen, char* ciphertext, char* key, char* ping) {
    // make sure were not attempting to connect to otp_enc_d
    char* veriPing = malloc(sizeof(char));
    dataSent = write(socketfd, "d", 1); //tell otp_dec_d we are otp_dec
    if (dataSent < 1) {
        fprintf(stderr, "ERROR: could not verify connection on port %d\n", port);
        return 2;
    }

    // receive ping from otp_dec_d
    dataReceived = read(socketfd, veriPing, 1);

    if (dataReceived < 0) {
       fprintf(stderr, "ERROR: failed to receive verification response from otp_dec_d\n");
       exit(2);
    }

    if (printdebug) {
        printf("DEBUG: otp_dec received verification response\n");
        printf("DEBUG: otp_dec verification response ---> %c\n", veriPing[0]);
    }

    if (veriPing[0] == 'd') { //verification received from otp_dec_d
        if (printdebug)
            printf("DEBUG: otp_dec received verification response from otp_dec_d, continuing...\n");
    } else { //connected to something other than otp_enc_d
        fprintf(stderr, "ERROR: otp_dec is not connected to a otp_dec_d process\n");
        exit (2);
    }

    free(veriPing);

    // send ciphertext to otp_dec_d
    dataSent = write(socketfd, ciphertext, textLen);
    if (dataSent < textLen) {
        fprintf(stderr, "ERROR: could not send ciphertext to otp_dec_d on port %d\n", port);
        return 2;
    }

    if (printdebug) {
        printf("DEBUG: otp_dec sent an %d character ciphertext\n", dataSent);
        printf("DEBUG: otp_dec ciphertext ---> %s\n", ciphertext);
    }

    memset(ping, 0, 1);

    // get acknowledgement from server
    dataReceived = read(socketfd, ping, 1);
    if (dataReceived < 0) {
       fprintf(stderr, "ERROR: failed to receive ping from otp_dec_d\n");
       return 2;
    }

    if (printdebug) {
        printf("DEBUG: otp_dec received ping from otp_dec_d\n");
    }

    // send key to otp_dec_d
    dataSent = write(socketfd, key, keyLen);
    if (dataSent < keyLen) {
        fprintf(stderr, "ERROR: failed to send key to otp_dec_d on port %d\n", port);
        return 2;
    }

    if (printdebug) {
        printf("DEBUG: otp_dec sent an %d character key.\n", dataSent);
        printf("DEBUG: otp_dec key ---> %s\n", key);
        printf("Now reading response from server...\n");
    }
    return 0; //safe break
}

/*******************************************************************************
 *                                  Read File                                  *
 * The readFile function takes a given filename and reads all the content of   *
 *      the file into a char* and returns the read file.                       *
 *                                                                             *
 * Arguments: filename as the string to the file to read                       *
 * Returns  : resulting opened file contents                                   *
 *******************************************************************************/
char* readFile(char* filename) {
    FILE *fp;
    char* buff = malloc(sizeof(char) * max_size);
    char c;
    int i;

    if ((fp = fopen(filename,"r")) == NULL) {
       fprintf(stderr, "ERROR: otp_dec failed to open %s\n", filename);
       exit(2);
    }
    i = 0;
    while((c = fgetc(fp)) != EOF) { //read char
        if (c == '\n') //ignore newline char
            break;
        buff[i] = c; //save to buffer
        i++;
    }
    fclose(fp);
    charSize = i;
    char* res = malloc(sizeof(char) * i); //dynamically allocated to size of input
    strncpy(res, buff, i); //copy contents to new array
    free(buff); //free temporary buffer size
    return res;
}

/*******************************************************************************
 *                                Is Good Text                                 *
 * The isGoodText function takes a given char* and verifies that the contents  *
 *      of the string are within (A-Z) and " "                                 *
 *                                                                             *
 * Arguments: text as the text to verify                                       *
 * Returns  : True if valid (A-Z) or Space                                     *
 *******************************************************************************/
bool isGoodText(char* text) {
    int i;
    for (i = 0; i < sizeof(text); i++) {
        if ( ( (int)text[i] > 90 || (int)text[i] < 65 ) && (int)text[i] != 32) {
            //the char is not a capital letter or space, no go
            return false;
        }
    }
    return true;
}


int main(int argc, char** argv) {
    if (argc == 4) { //otp_dec plaintext key sendPort
        return openPort(atoi(argv[3]), argv[2], argv[1]);
    } else {
        fprintf(stderr, "ERROR: Improper arguments syntax! Syntax is as follows: ./otp_dec [plaintext] [key] [sendPort]\n");
        return -1; //fail break
    }
    return 1; //safe break
}
