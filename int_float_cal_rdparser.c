/*
Syntax Analytics : only detected error from potential sentense
+
Calculator for "Integer" Operation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

enum{null, PLUS, STAR, NUMBER, LPAREN, RPAREN, END} t_type;
enum{null2, INTEGER_VAL, FLOAT_VAL} d_type;

// 정수타입이라면 dtype = INTEGER_VAL, 실수타입이라면 dtype = FLOAT_VAL 이 들어갈 것이고,
// 정수타입이라면 i_value 에 값이, 실수타입이라면 f_value 에 값이 들어가 있을 것으로 약속한다
typedef struct{
	int dtype;
	int i_value;
	float f_value;
}RET;

char token = ' ';
char target_string[] = "2.54*+(6+4)$"; 
int cursor = 0; // target string 서 읽어들이는 커서 위치값
int max_len = -1; // Target String 의 길이
int is_debug = 0; // Debug message 출력 여부

int num = 0; // 정수타입 계산값
int float_flag = 0; // 실수 여부 판단('.')
int floating_size = 0; // 실수의 아랫위치값
float f_num = 0.0; // 실수타입 계산값

RET glb_ret;


RET* term();
RET* factor();
RET* expression();
void error(int);
void nice_end();
void do_debug();
void show_RET(RET * ret, char*);
void warning(int i);

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
		float_flag = 0; // 실수 플레그 초기화
		floating_size = 0; // 실수 크기 초기화
		f_num = 0.0; // 실수값 초기화 
		token = NUMBER;
		num = here - '0'; // because of ascii type
		while(((target_string[cursor+1] >= '0') && (target_string[cursor+1] <='9')) || (target_string[cursor+1] == '.')){
			if(target_string[cursor+1] == '.'){
				// printf("up float flag\n");
				float_flag = 1; // 실수 플레그 올림
				f_num = num; // 실수값에 정수부 저장
				cursor++; // '.' : 진행시킴
				continue;
			}
			if(float_flag == 1){
				// 실수플레그가 서있다면
				// printf("in to float_flag\n");
				floating_size += 1; // 실수 크기 하나 증가시키고
				cursor++;
				here = target_string[cursor] - '0';
				f_num += here*pow(0.1, floating_size);
			}else{
				// 정수 플레그
				num = num*10;
				cursor++;
				here = target_string[cursor];
				num += here - '0';
			}
			// printf(">> f_flag : %d\n", float_flag);
			// printf(">> %d\n", num);
			// printf(">> %f\n", f_num);
		}
		if(float_flag == 1){
			glb_ret.dtype = FLOAT_VAL;
			glb_ret.f_value = f_num;
		}else{
			// integer value;
			glb_ret.dtype = INTEGER_VAL;
			glb_ret.i_value = num;
		}
		show_RET(&glb_ret, "global ret");
		
	}else{
		error(cursor);
	}
	do_debug();
	
	cursor += 1;
	
}

RET* expression(){
	// t + t + t ...
	// 이 작업을 하기 전에 이미 get_token() 수행하여, token 값이 채워져 있다고 가정
	RET * ret = term();
	show_RET(ret, "expression");
	while(token == PLUS){
		get_token();
		RET * tmp = term();
		if(ret->dtype == INTEGER_VAL && tmp->dtype == INTEGER_VAL){
			ret->i_value += tmp->i_value;
		}else if(ret->dtype == INTEGER_VAL && tmp->dtype == FLOAT_VAL){
			warning(cursor);
			ret->dtype = FLOAT_VAL;
			ret->f_value = ret->i_value;
			ret->f_value += tmp->f_value;
		}else if(ret->dtype == FLOAT_VAL && tmp->dtype == INTEGER_VAL){
			warning(cursor);
			ret->f_value += tmp->i_value;
		}else if(ret->dtype == FLOAT_VAL && tmp->dtype == FLOAT_VAL){
			ret->f_value += tmp->f_value;
		}else{
			error(cursor);
		}
		free(tmp);
		
	}
	return ret;

	// 이 작업이 끝나기 전에 다음 token 값을 읽어서 넘김
}

RET* term(){
	// f * f * f ...
	RET * ret = factor();
	show_RET(ret, "term");
	while(token == STAR){
		get_token();
		RET * tmp = factor();
		if(ret->dtype == INTEGER_VAL && tmp->dtype == INTEGER_VAL){
			ret->i_value *= tmp->i_value;
		}else if(ret->dtype == INTEGER_VAL && tmp->dtype == FLOAT_VAL){
			warning(cursor);
			ret->dtype = FLOAT_VAL;
			ret->f_value = ret->i_value;
			ret->f_value *= tmp->f_value;
		}else if(ret->dtype == FLOAT_VAL && tmp->dtype == INTEGER_VAL){
			warning(cursor);
			ret->f_value *= tmp->i_value;
		}else if(ret->dtype == FLOAT_VAL && tmp->dtype == FLOAT_VAL){
			ret->f_value *= tmp->f_value;
		}else{
			error(cursor);
		}
		free(tmp);
	}
	return ret;
}

RET* factor(){
	// n | (expression)
	RET * ret = (RET*)malloc(sizeof(RET));
	// int n = 0;
	if(token == NUMBER){
		if(glb_ret.dtype == INTEGER_VAL){
			ret->dtype = INTEGER_VAL;
			ret->i_value = glb_ret.i_value;
		}else{
			ret->dtype = FLOAT_VAL;
			ret->f_value = glb_ret.f_value;
		}
		get_token();
	}
	else if(token == LPAREN){
		get_token();
		ret = expression();
		if(token == RPAREN){
			get_token();
		}else{
			error(cursor);
		}
	}else{
		error(cursor);
	}
	return ret;
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

void warning(int i){
	printf(">> warning : %s at %d\n", "type mismatch", i);
}

void do_debug(){
	if(is_debug == 1){
		printf("max_len : %d\tcursor : %d\ttoken : %d\ttoken mean : %c\n", max_len, cursor, token, target_string[cursor]);
	}
}

void show_RET(RET * ret, char * str){
	if(is_debug == 1){
		if(ret->dtype == INTEGER_VAL){
			printf("message : %s, type : %s, value : %d\n", str, "정수형", ret->i_value);
		}else{
			printf("message : %s, type : %s, value : %f\n", str, "실수형",ret->f_value);
		}
	}
	
}

int main(){
	max_len = strlen(target_string);
	printf("target_string : %s\n", target_string);
	printf("cursor : %d,  max length : %u\n", cursor, max_len);
	
	get_token(); // read start symbol
	// int answer = 0;

	RET * answer = expression(); // start symbol
	if(token != END){
		error(cursor);
	}else{
		// printf("Total Answer is %d\n", answer);
		printf("\n###Total answer###\n");
		is_debug = 1;	
		show_RET(answer, "answer");
	}
	
	return 0;
}

