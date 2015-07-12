/********************************************************
**  myar.c
**
**  Created by Cierra Shawe on 7/8/15.
**  Copyright (c) 2015 Cierra Shawe. All rights reserved.
*********************************************************/

/* Defines */
#define _POSIX_C_SOURCE 200809L
#define _BSD_SOURCE
#define BSIZE 128 // Default buffer size
#define endl printf("\n")

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ar.h>

/* function declarations */
void print_usage(); //prints out how to use program
void q(); // handles -q
void x(); // handles -x
int isAr(int, char *const *); // handles -t
void t();
void v(); // handles -v
void d(); // handles -d
void A(); // handles -A
//void w(); //EXTRA CREDIT

void open_file(){



}

void append_file(){}



/* this is used to give a default name in case of no archive name being specified */
void createAr(){}



int main(int argc, char *const argv[]) {

    int current = argv[1][1];

    for (int i = 0; i < argc; i++) {
        printf("argv[%i]: %s\n", i, argv[i]);
    }
    /* No arguments passed */
    if (argc == 1 || argc == 2)
        print_usage();

    /* As long as there are at least two arguments */
    else if (argc > 2 && argv[1][1]) {

        if(isAr(argc, argv) == 1)
            printf("Using archive: %s\n", argv[2]);
        else
            printf("Invalid archive.\n");

        /* Controls what happens when the flags are recieved*/
        switch (current) {
            case 'q':
                q();
                break;

            case 'x':
                x();
                break;

            case 't':
                t(argc, argv);
                break;

            case 'd':
                d();
                break;

            case 'A':
                A();
                break;

            default:
                print_usage();
                break;
        }

    }

    /* -w Extra credit: for a given timeout, add all modified files to the archive. (except the archive itself) */

    return 0;
}



/* -q quickly append named files to archive */
void q(){

    printf("I recieved a q!\n");

}


/* -x extract named files */
void x(){

    printf("I recieved a x!\n");

}

int isAr(int argc, char *const argv[]){

    for (int i = 0; i < sizeof(argv[2])/sizeof(char); i++)
        if(argv[2][i] == '.')
            if (argv[2][i+1] == 'a')
                return 1;

    print_usage();
    return 0;

}

/* -t print a concise table of contents of the archive */
void t(){

    printf("I recieved a t!\n");

}

/* -l option is given, the following information is displayed for each
 file: file mode, number of links, owner name, group name, number of bytes
 in the file, abbreviated month, day-of-month file was last modified, hour
 file last modified, minute file last modified, and the pathname. */
/* -v iff specified with -t, print a verbose table of contents of the archive */
void v(){

    printf("I recieved a v!\n");

}


/* -d delete named files from the archive */
void d(){


    printf("I recieved a d!\n");

}


/* -A quickly append all “regular” files in the current directory (except the archive itself) */
void A(){

    printf("I recieved an A!\n");

}

/* prints out the usage of myar */
void print_usage(){

    printf("usage: ./myar [-qxtvdA] <archivename>.a <files> ... \n\t\t"
           "-q: quickly append named files to archive\n\t"
           "-x: extract named files\n\t\t"
           "-t: print a concise table of contents of the archive\n\t\t"
           "-v: iff specified with -t, print a verbose table of contents of the archive\n\t\t"
           "-d: delete named files from the archive\n\t\t"
           "-A: quickly append all “regular” files in the current directory\n"

           //"        Extra credit: \n"
           //"        -w for a given timeout, add all modified files to the archive.\n"
           //"            myar -w archive [file ...]\n"
           );
    
}


/* ~~~~~~ Notes ~~~~~~ */
/* The archive file must use exactly the standard format defined in /usr/inc1ude/ar.h, and may be tested with archives created with the ar command. The options above are compatible with the options having the same name in the ar command. -A is a new option not available with the usual ar command.

 For the -q command myar should create an archive file if it doesn’t exist, using permissions 666. For the other commands myar reports an error if the archive does not exist, or is in the wrong format.

 You will have to use the system calls stat and utime to properly deal with extracting and restoring timestamps. Since the archive format only allows one timestamp, store the mtime and use it to restore both the atime and mtime. Permissions should also be restored to the original value, subject to umask limitations.

 The -q and -A commands do not check to see if a file by the chosen name already exists. It simply appends the files to the end of the archive.

 The -x and -d commands operate on the first file matched in the archive, without checking for further matches.
 In the case of the -d option, you will have to build a new archive file to recover the space. Do this by unlinking the original file after it is opened, and creating a new archive with the original name.

 You must handle multiple file names as arguments.

 File I/O is expensive, do not make more than one pass through the archive file (this is especially relevant to the multiple delete case).

 For the -w flag, the command will take as long as specified by the timeout argument. You should print out a status message upon adding a new file. This may result in many different copies of the same file in the archive. More extra credit: any time a file is added that already exists, remove the old copy from the archive if it is not the same. If identical, do not add the new file.
 */
