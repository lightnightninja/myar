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
#define EMPTYSPACE 128
#ifndef AR_HDR_SIZE
#define AR_HDR_SIZE 60
#endif
#define endl printf("\n")

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
int     delete(int, char **, int, int); // handles -d
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

    //Trying to set everyting to rw-rw-rw
    mode_t perms = S_IXUSR | S_IXGRP | S_IXOTH;
    umask(perms);

    /* ensuring correct input, and that archive recieves .a extention */
    if (argv[1][1] == 't') {
        v = _verbose(argv);
        if (v == 1)
            arch_name_pos++;
        if (v == 0)
            v = 1;
    }
    if(_input(argc, argv, arch_name_pos) == 1) {
        //printf("Using archive: %s\n", argv[arch_name_pos]);//debug
        arch = (char*)argv[arch_name_pos];
        choice = argv[1][1];
        file_count = argc - arch_name_pos - 1;
        arch_fd = open(arch, O_RDWR);
    }
    else
        fprintf(stderr, "Invalid archive name.\n");

    arch_fd = is_ar(arch_fd, arch, choice);
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
                        if (append((char *)argv[file_pos], arch_fd, file_fd) == -1)
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
            break;

        case 'd':
            if(delete(argc, (char **)argv, arch_fd, file_count) == -1)
                return -1;
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

//this used to be a struct ar * type, but I realized tehre was no point
char *_create_header(int file_fd, char *file_name){

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

    return (char*)header;
}
/* -q quickly append named files to archive */
int append(char *file_name, int arch_fd, int file_fd){

    /*Attempt to change the perms of the file to rw-rw-rw- */

    //mode_t perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    //fchmod(file_fd, perms);

    char *buffer = malloc(sizeof(struct ar_hdr));
    buffer = _create_header(file_fd, file_name);
    size_t bytes_read;
    for (int i = 0; i < AR_HDR_SIZE; i++) {
        if (buffer[i] == 0) {
            buffer[i] = ' ';
        }
    }
    /* This allows to differentiate the boundaries between files, making searching easy */



    /* Ensure we are starting on an even boundry */
    int evenboundry;
    char newline[] = {10};//newline char

    evenboundry = lseek(arch_fd, 0, SEEK_END) % 2;
    if (evenboundry != 0) {
        if(write(arch_fd, newline, 1) == -1)
            return -1;
    }
    /* Places the header in the file. */
    if(write(arch_fd, (char *)buffer, sizeof(struct ar_hdr)) == -1)
        return -1;
    /* Transfers data from file to archive */
    while ((bytes_read = read(file_fd, buffer, BSIZE)) > 0) {
        if (write(arch_fd, buffer, bytes_read) != bytes_read) { //checking to make sure we don't have a write error
            return -1;
        }
    }
    return 1;

}


