// the beginnings of a repl

#include <stdio.h>


// declare a buffe for user input of size 2048
static char input[2048];

int main(int argc, char**argv){
	//print version and exit information
	puts("Lispy version 0.0.0.0.1");
	puts("Press cntl +c to Exit \n");

	// in a neverending loop
	while(1){
		fputs("lispy>",stdout);
		//reada line of user input of maxiuimum size 2048
		fgets(input, 2048, stdin);
		//echo input back to user
		printf("No you're a %s", input);
	}
	return 0;
}