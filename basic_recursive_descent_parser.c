/*
Syntax Analytics : only detected error from potential sentense
*/

#include <stdio.h>
#include <string.h>

enum{null, PLUS, STAR, NUMBER, LPAREN, RPAREN, END} dtype;

typedef struct{
	int dtype;
	int value;
}RET;

char token = ' ';
char target_string[] = "3+2*6$";
int cursor = 0;
int max_len = -1;

void term();
void factor();
void expression();
void error(int);

void get_token(){
	// next token of input --> token
	int here = target_string[cursor];
	
	if(here == '+'){
		token = PLUS;
	}else if(here == '*'){
		token = STAR;
	}else if(here == '('){
		token = LPAREN;
	}else if(here == ')'){
		token = RPAREN;
	}else if(here == '$'){
		token = END;
	}else if((here > '0') && (here < '9')){
		token = NUMBER;
	}else{
		error(cursor);
	}
	
	cursor += 1;
	
}

void expression(){
	// t + t + t ...
	term();
	// while()
}

void term(){
	// f * f * f ...
}

void factor(){
	// n | (expression)
}

void error(int i){
	printf("you get error! at %d\n", i);
}

int main(){
	// expression(); // start symbol
	max_len = strlen(target_string);
	get_token(); // read start symbol
	printf("%d\n", token);
	printf("%s\n", target_string);
	printf("%d %u\n", cursor, max_len);
	return 0;
}

