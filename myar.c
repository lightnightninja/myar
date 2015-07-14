/********************************************************
**  myar.c
**
**  Created by Cierra Shawe on 7/8/15.
**  Copyright (c) 2015 Cierra Shawe. All rights reserved.
*********************************************************/

/* Defines */
#define _POSIX_C_SOURCE 200809L
#define _BSD_SOURCE
#define BSIZE 256 // Default buffer size
#define EMPTYSPACE 1024

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
/* 
 * Information on how to use this found at:
 * http://www.mit.edu/afs.new/athena/system/rs_aix32/os/usr/include/ar.h 
 */
#include <ar.h>

/* function declarations */
void    print_usage(); //prints out how to use program
int     append(char *, int, int); // handles -q
void    extract(); // handles -x
void    print_table(int, int);
void    delete(); // handles -d
int     append_all(char *, int); // handles -A
int     is_ar(int, char *, char);
//void w(); //EXTRA CREDIT

int     _verbose(const char **); // handles -v
int     _input(int, const char **, int);
int     _create_ar(char *, int);
int     _is_file_path();


int main(int argc, const char *argv[]) {

    int     choice;
    int     file_count = 0;
    int     v = 0;
    int     arch_name_pos = 2;
    int     file_pos;

    char    *arch = NULL;
    int     arch_fd;
    int     file_fd;

    /* ensuring correct input, and that archive recieves .a extention */
    if (argv[1][1] == 't') {
        v = _verbose(argv);
        if (v == 1)
            arch_name_pos++;
        if (v == 0)
            v = 1;
    }
    if(_input(argc, argv, arch_name_pos) == 1) {
        printf("Using archive: %s\n", argv[arch_name_pos]);//debug
        arch = (char*)argv[arch_name_pos];
        choice = argv[1][1];
        file_count = argc - arch_name_pos - 1;
        arch_fd = open(arch, O_RDWR);
        printf("file count: %i\n", file_count);//debug
    }
    else
        fprintf(stderr, "Invalid archive name.\n");

    arch_fd = is_ar(arch_fd, arch, choice);
    printf("fd = %i\n", arch_fd);
    if (arch_fd == -1) {
        print_usage();
        exit(EXIT_FAILURE);
    }

    file_pos = arch_name_pos + 1;

    /* Controls what happens when the flags are recieved*/
    switch (choice) {
        case 'q':
            if (file_count != 0) {
                for (int i = 0; i < file_count; i++, file_pos++) {
                    if ((file_fd = open(argv[file_pos], O_RDONLY)) != -1) {
                        if (append(arch, arch_fd, file_fd) == -1)
                            exit(EXIT_FAILURE);
                        close(file_fd);
                    }
                    else{
                        fprintf(stderr, "Unable to append %s.\n", argv[file_pos]);
                        exit(EXIT_FAILURE);
                    }
                }
            }
            break;

        case 'x':
            extract();
            break;

        case 't':
            print_table(v, arch_fd);
            close(arch_fd);
            break;

        case 'd':
            delete();
            break;

        case 'A':
            if (file_count == 0) {
                if(append_all(arch, arch_fd) == -1)
                   fprintf(stderr, "Something is wrong...\n");
                close(file_fd);
            }
            else
                fprintf(stderr, "Please only enter the archive name, no files.\n");
            break;

        default:
            print_usage();
            exit(EXIT_FAILURE);
    }



    if (close(arch_fd) == -1) {
        printf("There was an issue with closing the file... Hopefully everything is okay.\n");
    }

    /* -w Extra credit: for a given timeout, add all modified files to the archive. (except the archive itself) */

    return 0;
}


struct ar_hdr *_create_header(int file_fd, char *file_name){

    struct ar_hdr *header = malloc(sizeof(struct ar_hdr));
    struct stat stats;

    lstat(file_name, &stats);

    snprintf(header->ar_name, sizeof(header->ar_name), "%-16s",  file_name);
    snprintf(header->ar_date, sizeof(header->ar_date), "%-12ld", (long)stats.st_mtime);
    snprintf(header->ar_uid,  sizeof(header->ar_uid),  "%-6ld",  (long)stats.st_uid);
    snprintf(header->ar_gid,  sizeof(header->ar_gid),  "%-6ld",  (long)stats.st_gid);
    snprintf(header->ar_mode, sizeof(header->ar_mode), "%-8o",   stats.st_mode);
    snprintf(header->ar_size, sizeof(header->ar_size), "%-10lld",(long long)stats.st_size);
    snprintf(header->ar_fmag, sizeof(ARFMAG), "%s", ARFMAG);

