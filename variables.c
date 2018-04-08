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
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR};

enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

// so an s-expression is a list, of variable length. This cannot be directly 
// represented in c, so I use a pointer to the start of the list
// which can then expand or decrease as needed!
// the cel will be a pointer to a list of pointers to the individual lvals



// so to store varibales and functions, there needs to be a datastructure, or "environment"
// which stores all the defiend varibales and functions
// and has them defined as function pointers
//so lets try and figure this out
// define the function pointer type

//do this to avoid the structs and forward declare the types

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

typedef lval*(*lvalbuiltin)(levn*, lval*);


typedef struct lval{
	int type;
	long num;
	//error and symbol types are strings now
	char* err;
	char* sym;
	int count;
	// and nwo a struct within a struct, count and pointer to list of lvals
	struct lval** cell;
} lval;
// lenv is the environment data structure!

// this is effectively just a parrallel array of symbols and values
// for the variables in the environment
// and ofcourse the count since c needs that since it's array abilities are rather lacking!
struct lenv{
	int count;
	char** syms;
	lval** vals;
}

//create a new environment
lenv* lenv_new(void){
	lenv* e = malloc(sizeof(lenv));
	e->count = 0;
	e->syms = NULL;
	e->vals = NULL;
	return e;
}

void lenv_del(lenv* e){
	//ensure all the memory taken up is freed!
	// it is kind of cool in a way how you have to do this
	for (int i = 0; i<e->count; i++){
		free(e->syms[i]);
		lval_del(e->vals[i]);
	}
	free(e->syms);
	free(e->vals);
	free(e);
}

// so, now there needs to be ways to add, presumably remove, and get variables from the environment
// at the moment it uses a simple linear(!) list search, which is horrendously inefficient
// inreality you probably need hashmaps

lval* lenv_get(lenv* e, lval* k){
	// I'm not a fan of this guy's variable naming. it obscures more than it reveals!

	//iterate over all items in environment
	for (int i = 0; i< e->count; i++){
		//check if stored string matches symbol string
		// if it does return copy - since immutability!
		if(strcmp(c->syms[i], k->sym)==0){
			return lval_copy(e->vals[i])
		}
	}	
	// if no symbol foud return error
	return lval_err("unbound symbol!");
}

//add a variable to the environment

void lenv_put(lenv* e, lval* k, lval* v){
	// k is the key - i.e. the symbol presumably, and v -s the value
	//  but this could be solved by naming the variables better, dagnabbit!

	//first iterate to check if variable already exists
	for (int i =0; i<e->count; i++){
		//if variable is found delete with item at that position and repalce with var
		// supplied by user
		if(strcmp(e->syms[i], k->sym)==0){
			lval_del(e->vals[i]); // this is necessary to prevent memory leaks!
			e->vals[i] = lval_copy(v);
			return;
		}
	}

	//if no existing entry allocate space for the new entry
	e->count++;
	e->vals = realloc(e->vals, sizeof(lval*) * e->count);
	e->syms = realloc(e->syms, sizeof(char*) * e->count);
}
//rewrite the lvals to return a pointer to the lval and not the actual thing
// itself, so it's easier!

// okay, now the lvals now need t obe defined to eval a lval

lval* lval_eval(lenv( e, lval* v){
	if (v->type == LVAL_SYM){
		//get it from environment
		lval* x = lenv_get(e,v);
		lval_del(v);
		return x;
	}
	if (v->type ==LVAL_SEXPR){
		return lval_eval_sexpr(e,v);
	}
	return v;
})

// so now the evaluation of the s expression needs to change also
lval* lval_eval_sexpr(lenv* e, lval* v){
	for (int i = 0; i<v->count; i++){
		v->cell[i] = lval_eval(e, v->cell[i]);
	}

	for (int i = 0; i< v->count; i++){
		if (v->cell[i] ->type == LVAL_ERR){
			return lval_take(v,i)
		}
	}

	if (v->count ==0){
		return v;
	}
	if (v->count == 1){
		return lval_take(v,0);
	}
	//ensure first element is a function afte evaluatoin
	lval* f = lval_pop(v,0)
	if (f->type !=LVAL_FUN){
		lval_del(v);
		lval_del(f);
		return lval_err("first elemen is not a functoin");
	}
	// if so cal function to get result
	lval* result = f->fun(e,v);
	lval_del(f);
	return result;
}

