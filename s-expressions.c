// okay, now for the serious internals of the language

// okay, this is the error handling section

#include "mpc.h"

#ifdef _WIN32


static char buffer[2048];

char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

void add_history(char* unused) {}

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif



//create enum types for possible lval types
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR};

enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

// so an s-expression is a list, of variable length. This cannot be directly 
// represented in c, so I use a pointer to the start of the list
// which can then expand or decrease as needed!
// the cel will be a pointer to a list of pointers to the individual lvals


typedef struct {
	int type;
	long num;
	//error and symbol types are strings now
	char* err;
	char* sym;
	int count;
	// and nwo a struct within a struct, count and pointer to list of lvals
	struct lval** cell;
} lval;


//rewrite the lvals to return a pointer to the lval and not the actual thing
// itself, so it's easier!

lval* lval_num(long x){
	lval* v = malloc(sizeof(lval)); // allocate the memory ahead of time
	v->type = LVAL_NUM;
	v->num = x;
	return v;
}


//returns a pointer to the error eval just created
lval* lval_err(char* m){
	lval* v = malloc(sizeof(lval)); // I don't know how it gets sizeof lval
	// since lval contains cell which is a variable length list?
	// so I really don't know there. or maybe it just contains a fixed length pointer?
	v->type = LVAL_ERR;
	// assign memory for the string!
	v->err = malloc(strlen(m)+1);
	strcpy(v->err, m);
	return v;
}

lval* lval_sym(char* s){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s)+1);
	strcpy(v->sym, s);
	return v;
}


//create an empty s-expr lval
lval* lval_sexpr(void){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}


//delete the lval correctly depending on type to ensure there are no memory leaks
void lval_del(lval* v){
	switch(v->type){
		//do nothing special for numbertypes
		case LVAL_NUM: break;

		//for err and sym free string data // you need to be careful about remembering
		//to free strings as they are things to easily forger
		case LVAL_ERR: free(v->err); break;
		case LVAL_SYM: free(v->sym); break;

		// if s-expr then delete all elements inside
		case LVAL_SEXPR:
			for (int i = 0; i<v->count; i++){
				lval_del(v->cell[i]);
			}
			//also free memory allocated to contain the pointers
			// you need to remember this!
			free(v->cell);
			break;
	}
	//free memory allocated for lval struct iteself
	free(v);
}


// now it's time to add the functions allowing reading in lvals
// andconstructing the lval from the AST parsed from the input

lval* lval_read_num(mpc_ast_t* t){
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return errno!=ERANGE ? lval_num(x): lval_err("Invalid Number!");
}

lval* lval_read(mpc_ast_t* t){
	//if symbol or number return conversion to that type
	if(strstr(t->tag, "number")){
		return lval_read_num(t);
	}
	if(strstr(t->tag, "symbol")){
		return lval_sym(t->contents);
	}

	// if root or sexpr then create empty list
	lval* x = NULL;
	if (strcmp(t->tag, ">")==0){
		x = lval_sexpr();
	}
	if(strstr(t->tag, "sexpr")){
		x = lval_sexpr();
	}
	//fill in the list with any valid expression contained within
	for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }
  return x;
}

// add a new lval to the list in cell
lval* lval_add(lval* v, lval* x){
	v->count++;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count); // his allocates 
	// enough space for the new cell structure
	v->cell[v->count-1] = x; // since it is zero indexed!
	return v;
}



// resolve circular dependency using a forward definition
void lval_print(lval* v);

// okay, so I also need to be able to print the s-expressions
// which possily needs a special function
// it loops over all the sub expressions and prints out them individually separated by spaces
// i.e. as they are as the input

void lval_expr_print(lval* v, char open, char close){
	putchar(open);
	for (int i = 0; i<v->count; i++){
		lval_print(v->cell[i]);
	//don't put a trailing spaceif last element!
	if(i!=(v->coutn-1)){
		putchar(' ');
	}
}
putchar(close);
}


void lval_print(lval* v){
	//needs to check if an error or not, to print correctly
	switch(v.type){
		case LVAL_NUM: printf("%li",v.num); break;
		case LVAL_ERR: printf("Error: %s", v->err); break;
		case LVAL_SYM: printf("%s", v->sym); break;
		case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
	}
}

void lval_println(lval* v){
	lval_print(v);
	putchar('\n');
}

// replace the eval op to deal with lvals as intended
//including with the errors
lval eval_op(lval x, char* op, lval y){
	if(x.type==LVAL_ERR){
		return x;
	}
	if(y.type==LVAL_ERR){
		return y;
	}

	//otherwise do the maths on the numbervalues
  if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
  if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
  if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
  if (strcmp(op, "/") == 0) {
    /* If second operand is zero return error */
    return y.num == 0
      ? lval_err(LERR_DIV_ZERO)
      : lval_num(x.num / y.num);
  }

  return lval_err(LERR_BAD_OP);
}

// the pointer now makes sense, since it's a pointer to an object
// since c is always pass by value, and not pass by reference
// you essentially have to force pass by reference in by using pointers
// and do it manually!
lval eval(mpc_ast_t* t){
	if(strstr(t->tag, "number")){
		//if there is an error in conversion
		// I don't understand what is happening here. ERANGE is not devined
		// and errno is always 0, so I don't know!
		errno = 0;
		long x = strtol(t->contents, NULL, 10);
		return errno!=ERANGE ? lval_num(x): lval_err(LERR_BAD_NUM);
	}
	//get the operation and the first operand!
	char* op = t->children[1]->contents;
	//evaluate the first operand to its maximum potential
	lval x = eval(t->children[2]);

	int i = 3;
	while(strstr(t->children[i]->tag, "expr")){
		x = eval_op(x, op, eval(t->children[i]));
		i++;
		//I'm not sure I'm understanding thie fully, the recursion. oh well
	}
	return x;
}

int main(int argc, char** argv) {
  
  
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr  = mpc_new("sexpr");
	mpc_parser_t* Expr   = mpc_new("expr");
	mpc_parser_t* Lispy  = mpc_new("lispy");

	mpca_lang(MPCA_LANG_DEFAULT,
	  "                                          \
	    number : /-?[0-9]+/ ;                    \
	    symbol : '+' | '-' | '*' | '/' ;         \
	    sexpr  : '(' <expr>* ')' ;               \
	    expr   : <number> | <symbol> | <sexpr> ; \
	    lispy  : /^/ <expr>* /$/ ;               \
	  ",
	  Number, Symbol, Sexpr, Expr, Lispy);
  
  puts("Lispy Version 0.0.0.0.3");
  puts("Press Ctrl+c to Exit\n");
  
  while (1) {
  
    char* input = readline("lispy> ");
    add_history(input);
    
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      
      lval* x = lval_read(r.output);
      lval_println(x);
      lval_del(x);
      
    } else {    
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    
    free(input);
    
  }
  
  mpc_cleanup(5, Number, Symbol, Sexpr,Expr, Lispy);
  
  return 0;
}
