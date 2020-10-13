#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> //for tolower
#include <string.h> //for strcpy, strcmp, strtok_r

int main(int argc, char** argv) {
    if (argc > 1) { //too many arguments, we only want a program name
        printf("Error, you provided too many arguments! Syntax: ./MovieDirs\n");
        return -1; //error break
    }
    int resp;

    while (1) {
        printf("1.  Select file to process\n2. Exit from the program\n\n");
        printf("Enter a choice 1 or 2: ");
        //get user input
        scanf("%d", &resp);
        switch (resp) {
            case 1: //list movies from a specific year
                printf("\nWhich file you want to process?\nEnter 1 to pick the largest file\nEnter 2 to pick the smallest file\nEnter 3 to specify the name of a file\n\n");
                printf("Enter a choice from 1 to 3: ");
                scanf("%d", &resp);
                switch (resp) {
                    case 1: //largest file

                        break;
                    case 2: //smallest file

                        break;
                    case 3: //manually enter name

                        break;
                    default: //invalid option
                        printf("Error! You entered an invalid response. Please choose 1-3\n\n");
                        break;

                };
            break;
            case 2: //exit the program
                printf("Thanks for using Bryce Hahn's Movie Dirs program! Goodbye\n");
                return 0;
            break;
            default: //anything else and it should error out
                printf("You entered an incorrect choice. Try again.\n");
            break;
        };
    }
    return 0;
}