//lval function constructor
lval* lval_fun(lbuiltin func){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_FUN;
	v->fun = func;
	return v;
}

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
//constuct the q expression
lval* lval_qexpr(void){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_QEXPR;
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
		case LVAL_QEXPR:
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

// add a new lval to the list in cell
lval* lval_add(lval* v, lval* x){
	v->count++;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count); // his allocates 
	// enough space for the new cell structure
	v->cell[v->count-1] = x; // since it is zero indexed!
	return v;
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
		//can reuse most of the stuff for sexpr for qexpr, but need to create an empty one
	if(strstr(t->tag,'qexpr')){
		x = lval_qexpr();
	}
	//fill in the list with any valid expression contained within
	for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }
  return x;
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
	if(i!=(v->count-1)){
		putchar(' ');
	}
}
putchar(close);
}


void lval_print(lval* v){
	//needs to check if an error or not, to print correctly
	switch(v->type){
		case LVAL_NUM: printf("%li",v->num); break;
		case LVAL_ERR: printf("Error: %s", v->err); break;
		case LVAL_SYM: printf("%s", v->sym); break;
		case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
		case LVAL_QEXPR: lval_expr_print(v, '{','}'); break;
	}
}

void lval_println(lval* v){
	lval_print(v);
	putchar('\n');
}


// now there needs to be a way to actuall evaluate these expressions
// hopefully not too horrendously difficult
// okay,, now time to define the lval pop and lval take functions

// pop removes an element from the lval list and returns it

lval* lval_pop(lval* v, int i){
	lval* x = v->cell[i];

	// now neeed to replace the item in the list, shift the memory after the items over on top of it
	memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*)*(v->count-i-1));

	//decrease the coutn
	v->count--;
	//reallocate the memory used
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	return x;
}

lval* lval_take(lval* v, int i){
	// this function just takes one element of a list of lvals nd dleletes the rest of the list
	lval* x = lval_pop(v,i);
	lval_del(v);
	return x;
}



//and finally the builtin op for the lval
lval* builtin_op(lval* a, char* op){

	// ensure all the arguments to the thing are numbers
	for (int i = 0; i< a->count; i++){
		if(a->cell[i]->type!=LVAL_NUM){
			lval_del(a);
			return lval_err("Cannot operate on a non number");
		}
	}

	//pop the first element
	lval* x = lval_pop(a,0);
	//if no arguments and sub then perform unary negation
	if((strcmp(op, "-")==0) && a->count==0){
		x->num = -x->num;
	}

	//while there are still elements remaining
	while(a->count>0){
		//pop next element
		lval* y = lval_pop(a,0);

		if(strcmp(op, "+")==0) {
			x->num += y->num;
		}
		if(strcmp(op, "-")==0){
			x->num -=y->num;
		}
		if(strcmp(op, "*")==0){
			x->num *= y->num;
		}
		if(strcmp(op, "/")==0){
			if(y->num==0){
				lval_del(x);
				lval_del(y);
				x = lval_err("Division by Zero!");
				break;
			}
			// I guess c doesn't have try-catch
			x->num /= y->num;
		}
		lval_del(y);
	}
	lval_del(a);
	return x;
}
// replace the eval op to deal with lvals as intended
//including with the errors

lval* lval_eval_sexpr(lval* v);


lval* lval_eval(lval* v){
	if(v->type == LVAL_SEXPR){
		return lval_eval_sexpr(v);
	}
	return v;
}

lval* builtin(lval* a, char* func);


lval* lval_eval_sexpr(lval* v){
	//first evaluate all the children of the s-expr
	for (int i = 0; i< v->count; i++){
		v->cell[i] = lval_eval(v->cell[i]);
	}

	// if there are any errors return it immediately
	for (int i = 0; i<v->count; i++){
		if (v->cell[i]->type ==LVAL_ERR){
			return lval_take(v,i);
		}
	}
	// if it's an empty expression just return
	if(v->count == 0){
		return v;
	}
	if(v->count ==1){
		return lval_take(v,0);
	}
	//ensure the first element is a symbol
	lval* f = lval_pop(v, 0);
	if(f->type!=LVAL_SYM){
		lval_del(f);
		lval_del(v);
		return lval_err("S-expression does not start with symbol!?");
	}
	//call builtin evaluator with operator
	lval* result = builtin(v, f->sym);
	lval_del(f);
	return result;
}





