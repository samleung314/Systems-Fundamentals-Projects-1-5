#ifndef SFISH_H
#define SFISH_H

/* Format Strings */
#define EXEC_NOT_FOUND "sfish: %s: command not found\n"
#define JOBS_LIST_ITEM "[%d] %s\n"
#define STRFTIME_RPRMT "%a %b %e, %I:%M%p"
#define BUILTIN_ERROR  "sfish builtin error: %s\n"
#define SYNTAX_ERROR   "sfish syntax error: %s\n"
#define EXEC_ERROR     "sfish exec error: %s\n"

//part1
char* prompt();
void rightPrompt();
char** parseline(char* input);
void freeStuff();

//builtin commands
int check_builtin(char **argv);
void builtinSelect(int n, char *args[]);
void builtin_help();
void builtin_exit();
void builtin_cd(char *args[]);
void builtin_pwd();
void builtin_color(char* color);
void builtin_error();

//part2
void evaluate(char** argv);
void unix_error(char *msg);

//part3
extern bool isOut;
extern bool isIn;
extern bool pipeIn;
extern bool pipeOut;
extern bool badSyntax;
void parseRedirection(char** argv);
char*** splitPipe(char** argv);
void redirect();
char* spacify(char* input);
int piping(char** argv);
void freeSplitPipe(char*** splitPipe);
void restoreOut();

//part4;
extern int mainFD;
extern int bg;
void jobControl(char **argv);
void builtin_jobs();
void builtin_fg();
void builtin_kill();

//colors
#define NONE "\033[0m"
#define RED "\033[1;31m"
#define GRN "\033[1;32m"
#define YEL "\033[1;33m"
#define BLU "\033[1;34m"
#define MAG "\033[1;35m"
#define CYN "\033[1;36m"
#define WHT "\033[1;37m"
#define BWN "\033[0;33m"

#endif
