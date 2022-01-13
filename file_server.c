#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "commandManagement.h"

#define BUFFER_SIZE 100

void msSleep(int ms){                           //  uses select() to make a thread sleep for subsecond durations
    struct timeval t;
    t.tv_sec  = ms / 1000;
    t.tv_usec = (ms % 1000) * 1000;
    select (0, NULL, NULL, NULL, &t);
}

void executeWrite(struct Command * command){    //  worker thread for write command
    
    // "loading" duration, 80% 1s, 20% 6s
    int waitTime = (rand()%10<8) ? 1 : 6;
    sleep(waitTime);

    // buffer that stores the characters to write
    char buffer[2] = "\0\0";

    // append stored character to path for every 25ms
    for(int i=0; command->str[i]!='\0';i++){
        msSleep(250);
        buffer[0]=command->str[i];
        appendOnFile(command->path, buffer);
    }
}

void executeRead(struct Command * command){     // worker thread for read command
    
    // "loading" duration, 80% 1s, 20% 6s
    int waitTime = (rand()%10<8) ? 1 : 6;
    sleep(waitTime);

    // print "<entire command>:" in read.txt
    char dest[MAX_INPUT_SIZE];
    copyCommand(dest,command);
    appendOnFile("read.txt",dest);
    appendOnFile("read.txt",":\t");

    // open file from path
    FILE * fp;
    fp = fopen(command->path,"r");

    // if file can't be opened, append "FILE DNE" to read.txt then exit
    if (fp == NULL){    
        appendOnFile("read.txt","FILE DNE\n");
        fclose(fp); return;
    }
        
    // buffer that stores BUFFER_SIZE characters
    char buffer[BUFFER_SIZE+1] = {0};
    
    // while loop through contents of the file and append them to read.txt
    int num_read;
    while((num_read=fread(buffer,1,BUFFER_SIZE,fp))==BUFFER_SIZE){
        appendOnFile("read.txt", buffer);
    }
    // null terminate the end of file before appending to read.txt
    buffer[num_read]='\0';
    appendOnFile("read.txt", buffer);
    appendOnFile("read.txt", "\n");
    fclose(fp);
    
    return;
}

void executeEmpty(struct Command * command){     // worker thread for empty command
    
    // "loading" duration, 80% 1s, 20% 6s
    int waitTime = (rand()%10<8) ? 1 : 6;
    sleep(waitTime);

    // print "<entire command>:" in empty.txt
    char dest[MAX_INPUT_SIZE];
    copyCommand(dest,command);
    appendOnFile("empty.txt",dest);
    appendOnFile("empty.txt",":\t");

    // open file from path
    FILE * fp;
    fp = fopen(command->path,"r");

    // if file can't be opened, append "FILE DNE" to empty.txt then exit
    if (fp == NULL){
        appendOnFile("empty.txt","FILE ALREADY EMPTY\n"); 
        fclose(fp); return;
    }
        
    // buffer that stores BUFFER_SIZE characters
    char buffer[BUFFER_SIZE+1] = {0}; int num_read;

    // while loop through contents of the file and append to empty.txt
    while((num_read=fread(buffer,1,BUFFER_SIZE,fp))==BUFFER_SIZE){
        appendOnFile("empty.txt", buffer);
    }
    // null terminate the end of file before appending to empty.txt
    buffer[num_read]='\0';
    appendOnFile("empty.txt", buffer);
    appendOnFile("empty.txt", "\n");
    fclose(fp);

    // "emptying" duration, between 7 and 10 whole seconds
    waitTime = (rand()%4)+7;
    sleep(waitTime);
    
    // overwriting the file with an empty file
    fp = fopen(command->path,"w");
    fclose(fp);
    
    return;
}

void executeCommand(struct Command * command){      // master thread, spawns the worker threads
    pthread_t worker;
    switch (command->action){
        case 'w':
            pthread_create(&worker, NULL, (void *) executeWrite, command);
            pthread_join(worker, NULL);
            break;
        case 'r':
            pthread_create(&worker, NULL, (void *) executeRead, command);
            pthread_join(worker, NULL);
            break;
        case 'e':
            pthread_create(&worker, NULL, (void *) executeEmpty, command);
            pthread_join(worker, NULL);
            break;
        default: break;
    }
}

int main(int argc, char *argv[]){
    // time based seed for randomization
    srand(time(NULL));

    // performs similar to a shell
    // loops repeatedly for console input
    // every input forks a master thread, which then spawns appropriate worker threads
    while (1){
        struct Command command;
        command = getCommand();
        if(command.action=='q') break;
        updateCommandsTxt(&command);

        // forks a master thread        
        pid_t pid = fork();

        if(pid<0){
            fprintf(stderr, "Failed to fork. Exiting...\n");
            return 1;
        } else if (pid==0){
            // child is a master thread that calls worker threads
            executeCommand(&command);
            break;
        } else{
            // parent repeats while loop
            wait(NULL);
        }
    }
    return 0;
}

/*  Directoried paths work.
    sample inputs:
        write test/abacaba.txt abcde
        read test/abacaba.txt
        empty test/text.txt
*/