/*
Syntax Analytics : only detected error from potential sentense
+
Calculator for "Integer" Operation
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
char target_string[] = "302+22*6$";
int cursor = 0;
int max_len = -1;
int is_debug = 1;

int num = 0;


int term();
int factor();
int expression();
void error(int);
void nice_end();
void do_debug();

void nice_end(){
	printf("This sentense end\n");
	// exit(0);
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
	}else if((here >= '0') && (here <= '9')){
		token = NUMBER;
		num = here - '0'; // because of ascii type
		while((target_string[cursor+1] >= '0') && (target_string[cursor+1] <='9')){
			num = num*10;
			cursor++;
			here = target_string[cursor];
			num += here - '0';
		}
	}else{
		error(cursor);
	}
	do_debug();
	
	cursor += 1;
	
}

int expression(){
	// t + t + t ...
	// 이 작업을 하기 전에 이미 get_token() 수행하여, token 값이 채워져 있다고 가정
	int n = term();
	while(token == PLUS){
		get_token();
		n += term();
	}
	return n;

	// 이 작업이 끝나기 전에 다음 token 값을 읽어서 넘김
}

int term(){
	// f * f * f ...
	int n = factor();
	while(token == STAR){
		get_token();
		n *= factor();
	}
	return n;
}

int factor(){
	// n | (expression)
	int n = 0;
	if(token == NUMBER){
		n = num;
		get_token();
	}
	else if(token == LPAREN){
		get_token();
		n = expression();
		if(token == RPAREN){
			get_token();
		}else{
			error(cursor);
		}
	}else{
		error(cursor);
	}
	return n;
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
	printf("\t\t");
	for(t=0; t<i-1 ;t++){
		printf("%c", ' ');
	}
	printf("%c\n", '|');
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
	int answer = 0;
	answer = expression(); // start symbol
	if(token != END){
		error(cursor);
	}else{
		printf("Total Answer is %d\n", answer);
	}
	
	return 0;
}

