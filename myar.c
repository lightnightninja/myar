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
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
/* 
 * Information on how to use this found at:
 * http://www.mit.edu/afs.new/athena/system/rs_aix32/os/usr/include/ar.h 
 */
#include <ar.h>

/* function declarations */
void print_usage(); //prints out how to use program
void quick_append(); // handles -q
void extract(); // handles -x
void print_table(int);
void delete(); // handles -d
void append_all(); // handles -A
//void w(); //EXTRA CREDIT

int  _verbose(const char **); // handles -v
int _check_input(int, const char **, int);
void _append();
void _create_ar();
int _is_file_path();


int main(int argc, const char *argv[]) {

    int     current = argv[1][1];
    int     v = 0;
    int     arch_name_pos = 2;

    char    *arch;
    //int     arch_fd;

    /* ensuring correct input */
    if (argv[1][1] == 't') {
        v = _verbose(argv);
        if (v == 1)
            arch_name_pos++;
        if (v == 0)
            v = 1;
    }

    if(_check_input(argc, argv, arch_name_pos) == 1){
        printf("Using archive: %s\n", argv[arch_name_pos]);
        arch = (char*)argv[arch_name_pos];
    }

    else
        printf("Invalid archive.\n");

    /* Controls what happens when the flags are recieved*/
    switch (current) {
        case 'q':
            quick_append();
            break;

        case 'x':
            extract();
            break;

        case 't':
            print_table(v);
            break;

        case 'd':
            delete();
            break;

        case 'A':
            append_all();
            break;

        default:
            print_usage();
            break;
    }



    /* -w Extra credit: for a given timeout, add all modified files to the archive. (except the archive itself) */

    return 0;
}



/* -q quickly append named files to archive */
void quick_append(){

    printf("I recieved a q!\n");

}


/* -x extract named files */
void extract(){

    printf("I recieved a x!\n");

}


/* -t print a concise table of contents of the archive */
void print_table(int verbose){

    printf("I recieved a t!\n");

}


/* returns whether or not the table is going to be printed in verbose mode */
int verbose(const char **argv){

    if (argv[1][1] == 't') {

        if (argv[1][2] == 'v')
            return 0;

        if (argv[2][0] == '-')
            if (argv[2][1] == 'v')
                return 1;
    }

    return -1;

}


/* -d delete named files from the archive */
void delete(){

    printf("I recieved a d!\n");

}


/* -A quickly append all “regular” files in the current directory (except the archive itself) */
void append_all(){

    printf("I recieved an A!\n");

}

/* prints out the usage of myar */
void print_usage(){

    printf("usage: ./myar [-qxtvdA] <archivename>.a <files> ... \n\t\t"

           "-q: quickly append named files to archive\n\t\t"
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


int _check_input(int argc, const char *argv[], int arch_name_pos){

    /* No arguments passed */
    if (argc == 1 || argc == 2)
        print_usage();

    /* checks to make sure that the archive has "valid" extention */
    for (int i = 0; i < sizeof(argv[arch_name_pos])/sizeof(char); i++){
        if(argv[arch_name_pos][i] == '.'){
            if (argv[arch_name_pos][i+1] == 'a' && sizeof(argv[arch_name_pos]) <= 15)
                return 1;
            else if(sizeof(argv[arch_name_pos]) > 15){
                printf("File name must be less than 15 charachters."); //this is simply to make things easier for later.
                return 0;
            }
        }
    }

    print_usage();
    return 0;

}

void _append(){}


int _is_ar(){

    return 1;
}

int _is_file_path(){


    return 0;
}
/* this is used to give a default name in case of no archive name being specified? */
void _create_ar(){



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
