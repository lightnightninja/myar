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

/* Standard libraries */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* System call prototypes! */
#include <unistd.h>
#include <signal.h>

/* File information */
#include <sys/stat.h>

/* File Control */
#include <fcntl.h>

/* errno, and error constants */
#include <errno.h>

void print_usage();

int main(int argc, const char * argv[]) {

    if (argc == 1)
        print_usage();


    /* -q quickly append named files to archive */
    /* -x extract named files */
    /* -t print a concise table of contents of the archive */
    /* -v iff specified with -t, print a verbose table of contents of the archive */
    /* -d delete named files from the archive */
    /* -A quickly append all “regular” files in the current directory (except the archive itself) */
    /* -w Extra credit: for a given timeout, add all modified files to the archive. (except the archive itself) */

    return 0;
}

void print_usage(){

    printf("usage:  -q: quickly append named files to archive\n"
           "            myar -q archive file ...\n"

           "        -x: extract named files\n"
           "            myar -x archive [file ...]\n"

           "        -t: print a concise table of contents of the archive\n"
           "        -v: iff specified with -t, print a verbose table of contents of the archive\n"
           "            myar -t [-v] archive [file ...]\n"

           "        -d: delete named files from the archive\n"
           "            myar -d archive file ...\n"

           "        -A: quickly append all “regular” files in the current directory\n"
           "            myar -A archive file ...\n"

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
