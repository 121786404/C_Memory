#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>

/* First half of main is dedicated to reading in and parsing
   the arguments that will be used with execvpe.             */
int main(int argc, char *argv[]) {

    char *args[4] = {NULL, NULL, NULL, NULL};
    char *env[] = {"LD_PRELOAD=./memory_shim.so", NULL};

    /* 1a. Make sure that they did not give a directory as an argument. If so, print message and exit. */
    struct stat sb;
    stat(argv[1], &sb);
    if(S_ISDIR(sb.st_mode)) {
	printf("You must enter a program as an argument, not directory. Exiting Leakout Program.\n");
	exit(1);
    }

    /* 1b. If there was no program arg entered print an appropriate error message and exit. */
    if(argc == 1) {
       printf("Need to enter program to run leak test on. Exiting Leakcount Program.\n");
       exit(1);
    }
	
    /* 1c. Extract the file name. */
    args[0] = basename(argv[1]);
	
    /* 1d. If an argument was given for the program, save it for execvp. */
    if(argc == 3) {
	args[1] = argv[2];
    }

    /* 2a. Fork the process. The child will used the parsed argument info
           from the first part of main to call execvpe, which will in turn,
           replace the child with the specifed program and set the env to
           load my shim library first.					   */
    if(fork() == 0) {
	execvpe(argv[1], args, env);
	perror("execvpe");
    }

    /* 2b. Have the parent wait on the child process to finish execution. */
    wait(NULL);
    return 0;
}