// okay, definitions for qexpr functions here!

// functions are list - creates a q expression containing it
//head, tail
// join,//and eval
// those are the basic building blocks of the q expression - i.e. macro!


#define LASSERT(args, cond, err) \
  if (!(cond)) { lval_del(args); return lval_err(err); }


 // rewrite with the lasserts!
lval* builtin_head(lval* a) {
  LASSERT(a, a->count == 1,
    "Function 'head' passed too many arguments!");
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
    "Function 'head' passed incorrect type!");
  LASSERT(a, a->cell[0]->count != 0,
    "Function 'head' passed {}!");

  lval* v = lval_take(a, 0);
  while (v->count > 1) { lval_del(lval_pop(v, 1)); }
  return v;
}


lval* builtin_tail(lval* a) {
  LASSERT(a, a->count == 1,
    "Function 'tail' passed too many arguments!");
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
    "Function 'tail' passed incorrect type!");
  LASSERT(a, a->cell[0]->count != 0,
    "Function 'tail' passed {}!");

  lval* v = lval_take(a, 0);
  lval_del(lval_pop(v, 0));
  return v;
}

//list just conerts s expression to q expression so it's trivial
lval* builtin_list(lval* a){
	a->type = LVAL_QEXPR;
	return a;
}

// built in eval just conerts it to s expression and evaluates with lval_eval

lval* builtin_eval(lval* a){
	LASSERT(a, a->count==1, "Function eval passed too many arguments");
	LASSERT(a, a->cell->type==LVAL_QEXPR, "Function'eval' passed incorrect type!");

	lval* x = lval_take(a,0);
	x->type = LVAL_SEXPR;
	return lval_eval(x);
}

// and now join. so it checks all args are q expression and join together one by one
// by popping it from y and adding it to x until y is empty and deletes y and returns x

lval* builtin_join(lval* a){
	for (int i  =0; i< a->count; i++){
		LASSERT(a->cell[i]->type==LVAL_QEXPR, "Function 'join' passed incorrect type");
	}

	lval* x = lval_pop(a,0);
	while(a->count){
		x = lval_join(x, lval_pop(a,0));
	}
	lval_del(a);
	return x;
}

lval* builtin(lval* a, char* func) {
  if (strcmp("list", func) == 0) { return builtin_list(a); }
  if (strcmp("head", func) == 0) { return builtin_head(a); }
  if (strcmp("tail", func) == 0) { return builtin_tail(a); }
  if (strcmp("join", func) == 0) { return builtin_join(a); }
  if (strcmp("eval", func) == 0) { return builtin_eval(a); }
  if (strstr("+-/*", func)) { return builtin_op(a, func); }
  lval_del(a);
  return lval_err("Unknown Function!");
}

// okay, now for the builtin op to evaluate the eval
// this is only meant to be processed once the entire AST is parsed
// so there are only numbers in the lef nodes
// and it does the parsing of the AST recursively of course

// the pointer now makes sense, since it's a pointer to an object
// since c is always pass by value, and not pass by reference
// you essentially have to force pass by reference in by using pointers
// and do it manually!
int main(int argc, char** argv) {
  
  
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr  = mpc_new("sexpr");
	mpc_parser_t* Qexpr  = mpc_new("qexpr");
	mpc_parser_t* Expr   = mpc_new("expr");
	mpc_parser_t* Lispy  = mpc_new("lispy");

	mpca_lang(MPCA_LANG_DEFAULT,
	  "                                          \
	    number : /-?[0-9]+/ ;                    \
	    symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;         \
	    sexpr  : '(' <expr>* ')' ;               \
	    qexpr  : '{' <expr>  '}' ;               \
	    expr   : <number> | <symbol> | <sexpr> ; \
	    lispy  : /^/ <expr>* /$/ ;               \
	  ",
	  Number, Symbol, Sexpr,Qexpr, Expr, Lispy);
  
  puts("Lispy Version 0.0.0.0.3");
  puts("Press Ctrl+c to Exit\n");
  
  while (1) {
  
    char* input = readline("lispy> ");
    add_history(input);
    
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      
      lval* x = lval_eval(lval_read(r.output));
      lval_println(x);
      lval_del(x);
      
    } else {    
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    
    free(input);
    
  }
  
  mpc_cleanup(6, Number, Symbol, Sexpr,Qexpr,Expr, Lispy);
  
  return 0;
}
