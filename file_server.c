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

#include "fileManagement.h"

// DEBUG Macro
//#define DEBUG
#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif

/*
    compile using:
        gcc file_server.c -lpthread -o file_server

    Relative pathing works. Does not make directories.
    sample inputs:
        write test/abacaba.txt abcde
        read test/abacaba.txt
        empty test/text.txt
*/

// locks on files the worker threads use
pthread_mutex_t readTxtLock;    
pthread_mutex_t emptyTxtLock;

int main(int argc, char *argv[]){
    // time based seed for randomization
    srand(time(NULL));
    
    D(printf("[PID: %d]\tMain thread starts.\n", getpid());)
    
    // initialize locks for updating read.txt and write.txt
    // command.txt is only accessed by a main thread so no need to lock it
    pthread_mutex_init(&readTxtLock, NULL);
    pthread_mutex_init(&emptyTxtLock, NULL);

    // loops repeatedly for console input
    // every input forks a master thread, which then spawns appropriate worker threads
    while (1){
        // gets input from console and stores information in a Command struct
        struct Command command;
        command = getCommand();
        //if(command.action=='q') break;
        
        // update command.txt file
        updateCommandsTxt(&command);

        // forks a master thread        
        pid_t pid = fork();

        if(pid<0){
            fprintf(stderr, "Failed to fork. Exiting...\n");
            return 1;
        } else if (pid==0){
            // child calls the worker threads while passing the Command struct
            executeCommand(&command);
            break;
        } else{
            // parent reaps child then repeats while loop
            waitpid(pid,NULL,WNOHANG);
        }
    }
    return 0;
}

// terminates strings at first newline character
void endOnNewline(char *str){
    str[strcspn(str,"\n")]=0;   
}

// copies command string of struct Command to dest
void copyCommand(char* dest, struct Command *command){ 
    switch(command->action){
        case 'w':   sprintf(dest,"write %s %s", command->path, command->str); break;
        case 'r':   sprintf(dest,"read %s", command->path); break;
        case 'e':   sprintf(dest,"empty %s", command->path); break;
        default:    strcpy(dest, "Unrecognized Command");
    }
}

// prints result from copyCommand()
void printCommand(struct Command *command){
    char dest[MAX_INPUT_SIZE];
    copyCommand(dest,command);
    printf("%s\n",dest);
}

// gets and process input from console and returns a struct Command storing the input
struct Command getCommand(){
    // initializes Command struct
    char feed[MAX_INPUT_SIZE];
    struct Command command; char * buffer; int index=0; int flag=0;

    // puts input to feed
    fgets(feed, MAX_INPUT_SIZE, stdin);
    endOnNewline(feed);

    // uses strtok to separate the command parameters and stores them in the struct
    buffer = strtok(feed," ");
    while(buffer!=NULL){
        switch(index){
            case 0:     command.action = buffer[0]; break;
            case 1:     strcpy(command.path, buffer); break;
            case 2:     strcpy(command.str, buffer); break;
            default:    flag = 1;
        }
        if(flag) break;
        buffer = strtok(NULL," ");
        index++;
    }

    // returns the struct
    return command;
}

// appends str string at the end of file in path, creates file if nonexistent
// simple function for file creation/opening and appending
// created because all worker threads call for writing on files
void appendOnFile(char *path, char *str){
    FILE * fp;
    fp = fopen(path,"a");
    fprintf(fp,"%s",str);
    fclose(fp);
}

// copies the current timestamp string to dest
// uses the asctime() formatting: "Www Mmm dd hh:mm:ss yyyy"
void copyTimeStamp(char* dest){ 
    time_t currTime;
    time(&currTime);
    struct tm *lclTime = localtime(&currTime);
    sprintf(dest,"%s", asctime(lclTime));
    endOnNewline(dest);
}

// updates commands.txt based on struct Command
// calls copyTimeStamp() and copyCommand(), then appends the concatenation in command.txt
int updateCommandsTxt(struct Command *command){
    char cmd[MAX_INPUT_SIZE]; char tmstamp[TIMESTAMP_SIZE]; char buffer[MAX_INPUT_SIZE + TIMESTAMP_SIZE + 5];
    copyCommand(cmd,command);
    copyTimeStamp(tmstamp);
    sprintf(buffer, "%s\t%s\n",tmstamp, cmd);
    appendOnFile("commands.txt",buffer);
}

// uses select() to make a thread sleep for milliseconds
void msSleep(int ms){                           
    struct timeval t;
    t.tv_sec  = ms / 1000;
    t.tv_usec = (ms % 1000) * 1000;
    select (0, NULL, NULL, NULL, &t);
}

// returns current time in milliseconds
long long getMsTime() {
    struct timeval t;
    gettimeofday(&t,NULL);
    return (((long long)t.tv_sec)*1000)+(t.tv_usec/1000);
}
/*    long long start, lap;
    start = getMsTime();
    msSleep(201);
    lap = getMsTime();

    D(printf("ms taken: %lld\n",lap-start);)
*/

