#define ACT_SIZE 6              // max size of possible command inputs
#define ARG_SIZE 51             // max size of both path and string inputs
#define TIMESTAMP_SIZE 20       // max size of time stamp string

#define MAX_INPUT_SIZE ACT_SIZE+(2*ARG_SIZE)+2  

struct Command{     // struct that stores the inputted information
    char action;            // 'w', 'r', 'e' corresponds to write, read and empty commands respectively
    char path[ARG_SIZE];    // stores the null terminated path from input
    char str[ARG_SIZE];     // stores the null terminated string frpm input for write command
};

void endOnNewline(char *str);                               // terminates strings at first newline character
struct Command getCommand(void);                            // gets and process input from console and returns a struct Command storing the input
void copyCommand(char* dest, struct Command *command);      // copies command string of struct Command to dest
void printCommand(struct Command *command);                 // prints result from printCommand()
void appendOnFile(char *path, char *str);                   // appends str string at the end of file in path, creates file if nonexistent
void copyTimeStamp(char* dest);                             // copies the current timestamp string to dest
int updateCommandsTxt(struct Command *command);             // updates commands.txt based on struct Command

void endOnNewline(char *str){
    str[strcspn(str,"\n")]=0;
}

void copyCommand(char* dest, struct Command *command){ 
    switch(command->action){
        case 'w':   sprintf(dest,"write %s %s", command->path, command->str); break;
        case 'r':   sprintf(dest,"read %s", command->path); break;
        case 'e':   sprintf(dest,"empty %s", command->path); break;
        default:    strcpy(dest, "Unrecognized Command");
    }
}

void printCommand(struct Command *command){
    char dest[MAX_INPUT_SIZE];
    copyCommand(dest,command);
    printf("%s\n",dest);
}

struct Command getCommand(){
    char feed[MAX_INPUT_SIZE];
    struct Command command; char * buffer; int index=0; int flag=0;

    fgets(feed, MAX_INPUT_SIZE, stdin);
    endOnNewline(feed);
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
    return command;
}


void appendOnFile(char *path, char *str){
    FILE * fp;
    fp = fopen(path,"a");
    fprintf(fp,"%s",str);
    fclose(fp);
}

void copyTimeStamp(char* dest){ 
    time_t currTime;
    time(&currTime);
    struct tm *lclTime = localtime(&currTime);
    sprintf(dest,"%s", asctime(lclTime));
    endOnNewline(dest);
}

int updateCommandsTxt(struct Command *command){
    char cmd[MAX_INPUT_SIZE]; char tmstamp[TIMESTAMP_SIZE]; char buffer[MAX_INPUT_SIZE + TIMESTAMP_SIZE + 5];
    copyCommand(cmd,command);
    copyTimeStamp(tmstamp);
    sprintf(buffer, "%s\t%s\n",tmstamp, cmd);
    appendOnFile("commands.txt",buffer);
}
