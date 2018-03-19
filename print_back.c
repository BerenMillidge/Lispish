// the beginnings of a repl

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <string.h>

// declare a buffe for user input of size 2048
static char buffer[2048];

//fake readline function
char* readline(char* prompt){
	fputs(prompt, stdout);
	fgets(buffer, 2048, stdin);
	char* cpy = malloc(strlen(buffer)+1);
	cpy[strlen(cpy)-1] = "/0";
	return cpy;
}

//fake add history function

void add_history(char* unused){}

//otherwise include the editline headers 
#else
#include <editline/readline.h>
#include<editline/history.h>
#endif



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