//  worker thread for write command
void executeWrite(struct Command * command){    

#   ifdef DEBUG
    long long start, lap;
    printf("[PID: %d]\tWrite worker thread created.\n", getpid());
    start = getMsTime();
#   endif

    // "loading" duration, 80% 1s, 20% 6s
    int waitTime = (rand()%10<8) ? 1 : 6;
    sleep(waitTime);

#   ifdef DEBUG
    lap = getMsTime();
    printf("[PID: %d]\tFile Load Duration:\t%llds\n", getpid(), (lap-start)/1000);
    start = getMsTime();
#   endif

    // buffer that stores the characters to write
    char buffer[2] = "\0\0";

    // append stored character to path for every 25ms
    int i;
    for(i=0; command->str[i]!='\0';i++){
        msSleep(250);
        buffer[0]=command->str[i];
        appendOnFile(command->path, buffer);
    }

#   ifdef DEBUG
    lap = getMsTime();
    printf("[PID: %d]\tExpected Write Duration:\t%dms\n", getpid(), i*250);
    printf("[PID: %d]\tActual Write Duration:\t\t%lldms\n", getpid(), lap-start);
    printf("[PID: %d]\tWrite worker thread closes.\n", getpid());
#   endif

}

// worker thread for read command
void executeRead(struct Command * command){     
    
#   ifdef DEBUG
    long long start, lap;
    printf("[PID: %d]\tRead worker thread created.\n", getpid());
    start = getMsTime();
#   endif

    // "loading" duration, 80% 1s, 20% 6s
    int waitTime = (rand()%10<8) ? 1 : 6;
    sleep(waitTime);
    
#   ifdef DEBUG
    lap = getMsTime();
    printf("[PID: %d]\tFile Load Duration:\t%llds\n", getpid(), (lap-start)/1000);
    start = getMsTime();
#   endif

    // print "<entire command>:" in read.txt
    char dest[MAX_INPUT_SIZE];
    copyCommand(dest,command);
    appendOnFile("read.txt",dest);
    appendOnFile("read.txt",":\t");

    // open file from path
    FILE * fp;
    
    fp = fopen(command->path,"r");

    // lock read.txt
    pthread_mutex_lock(&readTxtLock);
    
    // if file can't be opened, append "FILE DNE" to read.txt then exit
    if (fp == NULL){    
        appendOnFile("read.txt","FILE DNE\n");
        fclose(fp); return;
    } else{

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
    }
    // unlock read.txt
    pthread_mutex_unlock(&readTxtLock);

#   ifdef DEBUG
    lap = getMsTime();
    printf("[PID: %d]\tExpected Read Duration:\t0ms\n", getpid());
    printf("[PID: %d]\tActual Read Duration:\t\t%lldms\n", getpid(), lap-start);
    printf("[PID: %d]\tRead worker thread closes.\n", getpid());
#   endif

    return;
}

// worker thread for empty command
void executeEmpty(struct Command * command){     
    
#   ifdef DEBUG
    long long start, lap;
    printf("[PID: %d]\tEmpty worker thread created.\n", getpid());
    start = getMsTime();
#   endif

    // "loading" duration, 80% 1s, 20% 6s
    int waitTime = (rand()%10<8) ? 1 : 6;
    sleep(waitTime);
    
#   ifdef DEBUG
    lap = getMsTime();
    printf("[PID: %d]\tFile Load Duration:\t%llds\n", getpid(), (lap-start)/1000);
    start = getMsTime();
#   endif

    // lock empty.txt
    pthread_mutex_lock(&emptyTxtLock);

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
        fclose(fp); 

#       ifdef DEBUG
        lap = getMsTime();
        printf("[PID: %d]File already empty.\n", getpid());
#       endif

    } else{      
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

#       ifdef DEBUG
        lap = getMsTime();
        printf("[PID: %d]\tFile emptying duration:\t%llds\n", getpid(),(lap-start)/1000);
#       endif

    }
    // unlock empty.txt
    pthread_mutex_unlock(&emptyTxtLock);

#   ifdef DEBUG
    printf("[PID: %d]\tEmpty worker thread closes.\n", getpid());
#   endif

    return;
}

// spawns the worker threads, called by the master thread which is the forked child in main
// uses switch-case on the stored command, then creates respective worker thread
void executeCommand(struct Command * command){      

    D(printf("[PID: %d]\tMaster thread forked from main.\n", getpid());)

    pthread_t worker;
    switch (command->action){
        case 'w':
            D(printf("[PID: %d]\tMaster thread spawns a write thread.\n", getpid());)
            pthread_create(&worker, NULL, (void *) executeWrite, command);
            pthread_join(worker, NULL);
            break;
        case 'r':
            D(printf("[PID: %d]\tMaster thread spawns a read thread.\n", getpid());)
            pthread_create(&worker, NULL, (void *) executeRead, command);
            pthread_join(worker, NULL);
            break;
        case 'e':
            D(printf("[PID: %d]\tMaster thread spawns an empty thread.\n", getpid());)
            pthread_create(&worker, NULL, (void *) executeEmpty, command);
            pthread_join(worker, NULL);
            break;
        default: break;
    }
    
    D(printf("[PID: %d]\tMaster thread closes.\n", getpid());)
}


