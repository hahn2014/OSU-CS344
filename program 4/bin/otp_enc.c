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
    textLen   = getBufferSize(plaintext);
    key       = readFile(keyFile);
    keyLen    = getBufferSize(key);

    if (printdebug)
        printf("DEBUG: plaintext - %s\nDEBUG: key - %s\n", plaintext, key);

    //make sure key is bigger than plaintext
    if (textLen > keyLen) {
        fprintf(stderr, "ERROR: the key given to otp_enc is too small!\n");
        return 1; //fail break
    }

    //validate legitimacy of text
    if (!isGoodText(plaintext)) {
        fprintf(stderr, "ERROR: otp_enc received bad plaintext input!\n");
    } else {
        if (printdebug)
            printf("DEBUG: otp_enc received a good plaintext.\n");
    }
    if (!isGoodText(key)) {
        fprintf(stderr, "ERROR: otp_enc received bad key input!\n");
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

    // make sure not otp_dec_d though

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
    memset(plaintext, 0, 64000);

    do {
        // receive ciphertext from otp_enc_d
        dataReceived = read(socketfd, plaintext, textLen);
    } while (dataReceived > 0);

    if (dataReceived < 0) {
       fprintf(stderr, "ERROR: failed to receive ciphertext from otp_enc_d\n");
       exit(2);
    }

    if (printdebug) {
        printf("DEBUG: otp_enc received cipher response\n");
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
    // send plaintext to otp_enc_d
    dataSent = write(socketfd, plaintext, textLen);
    if (dataSent < textLen) {
        fprintf(stderr, "ERROR: could not send plaintext to otp_enc_d on port %d\n", port);
        exit(2);
    }

    if (printdebug) {
        printf("DEBUG: otp_enc send an %d character plaintext\n", dataSent);
    }

    memset(ping, 0, 1);

    // get acknowledgement from server
    dataReceived = read(socketfd, ping, 1);
    if (dataReceived < 0) {
       fprintf(stderr, "ERROR: failed to receive ping from otp_enc_d\n");
       exit(2);
    }

    if (printdebug) {
        printf("DEBUG: otp_enc received ping from otp_enc_d\n");
    }

    // send key to otp_enc_d
    dataSent = write(socketfd, key, keyLen);
    if (dataSent < keyLen) {
        fprintf(stderr, "ERROR: filed to send key to otp_enc_d on port %d\n", port);
        exit(2);
    }

    if (printdebug) {
        printf("DEBUG: otp_enc sent an %d character key.\nNow reading response from server...\n", dataSent);
    }
    return 0; //safe break
}

char* readFile(char* filename) {
    FILE *fp;
    char* buff = malloc(sizeof(char) * 64000);
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
    return buff;
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
    if (argc == 4) { //otp_enc plaintext key port
        return openPort(atoi(argv[3]), argv[2], argv[1]);
    } else {
        fprintf(stderr, "ERROR: Improper arguments syntax! Syntax is as follows: ./otp_enc [plaintext] [key] [outputport]\n");
        return -1; //fail break
    }
    return 1; //safe break
}