    return header;
}
/* -q quickly append named files to archive */
int append(char *file_name, int arch_fd, int file_fd){

    struct ar_hdr *buffer = malloc(sizeof(struct ar_hdr));
    buffer = _create_header(file_fd, file_name);
    size_t bytes_read;

    /* This allows to differentiate the boundaries between files, making searching easy */

    char empty[EMPTYSPACE]; // I just realized this could be done using the offset in lseek...
    for (int i = 0; i < EMPTYSPACE; i++) {
        empty[i] = 0;
    }
    char newlines[]={10, 10, 10, 10};

    /* Ensure we are starting on an even boundry */
    int evenboundry;
    char blank[] = {0};
    evenboundry = lseek(arch_fd, 0, SEEK_END) % 4;
    if (evenboundry != 0) {
        for (int i = 4-evenboundry; i > 0; i--) {
            if(write(arch_fd, blank, 1) == -1)
                return -1;
        }
    }
    if(write(arch_fd, empty, EMPTYSPACE) == -1)
        return -1;

    /* Places the header in the file. */
    if(write(arch_fd, (char *)buffer, sizeof(struct ar_hdr)) == -1)
        return -1;

    /* Transfers data from file to archive */
    while ((bytes_read = read(file_fd, buffer, BSIZE)) > 0) {
        if (write(arch_fd, buffer, bytes_read) != bytes_read) { //checking to make sure we don't have a write error
            return -1;
        }
    }
    write(arch_fd, newlines, 4);
    return 1;

}


/* -x extract named files */
void extract(){

    printf("I recieved a x!\n");

}

/* Goes and fetches the header information*/
off_t go_fetch(off_t offset, int arch_fd){

    char *buffer = malloc(sizeof(struct ar_hdr));
    off_t arch_size;
    size_t bytes_read;

    if ((arch_size = lseek(arch_fd, 0, SEEK_END)) == -1) //Find archive size
        return -1;
    while (bytes_read < arch_size) {
        ;
    }


    return -1;

}
/* returns whether or not the table is going to be printed in verbose mode */
int _verbose(const char **argv){

    if (argv[1][1] == 't') {

        if (argv[1][2] == 'v')
            return 0;

        if (argv[2][0] == '-')
            if (argv[2][1] == 'v')
                return 1;
    }
    
    return -1;
    
}

void _file_perms(){

    /* code used from example at: http://codewiki.wikidot.com/c:system-calls:stat
     * It uses a 'ternary if' statement to determine if the file has a - or r/w/x
     * it's simliar to asking the if statement (is x > y?)
     * x : y is the quivalent of the statement evaluating as true, and the y is false
     * However, this can lead to bad things if it doesn't evaluate correctly, but
     * for this purpose, it just shortens the code a bit, rather than a bunch of
     * if and else statements.
     */

}
/* -t print a concise table of contents of the archive */
void print_table(int verbose, int arch_fd){

    struct  ar_hdr *temp = malloc(sizeof(struct ar_hdr)); //storing the new header in temp, so we can access stuffs.
    off_t offset;

    lseek(arch_fd, SARMAG, SEEK_SET);

    offset = lseek(arch_fd, BSIZE, SEEK_DATA);


    /* go fetch all of the names */
    while (go_fetch(offset, arch_fd) != -1) {
        printf("%s\n", temp->ar_name);
    }
    printf("I recieved a t!\n");//debug
    free(temp);

}


/* -d delete named files from the archive */
void delete(){

    printf("I recieved a d!\n");

}


/* -A quickly append all “regular” files in the current directory (except the archive itself) */
int append_all(char *arch_name, int arch_fd){

    DIR *cur = opendir(".");
    struct dirent *file;
    int file_fd;

    /* Following code modified from: http://pubs.opengroup.org/onlinepubs/7908799/xsh/readdir.html*/
    while ((file = readdir(cur)) != NULL) {
        if ((file->d_type == DT_REG) && strcmp(file->d_name, arch_name) != 0 && strcmp(file->d_name, ".DS_Store") != 0 && strcmp(file->d_name, "myar.c") != 0) { //checking that we aren't looking at our arch
            if((file_fd = open(file->d_name, O_RDONLY)) != -1){
                append(file->d_name, arch_fd, file_fd);
                close(file_fd);
            }

            else
               fprintf(stderr, "Error opening file: %s\n", file->d_name);
        }

    }

    if (closedir(cur) == -1){
        fprintf(stderr, "Couldn't close the directory...\n");
        return -1;
    }

    return 1;

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

int _input(int argc, const char *argv[], int arch_name_pos){

    /* No arguments passed */
    if (argc == 1 || argc == 2)
        return 0;

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

int is_ar(int fd, char *arch, char c){

    char buffer[SARMAG]; //this is used to see if the header on the archive is correct
    
    if (fd == -1 && (c == 'q' || c == 'A')) {
        fd = _create_ar(arch, fd);
    }
    else if (fd == -1){
        fprintf(stderr, "Error finding archive. Please enter a valid name.\n");
        return -1;
    }

    if(lseek(fd, 0, SEEK_SET) != 0){ //if the seek was unsucessful, somethign is wrong
        fprintf(stderr, "Something isn't right... Please check the archive file. \n");
        return -1;
    }

    if(read(fd, buffer, SARMAG) == SARMAG){
        if (strncmp(ARMAG, buffer, SARMAG) == 0){
            printf("Valid archive file.\n");//debug
            return fd;
        }

    }

    return -1;
}

int _create_ar(char *file_name, int fd){


    int flags = (O_RDWR | O_CREAT | O_EXCL);

    printf("The FD is: %i\n", fd);//debug
    mode_t perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    fchmod(fd, perms);

    if ((fd = open(file_name, flags, perms)) == -1)
        return -1;

    write(fd, ARMAG, SARMAG);
    printf("The FD is: %i\n", fd);//debug

    return fd;

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
