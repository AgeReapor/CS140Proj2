#define ACT_SIZE 6              // max size of possible command actions ("write", "read", "empty")
#define ARG_SIZE 51             // max size of both path and string inputs
#define TIMESTAMP_SIZE 20       // max size of time stamp string
#define BUFFER_SIZE 100         // size of the buffer used for file reading

#define MAX_INPUT_SIZE ACT_SIZE+(2*ARG_SIZE)+2  

struct Command{     // struct that stores the inputted information
    char action;            // 'w', 'r', 'e' corresponds to write, read and empty commands respectively
    char path[ARG_SIZE];    // stores the null terminated path from input
    char str[ARG_SIZE];     // stores the null terminated string frpm input for write command
};

// Helper Functions
void msSleep(int ms);                                       // uses select() to make a thread sleep for milliseconds
long long getMsTime();                                      // returns current time in milliseconds
void endOnNewline(char *str);                               // terminates strings at first newline character

// Command Management Functions
struct Command getCommand(void);                            // gets and processes input from console and returns a struct Command storing the input
void copyCommand(char* dest, struct Command *command);      // copies command string of struct Command to dest
void printCommand(struct Command *command);                 // prints result from copyCommand()
void appendOnFile(char *path, char *str);                   // appends str string at the end of file in path, creates file if nonexistent
void copyTimeStamp(char* dest);                             // copies the current timestamp string to dest
int updateCommandsTxt(struct Command *command);             // updates commands.txt based on struct Command

// Thread Management Functions
void executeWrite(struct Command * command);                // worker thread for write command
void executeRead(struct Command * command);                 // worker thread for read command
void executeEmpty(struct Command * command);                // worker thread for empty command
void executeCommand(struct Command * command);              // spawns the worker threads, called by the master thread which is the forked child in main