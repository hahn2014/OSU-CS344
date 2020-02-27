#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h> //booleans
#include <stdio.h>  //  for printing
#include <stdlib.h> //      --
#include <signal.h> //for signal handling
#include <string.h> //string comparing and editing
#include <fcntl.h> //closing files

struct childProcs {
    pid_t pid;
    bool background;
    int childExitStatus;
};

bool foregroundOnly = false;

void getInput(char**, char*, char*, pid_t, bool);
int interpretArguments(char**, char*, char*, struct sigaction, struct childProcs*, bool, int);
int runCommand(char**, char*, char*, struct sigaction, struct childProcs*, bool);
int addChild(pid_t, bool, struct childProcs*);
int getChildIndex(pid_t, struct childProcs*);
void printExitStatus(int);
int smallsh();


/*******************************************************************************
 *                              Get User Input                                 *
 * The getInput function will request the user for input from stdin as a line  *
 *      of commands that will be then interpreted later. Once the user input   *
 *      has been tokenized we can transer desired tokens into the argArray to  *
 *      to sent back to smallsh.                                               *
 *                                                                             *
 * Arguments: argArray to store user commands, inputf to store a provided input*
 *              file, outputf to store a provided output file, shellPID as a   *
 *              provided parent pid, bg to store if the user wishes to run the *
 *              command in the background.                                     *
 * Returns  : Nohting.                                                         *
 *******************************************************************************/
void getInput(char** argArray, char* inputf, char* outputf, pid_t shellPID, bool bg) {
    int i,j; //for iterative purposes
    char* token; //tokenize the input to allow for variable expansion
    char* next;  //the next token in the sorted list
    const char space[2] = " "; //remove spaces to seperate variables
    char* userInput = malloc(sizeof(char) * 2048);


	fflush(stdout);
    fgets(userInput, 2048, stdin); //get the user input from stdin (up to 2048 chars)

    for (i = 0; i < sizeof(userInput); i++) {
        if (userInput[i] == '\n') {
            userInput[i] = '\0'; // replace the newline char at the end
            break;                  // with a null terminator
        }
    }

    token = strtok(userInput, space); //tokenize userinput
    i = 0;
    while (token != NULL) { //loop through all tokens
        next = strtok(NULL, space); //get next token
        if (next == NULL) { //we're at the end
            if (strcmp(token, "&") == 0 && foregroundOnly == false) {
                bg = true; //the command is a background process
            }
        }

        // if the last word is &, the following process should be
		if (strcmp(token, "<") == 0) { // if the special character < is found, expect an input file
			strcpy(inputf, next); //copy the next token as the input file
		} else if (strcmp(token, ">") == 0) { // if the special character > is found, expect an output file
			strcpy(outputf, next); //copy the next token as the output file
		} else { //if we reach here, the token is part of the command
            //fill the argument array with the token at current iteration
            argArray[i] = strdup(token);
            //check to see if we need to replace $$ with pid
            if (strcmp(argArray[i], "$$") == 0) {
                char* mypid = malloc(6);   // ex. 34567
                sprintf(mypid, "%d", shellPID); //convert pid_t to string
                strcpy(argArray[i], mypid);
            } else if (strcmp(argArray[i], "&") == 0) { //remove background symbol
                argArray[i] = "";
            }
        }
        //next token to interpret
        token = next;
        i++;
    }
    free(userInput);
}

/*******************************************************************************
 *                                Run The Command                              *
 *  The runCommand function will take the provided array of arguments and      *
 *      attempt to run the command. We split off a child process if the command*
 *      is valid command, and if the current pid is parent, we maintain the    *
 *      given processses, and terminate when necessary.                        *
 *                                                                             *
 * Arguments: argVars the provided array of command and its arguments, inputf  *
 *              to store a provided input file, outputf to store a provided    *
 *              output file, sigint as a way to provide child processes ^C     *
 *              support, childProcesses to keep track of the children, bg if   *
 *              the command should be made a background process.               *
 * Returns  : return 0 for an alive break to continue prompting user, 1 for a  *
 *              safe break to end the process, or 2 for error break.           *
 *******************************************************************************/
