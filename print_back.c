// the beginnings of a repl

#include <stdio.h>
#include <stdlib.h>

#include <editlibe/readline.h>
#include<editline/history.h>


// declare a buffe for user input of size 2048
static char input[2048];

int main(int argc, char**argv){
	//print version and exit information
	puts("Lispy version 0.0.0.0.1");
	puts("Press cntl +c to Exit \n");

	// in a neverending loop
	while(1){
		char* input = readline("lispy> ");
		//add input to history
		add_history(input);
		//echo input back to user
		printf("No you're a %s\n", input);
		//free retrievedd input
		free(input);
	}
	return 0;
}