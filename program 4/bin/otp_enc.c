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
int getBufferSize(char*);

int openPort(int port, char* keyFile, char* plaintextFile) {
    int i;
    int socketfd;
    int dataReceived;
    int dataSent;
    int textLen;
    int keyLen;
    char* plaintext;
    char* key;
    char* ping;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (printdebug)
        printf("DEBUG: Attempting to open port %i to encrypt %s with the key %s.\n", port, plaintextFile, keyFile);

    plaintext = readFile(plaintextFile);
    textLen   = charSize;
    key       = readFile(keyFile);
    keyLen    = charSize;
    ping      = malloc(sizeof(char));

    if (printdebug)
        printf("DEBUG: plaintext [%i] - %s\nDEBUG: key [%i] - %s\n", textLen, plaintext, keyLen, key);

    //make sure key is bigger than plaintext
    if (textLen > keyLen) {
        fprintf(stderr, "ERROR: the key given to otp_enc is too small!\n");
        return 1; //fail break
    }

    //validate legitimacy of text
    if (!isGoodText(plaintext)) {
        fprintf(stderr, "ERROR: otp_enc received bad plaintext input! -> %s\n", plaintextFile);
    } else {
        if (printdebug)
            printf("DEBUG: otp_enc received a good plaintext.\n");
    }
    if (!isGoodText(key)) {
        fprintf(stderr, "ERROR: otp_enc received bad key input! -> %s\n", keyFile);
    } else {
        if (printdebug)
            printf("DEBUG: otp_enc received a good key.\n");
    }

    //attempt to connect to server otp_enc_d
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        fprintf(stderr, "ERROR: could not contact otp_enc_d on port %d\n", port);
        exit(2);
    }

    // zero out the IP address memory space
    memset(&serv_addr, '\0', sizeof(serv_addr));

    server = gethostbyname("localhost"); //server will always be hosted on localhost
    if (server == NULL) {
        fprintf(stderr, "ERROR: could not connect to otp_enc_d\n");
        exit(2);
    }

    serv_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

    serv_addr.sin_port = htons(port);

    // connect to otp_enc_d
    if (connect(socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "ERROR: could not connect to otp_enc_d on port %d\n", port);
        exit(2);
    }

    if (printdebug) {
        printf("DEBUG: otp_enc successfully connected to server\n");
    }

    //send files to server for encryption
    if (sendFiles(dataSent, dataReceived, socketfd, port, textLen, keyLen, plaintext, key, ping) != 0) {
        fprintf(stderr, "ERROR: failed to send files to server\n");
        return 2;
    } else {
        if (printdebug)
            printf("DEBUG: otp_enc successfully sent files to otp_enc_d server\n");
    }

    // zero out response buffer
    memset(plaintext, 0, textLen);

    // receive ciphertext from otp_enc_d
    dataReceived = read(socketfd, plaintext, textLen);

    if (dataReceived < 0) {
       fprintf(stderr, "ERROR: failed to receive ciphertext from otp_enc_d\n");
       exit(2);
    }

    if (printdebug) {
        printf("DEBUG: otp_enc received cipher response\n");
        printf("DEBUG: otp_enc cipher ---> %s\n", plaintext);
    }

    // output ciphertext to stdout
    for (i = 0; i < textLen; i++) {
        printf("%c", plaintext[i]);
    }

    // add newline to ciphertext ouput
    printf("\n");

    // cleanup
    close(socketfd);
    free(plaintext);
    free(key);
    return 0;
}

int sendFiles(int dataSent, int dataReceived, int socketfd, int port, int textLen, int keyLen, char* plaintext, char* key, char* ping) {
    // make sure were not attempting to connect to otp_dec_d
    char* veriPing = malloc(sizeof(char));
    dataSent = write(socketfd, "e", 1); //tell otp_enc_d we are otp_enc
    if (dataSent < 1) {
        fprintf(stderr, "ERROR: could not verify connection on port %d\n", port);
        return 2;
    }

    // receive ping from otp_enc_d
    dataReceived = read(socketfd, veriPing, 1);

    if (dataReceived < 0) {
       fprintf(stderr, "ERROR: failed to receive verification response from otp_enc_d\n");
       exit(2);
    }

    if (printdebug) {
        printf("DEBUG: otp_enc received verification response\n");
        printf("DEBUG: otp_enc verification response ---> %c\n", veriPing[0]);
    }

    if (veriPing[0] == 'e') { //verification received from otp_enc_d
        if (printdebug)
            printf("DEBUG: otp_enc received verification response from otp_enc_d, continuing...\n");
    } else { //connected to something other than otp_enc_d
        fprintf(stderr, "ERROR: otp_enc is not connected to a otp_enc_d process\n");
        exit (2);
    }

    free(veriPing);

    // send plaintext to otp_enc_d
    dataSent = write(socketfd, plaintext, textLen);
    if (dataSent < textLen) {
        fprintf(stderr, "ERROR: could not send plaintext to otp_enc_d on port %d\n", port);
        return 2;
    }

    if (printdebug) {
        printf("DEBUG: otp_enc sent an %d character plaintext\n", dataSent);
        printf("DEBUG: otp_enc plaintext ---> %s\n", plaintext);
    }

    memset(ping, 0, 1);

    // get acknowledgement from server
    dataReceived = read(socketfd, ping, 1);
    if (dataReceived < 0) {
       fprintf(stderr, "ERROR: failed to receive ping from otp_enc_d\n");
       return 2;
    }

    if (printdebug) {
        printf("DEBUG: otp_enc received ping from otp_enc_d\n");
    }

    // send key to otp_enc_d
    dataSent = write(socketfd, key, keyLen);
    if (dataSent < keyLen) {
        fprintf(stderr, "ERROR: failed to send key to otp_enc_d on port %d\n", port);
        return 2;
    }

    if (printdebug) {
        printf("DEBUG: otp_enc sent an %d character key.\n", dataSent);
        printf("DEBUG: otp_enc key ---> %s\n", key);
        printf("Now reading response from server...\n");
    }
    return 0; //safe break
}

char* readFile(char* filename) {
    FILE *fp;
    char* buff = malloc(sizeof(char) * max_size);
    char c;
    int i;

    if ((fp = fopen(filename,"r")) == NULL) {
       fprintf(stderr, "ERROR: otp_enc failed to open %s\n", filename);
       exit(2);
    }
    i = 0;
    while((c = fgetc(fp)) != EOF) { //read char
        //printf("%c", c);
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
    if (argc == 4) { //otp_enc plaintext key sendPort
        return openPort(atoi(argv[3]), argv[2], argv[1]);
    } else {
        fprintf(stderr, "ERROR: Improper arguments syntax! Syntax is as follows: ./otp_enc [plaintext] [key] [sendPort]\n");
        return -1; //fail break
    }
    return 1; //safe break
}