int runCommand(char** argVars, char* inputf, char* outputf, struct sigaction sigint, struct childProcs* childProcesses, bool bg) {
    int result, childIndex;
    pid_t spawnpid = -5;

    if (strcmp(argVars[0], "") == 0) {
        return 0; //alive break
    }

	spawnpid = fork(); //fork a new child process
	if (spawnpid == -1) {
        perror("Hull Breach!\n");
		return 1;
	} else if (spawnpid == 0) { //successful child fork
        // Set ^C as default exit
        sigint.sa_handler = SIG_DFL;
        sigaction(SIGINT, &sigint, NULL);

        // Handle input file if one was provided (Lecture 3.4 for reading)
        if (strcmp(inputf, "") != 0) { //filename for input piping
            int input = open(inputf, O_RDONLY); // read the file by provided name
            if (input == -1) { // non-existant file
                perror("Unable to open input file\n");
                return 1;
            }
            // assign it
            result = dup2(input, 0);
            if (result == -1) {
                perror("Unable to assign input file\n");
                return 2;
            }
            // trigger its close
            fcntl(input, F_SETFD, FD_CLOEXEC);
        }
        // Handle output file if one was provided (Lecture 3.4 for writing)
        if (strcmp(outputf, "") != 0) {
            // open it
            int output = open(outputf, O_WRONLY | O_CREAT | O_TRUNC, 0666); // read write file by provided name
            if (output == -1) { //non-existant file
                perror("Unable to open output file\n");
                return 1;
            }
            // assign it
            result = dup2(output, 1);
            if (result == -1) {
                perror("Unable to assign output file\n");
                return 2;
            }
            // trigger its close
            fcntl(output, F_SETFD, FD_CLOEXEC);
        }
        // Execute user command with provided arguments to follow
        if (execvp(argVars[0], argVars)) {
            printf("No such file or directory: %s\n", argVars[0]); //couldn't find file
            fflush(stdout);
            return 2;
        }
        //add the pid to the struct of children
        childIndex = addChild(getpid(), bg, childProcesses);
	} else { //parent process, monitor children
        // Execute a process in the background ONLY if allowBackground
        childIndex = getChildIndex(spawnpid, childProcesses);
		if (bg && !foregroundOnly) { // create a background task
			// pid_t actualPid = waitpid(spawnpid, &childProcesses[childIndex].childExitStatus, WNOHANG);
			printf("background pid is %d\n", spawnpid);
			fflush(stdout);
		} else { // execute forground task
			pid_t actualPid = waitpid(spawnpid, &childProcesses[childIndex].childExitStatus, 0);
		}

    }

    //wait for background processes to terminate

    while ((spawnpid = waitpid(-1, &childProcesses[getChildIndex(spawnpid, childProcesses)].childExitStatus, WNOHANG)) > 0) {
        printf("child %d terminated\n", spawnpid);
        printExitStatus(childProcesses[getChildIndex(spawnpid, childProcesses)].childExitStatus);
        fflush(stdout);
    }
    return 0;
}

/*******************************************************************************
 *                                  Add Child                                  *
 * The addChild function adds a new pid to the struct array of children.       *
 *                                                                             *
 * Arguments: childPID as new child pid, bg if the process is in the background*
 *              childProcesses as the struct to add to.                        *
 * Returns  : returns the index added to the arry, or -1 if child add failed   *
 *******************************************************************************/
int addChild(pid_t childPID, bool bg, struct childProcs* childProcesses) {
    int i;
    for (i = 0; i < sizeof(childProcesses); i++) {
        if (childProcesses[i].pid == -1) { //empty, use this space
            childProcesses[i].pid = childPID;
            childProcesses[i].background = bg;
            childProcesses[i].childExitStatus = 0;
            return i;
        }
    }
    return -1; //child add failed
}

/*******************************************************************************
 *                                  Child Index                                *
 * The getChildIndex function returns the index a provided child pid would be  *
 *      found in the childProcesses struct array.                              *
 *                                                                             *
 * Arguments: childPID as the search pid, childProces.. as the struct to add to*
 * Returns  : returns the index added to the arry, or -1 if not found in array *
 *******************************************************************************/
int getChildIndex(pid_t childPID, struct childProcs* childProcesses) {
    int i;
    for (i = 0; i < sizeof(childProcesses); i++) {
        if (childProcesses[i].pid == childPID) {
            return i;
        }
    }
    return -1; //non-existant pid
}

/*******************************************************************************
 *                              Interpret Arguments                            *
 * The interpretArguments function takes the provided argVars array to call    *
 *      the built in functions of the smallsh.                                 *
 *                                                                             *
 * Arguments: argVars as the array of command and its arguments,  inputf to    *
 *              store a provided input file, outputf to store a provided output*
 *              file, sigint as a way to provide child processes ^C support,   *
 *              childProcesses to keep track of the children, bg if the command*
 *              should be made a background process, childExitStatus to return *
 *              an exit status to keep track of background and child processes *
 * Returns  : return 0 for an alive break to continue prompting user, 1 for a  *
 *              safe break to end the process, or 2 for error break.           *
 *******************************************************************************/
