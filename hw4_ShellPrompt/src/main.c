#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <wait.h>
#include <signal.h>
#include <termios.h>

#include "sfish.h"
#include "debug.h"

int main(int argc, char *argv[], char* envp[]) {
    char* input;

    if(!isatty(STDIN_FILENO)) {
        if((rl_outstream = fopen("/dev/null", "w")) == NULL){
            perror("Failed trying to open DEVNULL");
            exit(EXIT_FAILURE);
        }
    }

    //ignore signals for shell
    signal(SIGINT, SIG_IGN); //crtl c
    signal(SIGTSTP, SIG_IGN); //crtl z

    while(true){
        //prompt for input
        char* leftPrompt = prompt();
        input = readline(leftPrompt);
        char* pointer = input;

        if(pointer == NULL) exit(EXIT_SUCCESS);
        if(strcmp(pointer, "") == 0) continue;

        while(pointer && (*pointer == ' ')){pointer++;} //ignore leading spaces

        //spacify input
        char* spaced = spacify(pointer);

        //parse input into tokens
        char** argv = parseline(spaced);

        //evaluate commands
        evaluate(argv);

        free(spaced);
        // free(argv);
        // free(leftPrompt);
        rl_free(input);
    }
}