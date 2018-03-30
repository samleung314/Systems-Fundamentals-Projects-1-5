#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <errno.h>
#include <wait.h>
#include <unistd.h>

#include "sfish.h"
#include "debug.h"

bool isOut, isIn, hasPipe, pipeIn, pipeOut, badSyntax;
char *inputFile = NULL, *outputFile = NULL, *pipeFile = NULL;

//PART 2
void parseRedirection(char** argv){
    isOut = false, isIn = false; pipeIn = false; pipeOut = false; hasPipe = false; badSyntax = false;
    char** pointer = argv;
    int leftCount = -2, rightCount = -2;

    bool firstPipe = false, angle = false, secondPipe = false;
    bool pipeBeforeLeft = false, pipeAfterRight = false;

    while(*pointer != NULL){
        int next =
        *(pointer + 1) == NULL ||
        *(*(pointer+1))=='<' ||
        *(*(pointer+1))=='>' ||
        *(*(pointer+1))=='|';

        //check for <
        if(strcmp(*pointer, "<") == 0){
            inputFile = *(pointer+1);
            isIn = true;
            leftCount++;

            //error cases
            if(next || !leftCount) badSyntax = true;
            if(hasPipe) pipeBeforeLeft = true;
            if(firstPipe) angle = true;
        }

        //check for >
        else if(strcmp(*pointer, ">") == 0){
            outputFile = *(pointer+1);
            isOut = true;
            rightCount++;

            //error cases
            if(next || !rightCount) badSyntax = true;

            if(firstPipe) angle = true;
        }

        //check for |
        else if(strcmp(*pointer, "|") == 0){
            hasPipe = true;

            //error cases
            if(next) badSyntax = true;
            if(isOut) pipeAfterRight = true;

            if(!firstPipe) firstPipe = true;
            if(firstPipe && angle) secondPipe = true;
        }
        pointer++;
    }
    pipeIn = isIn && hasPipe;
    pipeOut = isOut && hasPipe;

    if(isIn && isOut && hasPipe) badSyntax = true;
    if(secondPipe || pipeBeforeLeft || pipeAfterRight) badSyntax = true;

    //Separate args from redirection
    pointer = argv;
    while(*pointer != NULL && !pipeIn){
        int isSymbol =
        **pointer=='<' ||
        **pointer=='>';

        if(isSymbol){
            *pointer = NULL;
            break;
        }

        pointer++;
    }
}

int origOut;
void redirect(){
    if(inputFile != NULL){
        FILE *input = fopen(inputFile, "r");
        fflush(stdin);
        dup2(fileno(input), fileno(stdin)); //stdin fd now points to input fd
    }

    if(outputFile != NULL){
        origOut = dup(fileno(stdout));
        FILE *output = fopen(outputFile, "w+");
        fflush(stdout);
        dup2(fileno(output), fileno(stdout)); //stdout fd now points to output fd
    }
}

void restoreOut(){
    dup2(origOut, 1);
}

char* spacify(char* input){
    char* pointer = input;
    int length = 0;

    //Count number of < > |
    while(*pointer){
        int isSymbol =
        *pointer=='<' ||
        *pointer=='>' ||
        *pointer=='|' ||
        *pointer=='&';

        if(isSymbol){
            length+=3; //add 1 for symbol, 2 for spaces
        }else{
            length++; //else just add 1
        }

        pointer++;
    }
    length++; //add 1 for null terminator

    char* spaced = malloc(length * sizeof(char));
    char* start = spaced;
    pointer = input;

    while(*pointer){
        int isSymbol =
        *pointer=='<' ||
        *pointer=='>' ||
        *pointer=='|' ||
        *pointer=='&';

        if(isSymbol){
            *spaced = ' ';
            spaced++;
            *spaced = *pointer;
            spaced++;
            *spaced = ' ';
            spaced++;
        }else{
            *spaced = *pointer; //copy the char
            spaced++;
        }
        pointer++;
    }
    *spaced = '\0';

    return start;
}

