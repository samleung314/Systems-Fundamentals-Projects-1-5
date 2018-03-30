#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "hw1.h"

Test(hw1_tests_suite, validargs_help_test) {
    int argc = 2;
    char *argv[] = {"bin/hw1", "-h", NULL};

    unsigned short ret = validargs(argc, argv);
    unsigned short exp = 0x8000;

    cr_assert_eq(ret & exp, exp, "Most significant bit not set for -h");
}

Test(hw1_tests_suite, validargs_encrypt_test) {
    int argc = 5;
    char *argv[] = {"bin/hw1", "-f", "-e", "-k", "ARGS", NULL};

    unsigned short ret = validargs(argc, argv);
    unsigned short exp = 0x4000;

    cr_assert_eq(ret & exp, exp, "Invalid value of valid args. Got: %d | Expected: %d"
        , ret, exp);
}

Test(hw1_tests_suite, help_system_test) {
    char *cmd = "bin/hw1 -h";

    // system is a syscall defined in stdlib.h
    // it takes a shell command as a string and runs it
    // we use WEXITSTATUS to get the return code from the run
    // use 'man 3 system' to find out more
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS, "Program exited with %d instead of EXIT_SUCESS", return_code);
}