int interpretArguments(char** argVars, char* inputf, char* outputf, struct sigaction sigint, struct childProcs* childProcesses, bool bg, int childExitStatus) {
    if (argVars[0][0] == '#' || argVars[0][0] == '\0') {
        return 0; //alive break, comment/blank doesn't affect shell, get another
    } else if (strcmp(argVars[0], "exit") == 0) { //user requests exit shell, safe exit
        return 1; //safe break
    } else if (strcmp(argVars[0], "cd") == 0) { //user requests a directory change
        if (strcmp(argVars[1], "") == 0) { //user asked for dir change without a new dir
            chdir(getenv("HOME")); //if not new path was provided, go to user home
        } else { // a path was provided, ensure it's a real directory
            if (chdir(argVars[1]) == -1) { // attempt to change dir, if -1 dir doesn't exist
                printf("Directory does not exist.\n");
                fflush(stdout);
            }
        }
        return 0;
    } else if (strcmp(argVars[0], "status") == 0) { //user requests to view exit status
        // print exit status
        printExitStatus(childExitStatus);
        return 0; //alive break;
    } else { //command was found
        childExitStatus = runCommand(argVars, inputf, outputf, sigint, childProcesses, bg);
        return childExitStatus;
    }
    return 0; //alive break, get more user input
}

/*******************************************************************************
 *                              Print Exit status                              *
 * The printExitStatus function is a helper function to print the current      *
 *      process exit status.                                                   *
 *                                                                             *
 * Arguments: exitChildStatus a provided exit status from smallsh              *
 * Returns  : Nothing.                                                         *
 *******************************************************************************/
void printExitStatus(int exitChildStatus) {
    if (WIFEXITED(exitChildStatus)) {
		// If exited by status
		printf("exit value %d\n", WEXITSTATUS(exitChildStatus));
	} else {
		// If terminated by signal
		printf("terminated by signal %d\n", WTERMSIG(exitChildStatus));
	}
}

//FOR DEBUG ONLY, prints argvars
void printUserInput(char** argVars) {
    int i;
    for (i = 0; i < sizeof(argVars); i++) {
        if (argVars[i] != NULL)
            printf("%s\n", argVars[i]);
    }
}

/*******************************************************************************
 *                          Toggle Forground Only Mode                         *
 * The toggleForegroundMode function is a signal handler helper function to    *
 *      catch the ^Z operator and toggle on or off the ForgroundOnly mode.     *
 *                                                                             *
 * Arguments: signo (not sure, was in lecture noets but isn't called upon)     *
 * Returns  : Nothing.                                                         *
 *******************************************************************************/
void toggleForegroundMode(int signo) {
    if (foregroundOnly == false) {
    	write(1, "Entering foreground-only mode (& is now ignored)\n", 49);
    	fflush(stdout);
    	foregroundOnly = true;
    } else {
    	write (1, "Exiting foreground-only mode\n", 29);
    	fflush(stdout);
    	foregroundOnly = false;
    }
}

/*******************************************************************************
 *                                  SmallSH                                    *
 * The smallsh function is called upon by main to start the parent process     *
 *      shell. We then run a loop to continue getting user inputs until a sig  *
 *      handler is caught, an exit is returned, or an error is thrown.         *
 *                                                                             *
 * Arguments: None.                                                            *
 * Returns  : exit status.                                                     *
 *******************************************************************************/
int smallsh() {
    pid_t shellPID = getpid();
    int breakCode = 0;
    int i, childExitStatus = 0;
    bool background = false;

    char** argVars = malloc(sizeof(char*) * 512);
    char* fInputName = malloc(sizeof(char) * 256);
	char* fOutputName = malloc(sizeof(char) * 256);

    struct childProcs* childProcesses = malloc(sizeof(struct childProcs) * 10);

    //before we get inputs we need to define signal handlers
    //https://www.gnu.org/software/libc/manual/html_node/Sigaction-Function-Example.html
    // Catch ^C
	struct sigaction sigint = {0};
    /* Set up the structure to specify the new action. */
	sigint.sa_handler = SIG_IGN;
	sigfillset(&sigint.sa_mask);
	sigint.sa_flags = 0;
    //assign new action to hault only foreground process
	sigaction(SIGINT, &sigint, NULL);

	// Catch ^Z
	struct sigaction sigstp = {0};
    /* Set up the structure to specify the new action. */
	sigstp.sa_handler = toggleForegroundMode;
	sigfillset(&sigstp.sa_mask);
	sigstp.sa_flags = 0;
    //assign new action to hault all processes
	sigaction(SIGTSTP, &sigstp, NULL);

    //init variables
    for (i = 0; i < sizeof(argVars); i++) {
        argVars[i] = NULL;
    }

    while (breakCode == 0) {
        printf(": ");
        getInput(argVars, fInputName, fOutputName, shellPID, background);
        //debugging
        //printUserInput(argVars);

        //interpret user input
        breakCode = interpretArguments(argVars, fInputName, fOutputName, sigint, childProcesses, background, childExitStatus);
        //printf("DEBUG: breakcode - %d\n", breakCode);
        //clean up variables
		for (i = 0; i < sizeof(argVars); i++) {
    		argVars[i] = NULL;
		}
		background = false;
		fInputName[0]  = '\0';
		fOutputName[0] = '\0';
    }
    free(argVars);
    free(fInputName);
    free(fOutputName);
    free(childProcesses);
    return breakCode;
}

int main() {
    return smallsh();
}
