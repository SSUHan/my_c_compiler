/*
Syntax Analytics : only detected error from potential sentense
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum{null, PLUS, STAR, NUMBER, LPAREN, RPAREN, END} dtype;

typedef struct{
	int dtype;
	int value;
}RET;

char token = ' ';
char target_string[] = "3+2*+6$";
int cursor = 0;
int max_len = -1;
int is_debug = 1;

void term();
void factor();
void expression();
void error(int);
void nice_end();
void do_debug();

void nice_end(){
	printf("This sentense end\n");
	exit(0);
}

void get_token(){
	// next token of input --> token
	if (cursor > max_len){
		nice_end();
	}
	
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
		nice_end();
	}else if((here > '0') && (here < '9')){
		token = NUMBER;
	}else{
		error(cursor);
	}
	do_debug();
	
	cursor += 1;
	
}

void expression(){
	// t + t + t ...
	// 이 작업을 하기 전에 이미 get_token() 수행하여, token 값이 채워져 있다고 가정
	term();
	while(token == PLUS){
		get_token();
		term();
	}

	// 이 작업이 끝나기 전에 다음 token 값을 읽어서 넘김
}

void term(){
	// f * f * f ...
	factor();
	while(token == STAR){
		get_token();
		factor();
	}
}

void factor(){
	// n | (expression)
	if(token == NUMBER){
		get_token();
	}
	else if(token == LPAREN){
		get_token();
		expression();
		if(token == RPAREN){
			get_token();
		}else{
			error(cursor);
		}
	}else{
		error(cursor);
	}
}

void error(int i){
	
	printf("\n\n");
	printf("\t\t%s\n", target_string);
	int t = 0;
	printf("\t\t");
	for(t=0; t<i-1 ;t++){
		printf("%c", ' ');
	}
	printf("%c\n", '^');
	printf("\n");
	printf("you get error! at position : %d\tvalue : %c\n", i-1, target_string[i-1]);
	printf("\n");
	exit(0);
}

void do_debug(){
	if(is_debug == 1){
		printf("max_len : %d\tcursor : %d\ttoken : %d\ttoken mean : %c\n", max_len, cursor, token, target_string[cursor]);
	}
}

int main(){
	max_len = strlen(target_string);
	printf("target_string : %s\n", target_string);
	printf("cursor : %d,  max length : %u\n", cursor, max_len);
	
	get_token(); // read start symbol
	expression(); // start symbol
	
	return 0;
}