/* -x extract named files */
void extract(){

    printf("I recieved a x!\n");

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
char *_perms(const char *p){

    /* code used from example at: http://codewiki.wikidot.com/c:system-calls:stat
     * It uses a 'ternary if' statement to determine if the file has a - or r/w/x
     * it's simliar to asking the if statement (is x > y?)
     * x : y is the quivalent of the statement evaluating as true, and the y is false
     * However, this can lead to bad things if it doesn't evaluate correctly, but
     * for this purpose, it just shortens the code a bit, rather than a bunch of
     * if and else statements.
     */
    mode_t octal = (mode_t)strtol(p, NULL, 8);
    //this converts it to octal and needes mode_t in order to be able to compare with the different types)
    static char perm[10]; //it yelled at me when it wasn't static.
    perm[0] =  *((octal & S_IRUSR) ? "r" : "-");
    perm[1] =  *((octal & S_IWUSR) ? "w" : "-");
    perm[2] =  *((octal & S_IXUSR) ? "x" : "-");
    perm[3] =  *((octal & S_IRGRP) ? "r" : "-");
    perm[4] =  *((octal & S_IWGRP) ? "w" : "-");
    perm[5] =  *((octal & S_IXGRP) ? "x" : "-");
    perm[6] =  *((octal & S_IROTH) ? "r" : "-");
    perm[7] =  *((octal & S_IWOTH) ? "w" : "-");
    perm[8] =  *((octal & S_IXOTH) ? "x" : "-");
    perm[9] = ' ';

    return perm;
}
char *whats_the_time(long time){
    char *bla = malloc(26);
    strftime(bla, 26, "%b %d %R %Y", localtime(&time));
    return bla;
}
/* Used to find headers, had to be totally reimplemented :( */
int seek_data(int arch_fd, int empty){

    off_t bytes_read;
    char buffer[empty*2];

    if((bytes_read = read(arch_fd, buffer, empty*2)) < 60)
         return -1;


    //printf("bytes read from seek_data = %lli\n", bytes_read); //debug
    for (int i = 0; i < bytes_read; i++) {

        if (buffer[i] != 0 && buffer[i] != 10) {
            if(buffer[i+58] == '`' && buffer[i+59] == '\n') { //ARFMAG
                //printf("Found header, ARFMAG was found at %i.\n", i);//debug
                return i;
            }
        }
    }
    return 0;//didn't find header
}

/* Goes and fetches the header information*/
off_t go_fetch(off_t offset, int arch_fd, struct ar_hdr *header){

    char buffer[AR_HDR_SIZE]; //no malloc because we are just storing stuff temp
    off_t check;

    while((check = seek_data(arch_fd, EMPTYSPACE)) != -1){
        offset += check; //updating the place in file.

        //printf("Check = %i\n", offset);
        if ((check = lseek(arch_fd, offset, SEEK_SET)) == 0) //Finds new place based on offset
            return -1;
        if (check == -1)
            return -1;
        if((check = read(arch_fd, buffer, AR_HDR_SIZE)) != -1){
            memcpy(header, buffer, AR_HDR_SIZE);
            return 1;
        }
        else
            return -1;

    }
    return -1;
    
}
/* -t print a concise table of contents of the archive */
void print_table(int verbose, int arch_fd){

    struct  ar_hdr *header = malloc(sizeof(struct ar_hdr)); //we have to malloc because the info needs updated in go_fetch
    off_t offset = 0;
    offset =lseek(arch_fd, SARMAG, SEEK_SET);
    //printf("verbose = %i\n", verbose);
    /* go fetch all of the names */
    while (go_fetch(offset, arch_fd, header) != -1) {
        //printf("offset1 = %lli\n", offset);
        if(verbose == -1){
            fwrite(header->ar_name, 1, 16, stdout); //kinda hacky(?) way around the '\0'
            printf("\n");
        }

        if (verbose == 1){
            printf("%-12s", _perms(header->ar_mode));
            printf("%-6ld/", atol(header->ar_uid));
            printf("%-6ld", atol(header->ar_gid));
            printf("%-10lld", atoll(header->ar_size));
            printf("%-26s", whats_the_time(atol(header->ar_date)));
            fwrite(header->ar_name, 1, 16, stdout);

            printf("\n");
        }

        offset = lseek(arch_fd, atoll(header->ar_size), SEEK_CUR);

    }
    free(header);
}


/* -d delete named files from the archive */
int delete(int argc, char **argv,  int arch_fd, int file_count){

    //needs to take in names of files from argv, so pass argv argc
    unlink("temp.a");
    //create temp archive
    int temp_fd;
    if ((temp_fd = open("temp.a", O_RDWR)) == -1) {
        if((temp_fd = _create_ar("temp.a", temp_fd)) == -1)
            return -1;
    }
    struct ar_hdr *header = malloc(sizeof(struct ar_hdr));
    off_t old_offset = 0;
    off_t new_offset;
    off_t offset;
    int len = 0;
    int something = 0;
    char name[16];
    char buffer[BSIZE];

    size_t bytes_read = 0;
    size_t to_read = BSIZE;
    int count;
    int thing = file_count;

    int evenboundry;
    char newline[] = {10};//newline char

    old_offset = lseek(arch_fd, SARMAG, SEEK_SET);
    new_offset = lseek(temp_fd, SARMAG, SEEK_SET);


    char **files = malloc(file_count);
    printf("Files to remove: ");
    endl;

    for (int i = 0; i < file_count; i++) {
        files[i] = argv[i+3]; //
        printf("%s\n", files[i]);
    }

    while (go_fetch(old_offset, arch_fd, header) != -1) {//while it's not the end of the file

        if (strncmp(header->ar_fmag, ARFMAG, 2) != 0) {
            break;
        }

            printf("found a header: %s\n", header->ar_name);
            printf("temp_fd = %i\n", temp_fd);
            something = 0;
            printf("temp_fd = %i\n", new_offset);
            new_offset = lseek(temp_fd, new_offset, SEEK_SET);

            printf("temp_offset1 = %i\n", new_offset);

            old_offset = lseek(arch_fd, 0, SEEK_CUR);
            printf("offset = %lli\n", old_offset);
            //find the headers, I need to check to see if it matches any of the files.
            if (thing != 0) {

                for (int i = 0; i < file_count ; i++) {
                    if (files[i] != NULL ) {
                        len = sizeof(files[i])/sizeof(char);//accounting for newline
                        memcpy(name, header->ar_name, len);//copying so we can check just the name

                        if(strncmp(name, files[i], len) == 0){//just want to compare len bytes
                            old_offset += 60;
                            offset = lseek(arch_fd, old_offset, SEEK_SET); //skip that shit

                            printf("old offset = %lli\n", offset);

                            files[i] = NULL;
                            thing--;
                            endl;
                            printf("woo, I can code!\n");
                            endl;
                            endl;
                            something = 1;

                        }

                    }
                //if they are the same, don't copy over
                }

            }
            /* Ensure we are starting on an even boundry */
            if (something != 1) {
                new_offset = lseek(temp_fd, new_offset, SEEK_SET);

                printf("temp_offset = %i\n", new_offset);
                //evenboundry = new_offset % 2;

               // if (evenboundry != 0) {
                 //   if(write(temp_fd, newline, 1) == -1)
                //        return -1;
               //      new_offset++;
                //}


                /* Places the header in the file. */
                if((new_offset += write(temp_fd, (char *)header, sizeof(struct ar_hdr))) == -1)
                    return -1;
                /* Transfers data from file to archive */
                old_offset = lseek(arch_fd, 0, SEEK_CUR);

                printf("old offset = %lli\n", old_offset);
                count = 0;
                while (count < atoll(header->ar_size)) {
                    if (atoll(header->ar_size) - count <= bytes_read) {
                        to_read = atoll(header->ar_size) - count;
                    }
                    bytes_read = read(arch_fd, buffer, to_read);
                    if (write(temp_fd, buffer, bytes_read) != bytes_read) { //checking to make sure we don't have a write error
                        return -1;
                    }

                    count += bytes_read;

                }

                new_offset += count;
                lseek(arch_fd, old_offset+count, SEEK_SET);
                printf("bytes read: %i\n", count);
                old_offset = lseek(arch_fd, 0, SEEK_CUR);
            }
            //if they are not the same, copy
            endl;
        }
        free(header);
        close(temp_fd);
        //rename("temp.a", argv[2]);
        //unlink("temp.a");
        //printf("unlinked\n");

        //when the end of file is hit, unlink old archive
        //rename current archive
        //close file

    return 1;
}


/* -A quickly append all “regular” files in the current directory (except the archive itself) */
int append_all(char *arch_name, int arch_fd){

    DIR *cur = opendir(".");
    struct dirent *file;
    int file_fd;

    /* Following code modified (totally changed) from:
     * http://pubs.opengroup.org/onlinepubs/7908799/xsh/readdir.html
     */
    while ((file = readdir(cur)) != NULL) {
        if ((file->d_type == DT_REG) && strcmp(file->d_name, arch_name) != 0 && strcmp(file->d_name, "myar") != 0 && strcmp(file->d_name, "myar.c") != 0) { //checking that we aren't looking at our arch, doesn't add itself, or it's executable.
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
            //printf("Valid archive file.\n");//debug
            return fd;
        }

    }

    return -1;
}

int _create_ar(char *file_name, int fd){


    int flags = (O_RDWR | O_CREAT);


    if ((fd = open(file_name, flags, 0666)) == -1)
        return -1;

    write(fd, ARMAG, SARMAG);
    printf("Created: %s\n", file_name);//debug

    return fd;

}

/* Todo */
/* Extract
 * * deal with atime and mtime
 * Delete
 * extra credit?
 * write up
 */


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
