#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <errno.h>
#include <wait.h>
#include <time.h>
#include <sys/ioctl.h>

#include "sfish.h"
#include "debug.h"

char* line, tokenTotal;
char* builtins[] = {
    "help",
    "exit",
    "cd",
    "pwd",
    "color",
    "jobs"
};

char* color = NONE;
char* prompt(){
    char* home  = getenv("HOME"); // /home/student
    char* tilda = "~";
    char* path  = getcwd(NULL, 0); //gets current path
    char* prompt = " :: samleung >> " NONE;

    char* leftPrompt = malloc(strlen(path) + strlen(prompt) + 1); //1 for null term

    //choose to replace home with ~
    if(strcmp(home, path) && strlen(path)>strlen(home)){
        strcpy(leftPrompt, tilda);
        strcat(leftPrompt, path + strlen(home));
        strcat(leftPrompt, prompt);

    }else{
        strcpy(leftPrompt, home);
        strcat(leftPrompt, prompt);
    }

    free(path);
    rightPrompt();
    printf("%s", color);
    return leftPrompt;
}

void rightPrompt(){
//build trailing prompt

    //get terminal width
    struct winsize terminal;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal);
    int width = terminal.ws_col;

    //get formatted time
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[85];
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 85, STRFTIME_RPRMT, timeinfo);

    width -= strlen(buffer);
    //hey
    char* save = "\e[s";
    char widthChar[8];
    sprintf(widthChar, "\e[%dC", width);
    char* restore = "\e[u";

    int promptLen = strlen(save) + strlen(widthChar) + strlen(buffer) + strlen(restore);
    char withTime[promptLen];

    strcpy(withTime, save);
    strcat(withTime, widthChar);
    strcat(withTime, buffer);
    strcat(withTime, restore);

    printf("%s", withTime);
}

int bg;
char** parseline(char* input){
    int size = 10, numTokens = 0;

    char** tokens = malloc(size * sizeof(char*));
    char* delims = " \n\r\t";
    char* pointer;
    char* savePtr;

    pointer = strtok_r(input, delims, &savePtr);
    while(pointer != NULL){
        tokens[numTokens] = pointer;
        numTokens++;

        if(numTokens >= size){
            size += 10;
            tokens = realloc(tokens, size * sizeof(char*));
        }

        pointer = strtok_r(NULL, delims, &savePtr);
    }
    tokens[numTokens] = NULL;

    //detect ending & for bg process and remove from list
    if(strcmp(tokens[numTokens-1], "&") == 0){
        bg = 1;
        tokens[numTokens-1] = NULL;
        numTokens--;
    }else{
        bg = 0;
    }
    tokenTotal = numTokens;

    parseRedirection(tokens); //check for redirections and remove from list
    return tokens;
}

void evaluate(char** argv){
    if(badSyntax){
        printf(SYNTAX_ERROR, "Unsupported redirection sequence");
        return;
    }
    if(piping(argv)) return; // Only execute piping code

    if(!check_builtin(argv)){
        jobControl(argv);
    }
}

int check_builtin(char** argv){
    //check for help/pwd cmd
    int needFork = (strcmp(argv[0], builtins[0]) == 0) || (strcmp(argv[0], builtins[3]) == 0);
    if(needFork){
        return 0;
    }

    //check for builtin commands
    int builtinNum = sizeof(builtins)/sizeof(builtins[0]);
    for(int i=0; i<builtinNum; i++){
        if(strcmp(argv[0], builtins[i]) == 0){
            builtinSelect(i, argv);
            return 1;
        }
    }
    return 0;
}

void builtinSelect(int n, char *args[]){
    switch(n){
        case 0:
            debug("Do not call help with builtinSelect");
            break;

        case 1:
            builtin_exit();
            break;

        case 2:
            builtin_cd(args);
            break;

        case 3:
            debug("Do not call pwd with builtinSelect");
            break;

        case 4:
            builtin_color(args[1]);
            break;

        case 5 :
            builtin_jobs();
    }
}

void builtin_jobs(){

}

void builtin_help(){
    printf("help - Print a list of all builtins and their basic usage in a single column\n"
        "exit - Exits the shell\n"
        "cd - Changes the current working directory of the shell\n"
        "pwd - Prints the absolute path of the current working directory\n"
        "color - Changes the color of the prompt\n");
    exit(0);
}

void builtin_exit(){;
    freeStuff();
    exit(EXIT_SUCCESS);
}

char* prevDir;
char* currDir;
void builtin_cd(char *args[]){
    char* preprevDir = prevDir;

    if(tokenTotal == 1){
        prevDir = getcwd(NULL, 0);
        builtin_error(chdir(getenv("HOME")));
        currDir = getcwd(NULL, 0);

    }else if(strcmp(args[1], ".") == 0){
        builtin_error(chdir("."));
        prevDir = getcwd(NULL, 0);
        currDir = getcwd(NULL, 0);

    }else if(strcmp(args[1], "..") == 0){
        prevDir = getcwd(NULL, 0);
        builtin_error(chdir(".."));
        currDir = getcwd(NULL, 0);

    }else if(strcmp(args[1], "-") == 0){
        prevDir = getcwd(NULL, 0);
        builtin_error(chdir(preprevDir));
        currDir = getcwd(NULL, 0);
    }else{
        prevDir = getcwd(NULL, 0);
        builtin_error(chdir(args[1]));
        currDir = getcwd(NULL, 0);
    }

    //free previous directory calls
    free(preprevDir);
}

void builtin_pwd(){
    char* path = getcwd(NULL, 0); //gets absolutepath
    printf("%s\n", path);
    free(path);
    exit(0);
}

void builtin_color(char* name){
    char *names[] = {"RED", "GRN", "YEL", "BLU", "MAG", "CYN", "WHT", "BWN"};
    char *colors[] = {RED, GRN, YEL, BLU, MAG, CYN, WHT, BWN};

    for(int i=0; i<8; i++){
        if(strcmp(names[i], name) == 0){
            color = colors[i];
        }
    }
}

void builtin_error(int num){
    if(num < 0){
        printf(BUILTIN_ERROR, strerror(errno));
    }
}

void freeStuff(){
    free(prevDir);
    free(currDir);
}

int countGit(){
    char* git[] = {"git", "status", "-s", NULL};
    evaluate(git);
    return 0;
}