//PART 3
int piping(char** argv){
    if(!hasPipe) return 0;

    char*** splitted = splitPipe(argv);

    pid_t pid;
    int last_stdin = fileno(stdin);
    int pipefd[2];

    //execute command segments
    while(*splitted != NULL){
        int lastCommand = (*(splitted+1) == NULL);
        int help = (strcmp((*splitted)[0], "help") == 0);
        int pwd = (strcmp((*splitted)[0], "pwd") == 0);
        pipe(pipefd);
        pid = fork();

        //CHILD
        if(pid == 0){
            dup2(last_stdin, 0); //makes 0(stdin) point to the last stdin of prev command

            if(lastCommand && pipeOut) redirect();
            //if not last command
            if(!lastCommand){
                dup2(pipefd[1], 1); //makes 1(stdout) point to (pipe write)pipefd[1]
            }
            close(pipefd[0]); //close read end

            if(help){ builtin_help();
            }else if(pwd){ builtin_pwd();
            }else if(execvp((*splitted)[0], *splitted) < 0) exit(EXIT_FAILURE);

        //PARENT
        }else if(pid > 0){
            //wait for child to exit
            int status;
            if(waitpid(pid, &status, 0) < 0){
                unix_error("wait foreground: waitpid error");
            }
            close(pipefd[1]); //close write end

            last_stdin =  pipefd[0]; //use for input of next cmd
            splitted++;

        //FAILTURE
        }else{
            exit(EXIT_FAILURE);
            debug("Fork failure");
        }
    }

    return 1;
}

char*** splitPipe(char** argv){
    if(pipeIn){
        while(strcmp(*argv, "<")){
            argv++;
        }
        argv+=3; //advance past <, input, and pipe

        pid_t pid = fork();
        int help = (strcmp(argv[0], "help") == 0);
        int pwd = (strcmp(argv[0], "pwd") == 0);
        //CHILD
        if(pid == 0){
            redirect();
            //HELP
            if(help){
                builtin_help();
            }
            //PWD
            else if(pwd){
                builtin_pwd();
            }
            //EXECUTABLE
            else if(execvp(argv[0], argv) < 0){
                printf(EXEC_NOT_FOUND, argv[0]);
                exit(EXIT_SUCCESS);
            }

            int status;
            if(waitpid(pid, &status, 0) < 0){
                unix_error("wait foreground: waitpid error");
            }
        }
    }

    char** pointer = argv;

    //count number of command segments separated by pipes
    int cmdCount = 1;   //cmdCount = (number of pipes) + 1
    while(*pointer != NULL){
        if(strcmp(*pointer, "|") == 0) cmdCount++;
        pointer++;
    }
    pointer = argv; //reset pointer

    //store the command segments
    char*** cmdSegs = malloc((cmdCount+1) * sizeof(char**)); //add one for null term
    cmdSegs[cmdCount] = NULL;

    //process command segs
    //count number of args
    int numArgs = 0;
    char** firstArg = pointer;

    for(int i=0; i<cmdCount; i++){
        firstArg = pointer; //location of first arg
        numArgs = 0;
        while(*pointer != NULL && strcmp(*pointer, "|") != 0){
            numArgs++;
            pointer++;
        }
        pointer++; //go past pipe;

        cmdSegs[i] = malloc((numArgs+1) * sizeof(char*));

        //copy args to commandSeg array of numArgs
        for(int j=0; j<numArgs; j++){
            cmdSegs[i][j] = *firstArg;
            firstArg++;
        }
        cmdSegs[i][numArgs] = NULL; //terminate with NULL
    }
    return cmdSegs;
}

void freeSplitPipe(char*** splitPipe){
    char*** pointer = splitPipe;
    while(*pointer != NULL){
        free(*pointer);
        pointer++;
    }

    free(splitPipe);
}

void unix_error(char *msg){
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

//PART 4
void jobControl(char **argv){
    pid_t pid;
    int help = (strcmp(argv[0], "help") == 0);
    int pwd = (strcmp(argv[0], "pwd") == 0);

    pid = fork();
        //CHILD
    if(pid == 0){
        pid = getpid();
        //create new job
        // job newJob;
        // newJob.jobNum = jobList;
        // newJob.pgid = pid;
        // tcsetpgrp (mainFD, newJob.pgid);
        //add first job

        //reset signal handlers
        signal(SIGINT, SIG_DFL); //crtl c
        signal(SIGTSTP, SIG_DFL); //crtl z

        if(isOut || isIn) redirect();

            //HELP
        if(help){
            builtin_help();
        }
            //PWD
        else if(pwd){
            builtin_pwd();
        }
            //EXECUTABLE
        else if(execvp(argv[0], argv) < 0){
            if(isOut || isIn) restoreOut();
            printf(EXEC_NOT_FOUND, argv[0]);
            exit(EXIT_SUCCESS);
        }
    }

    /* PARENT waits for foreground job to terminate */
    if(!bg){
        int status;
        if(waitpid(pid, &status, 0) < 0){
            unix_error("wait foreground: waitpid error");
        }
    }
}

void builtin_fg(){

}

void builtin_kill(){

}