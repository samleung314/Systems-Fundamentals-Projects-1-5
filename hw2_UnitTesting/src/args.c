#include "debug.h"
#include "utf.h"
#include "wrappers.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *STR_UTF16BE  = "UTF16BE";
char *const STR_UTF16LE = "UTF16LE";
char const *STR_UTF8  = "UTF8";

int opterr;
int optopt;
int optind;
char *optarg;

state_t *program_state;

void
parse_args(int argc, char *argv[])
{
  int i;
  char option;

  program_state = Calloc(1, sizeof(state_t));

  for (i = 0; optind < argc; ++i) {
    debug("%d opterr: %d", i, opterr);
    debug("%d optind: %d", i, optind);
    debug("%d optopt: %d", i, optopt);
    debug("%d argv[optind]: %s", i, argv[optind]);
    if ((option = getopt(argc, argv, "+ie:")) != -1) {
      switch (option) {

        case 'e': {
          info("Encoding Argument: %s", optarg);
          if ((program_state->encoding_to = determine_format(optarg)) == 0)
            print_state();
          break;
        }

        case '?': {
          if (optopt != 'h'){
            fprintf(stderr, KRED "-%c is not a supported argument\n" KNRM, optopt);
            free(program_state);
            USAGE(argv[0]);
            free(program_state);
            exit(EXIT_FAILURE);
          }
          free(program_state);
          USAGE(argv[0]);
          exit(0);
          break;
        }

        case 'h':{
          USAGE(argv[0]);
          free(program_state);
          exit(0);
          break;
        }

        default: {
          break;
        }
      }
    }
    else if(argv[optind] != NULL){
      if (program_state->in_file == NULL) {
        program_state->in_file = argv[optind];
      }
      else if(program_state->out_file == NULL)
      {
        program_state->out_file = argv[optind];
      }
      optind++;
    }else if(argc == 1){ // NO ARGUMENTS
        USAGE(argv[0]);
        free(program_state);
        exit(EXIT_FAILURE);
    }
  }
}

format_t
determine_format(char *argument)
{
  if (strcmp(argument, STR_UTF16LE) == 0)
    return UTF16LE;
  if (strcmp(argument, STR_UTF16BE) == 0)
    return UTF16BE;
  if (strcmp(argument, STR_UTF8) == 0)
    return UTF8;
  return 0;
}

const char*
bom_to_string(format_t bom){
  switch(bom){
    case UTF8: return STR_UTF8;
    case UTF16BE: return STR_UTF16BE;
    case UTF16LE: return STR_UTF16LE;
  }
  return "UNKNOWN";
}

char*
join_string_array(int count, char *array[])
{
  char *ret;

  int i;
  int len = 0, str_len,cur_str_len;

  str_len = array_size(count, array);
  char *charArray;
  charArray = (char *) malloc(str_len);
  ret = charArray;

  printf("%d", str_len);

  for (i = 0; i < count; ++i) {
    cur_str_len = strlen(array[i]);
    memcpy(ret + len, array[i], cur_str_len);
    len += cur_str_len;
    memcpy(ret + len, " ", 1);
    len ++;
  }

  return ret;
}

int
array_size(int count, char *array[])
{
  int i, sum = 1; /* NULL terminator */
  for (i = 0; i < count; ++i) {
    sum += strlen(array[i]);
    ++sum; /* For the spaces */
  }
  return sum+1;
}

void
print_state()
{
  if (program_state == NULL) {
    error("program_state is %p", (void*)program_state);
    free(program_state);
    exit(EXIT_FAILURE);
  }
  info("program_state {\n"
         "  format_t encoding_to = 0x%X;\n"
         "  format_t encoding_from = 0x%X;\n"
         "  char *in_file = '%s';\n"
         "  char *out_file = '%s';\n"
         "};\n",
         program_state->encoding_to, program_state->encoding_from,
         program_state->in_file, program_state->out_file);
}
