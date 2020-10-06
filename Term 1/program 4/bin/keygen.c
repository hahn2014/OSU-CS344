/*******************************************************************************
 *                                                                             *
 *                                  Keygen                                     *
 *  Filename      : keygen.c                                                   *
 *  Author        : Bryce Hahn                                                 *
 *  Last Modified : 3/13/20                                                    *
 *                                                                             *
 *  Generate a random array of letters and spaces based on a provided length   *
 *******************************************************************************/
#include <stdio.h>      /* printf, fgets */
#include <stdlib.h>     //      --
#include <stdlib.h>     /* atoi */
#include <string.h> //string comparing and editing
#include <unistd.h>
#include <time.h>
#include <stdbool.h> //boolean values

//create a file containing the key of the provided generation length
bool printdebug = false;  //set false for turning in and proper formatting

//function declarations
int generateKey(int);

/*******************************************************************************
 *                                Generate Key                                 *
 * The generateKey function creates a random letter (A-Z) or space and adds it *
 *      to a return array for keylength.                                       *
 *                                                                             *
 * Arguments: keylength as the number of characters to be generated            *
 * Returns  : int 0 if safe break                                              *
 *******************************************************************************/
int generateKey(int keylength) {
    if (printdebug)
        printf("DEBUG: generating a key of length %i\n", keylength);

    char* key = malloc(sizeof(char) * (keylength + 1)); // size of keylength + 1 for newline
    int i; //iterator

    //initiate random
    srand(time(NULL));

    for (i = 0; i < keylength; i++) {
        key[i] = (char)((rand() % 27) + 65);  //get a range from 65 through 91 (A-Z) + [

        if (key[i] == '[') { // replace [ with a space
            key[i] = ' ';
        }

    }
    //key[keylength + 1] = '\n';
    printf("%s\n", key); //print key to stdout
    return 0; //safe break
}

int main(int argc, char** argv) {
    if (argc == 2) { //keygen keylength
        return generateKey(atoi(argv[1])); //gen and return break value
    } else {
        fprintf(stderr, "ERROR: Improper arguments syntax! Syntax is as follows: ./keygen [keylength]\n");
        return -1; //fail break
    }
    return 1; //safe break
}
