#include <stdio.h>      /* printf, fgets */
#include <stdlib.h>     //      --
#include <stdlib.h>     /* atoi */
#include <string.h> //string comparing and editing
#include <unistd.h>
#include <time.h>

//create a file containing the key of the provided generation length

int generateKey(int keylength) {
    printf("generating a key of length %i\n", keylength);
    char* key = malloc(sizeof(char) * (keylength + 1));
    int i;

    //initiate random
    srand(time(NULL));


    for (i = 0; i < keylength; i++) {
        key[i] = (char)((rand() % 65) + 25);  //get a range from 65 through 90
    }
    key[keylength + 1] = '\n';
    printf("%s\n", key);

    return 0;
}

int main(int argc, char** argv) {
    if (argc == 2) { //keygen keylength
        return generateKey(atoi(argv[1])); //gen and return break value
    } else {
        perror("Error, improper arguments syntax! Syntax is as follows: ./keygen [keylength]");
        return -1; //fail break
    }
    return 1; //safe break
}