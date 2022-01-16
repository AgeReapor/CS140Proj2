Adrian Joshua M. Reapor
CS 140 Project 2 (January 14, 2022)

Implementation Level: 3
	- Continuously recieve user requests, and dispatches worker threads to execute them concurrently.
	- Data races can occur on files accessed by the commands.

=== Dependencies ===
OS Version Used: WSL Version 2

=== Setup ===
compilation: $ gcc file_server.c -lpthread -o file_server
executable: ./file_server

=== Included Files ===
1. file_server.c
	- Main executable that runs the file server processes.

2. fileManagement.h
	- Stores all the declarations and function prototypes used by file_server.c

3. commands.txt
	- Initially an empty file.
	- Is updated for every command accepted by the file server.

4. read.txt
	- Initially an empty file.
	- Is updated for every read command accepted by the file server.
	
5. empty.txt
	- Initially an empty file.
	- Is updated for every empty command accepted by the file server.

6. test.txt (Optional)
	- A text file with the command inputs used for testing the file server. 

7. test (Optional)
	- A directory used by test.txt inputs.

=== How To Use ===
1. Start by "./file_server" in the repository directory.
2. The console now endlessly accepts inputted commands separated by line breaks. The inputs are expected to be in the following formatting:
	"write <path/to/file> <string>"
		- Appends the <string> to the file in <path/to/file>.
	
		- Creates <path/to/file> if it is nonexistent, but does not create directories.
	
	"read <path/to/file>"
		- Scans the contents of the file in <path/to/file>,
		then appends them in read.txt in the format "<entire_command>: <file_contents><newline>"
		
		- If <path/to/file> is nonexistent, appends "<entire_command>: FILE DNE<newline>" in read.txt instead.
	
	"empty <path/to/file>"
		- Scans the contents of the file in <path/to/file>,
		then appends them in empty.txt in the format "<entire_command>: <file_contents><newline>",
		before emptying the file in <path/to/file>.
		
		- If <path/to/file> is nonexistent, appends "<entire_command>:  FILE ALREADY EMPTY<newline>" in empty.txt instead.

Additional Notes:
	-The <path/to/file> and <string> inputs can take in strings of up to 50 non-whitespace characters.
	-The <path/to/file> uses a relative pathing. That is, for the <path/to/file> string "<folder_name1>/<folder_name2>/<file_name>", the <folder_name1> must be a folder in the directory where file_server is run.
	-The file server does not translate escape sequences, that is the input string "\n" is treated as the character '\' followed by 'n', not the newline character.
	