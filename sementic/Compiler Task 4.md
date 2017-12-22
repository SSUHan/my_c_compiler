# Compiler Task 4 

> 컴퓨터학부 4학년
>
> 20132429 이준수
>
> 2017.12.22



#### 목적

YACC, LEX, syntax1.c semantic.c gen.c 등을 이용하여, 실제 C언어 파일을 Input 으로 사용하였을 때, 실행을 할 수 있는 어셈블리언어 코드로 변환하는 프로그램을 작성한다. 이후 이를 이용하여 여러가지 C언어 파일을 테스트 해보고 결과를 기록한다.

#### 테스트환경

Ubuntu 16.04

Yacc : Bison -> sudo apt install bison

lex : Flex -> sudo apt install flex

#### 사용 명령어

##### Shell Script 

do_recompile.sh:

```shell
rm a.out
rm a.asm
gcc y.tab.c lex.yy.c print.c syntax1.c gen.c semantic.c main.c -ll
./a.out $1
```

- 기존에 있는 a.out 파일을 항상 제거하고 재컴파일(recompile)를 하는 로직이다.
- 기존에 있는 a.asm 역시 제거하고 새로 한다. 
- -ll 옵션은 Unix 기반의 프로그램인 Yacc 과 lex 를 링크(Linkage)하는 과정에서 한개의 함수가 내부호출함수명이 다르다. 따라서 생기는 링키지 오류를 보류하는 역할을 해준다는 것을 확인하였다.
- $1 : 테스트 C 파일이 들어갈 자리

##### C File

main.c:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"

extern FILE *yyin;
extern int syntax_err;
extern int semantic_err;
extern A_NODE *root;
FILE *fout;

void initialize();
void print_ast();
void print_sem_ast();
void code_generation();
void semantic_analysis();
extern FILE *yyin;

void main(int argc, char *argv[]) 
{
	if (argc<2){
		printf("source file not given\n");
		exit(1);
	}
	if (strcmp(argv[1],"-o")==0) {
		if (argc>3) 
			if ((fout=fopen(argv[2],"w")) == NULL) {
				printf("can not open output file: %s\n",argv[3]); 
				exit(1);
			}
			else ;
		else { 
			printf("out file not given\n");
			exit(1);
		} 
	}
	else if (argc==2){
		if ((fout=fopen("a.asm","w")) == NULL) {
			printf("can not open output file: a.asm\n"); 
			exit(1);
		}
	}
	if ((yyin=fopen(argv[argc-1],"r")) == NULL){
		printf("can not open input file: %s\n",argv[argc-1]);
		exit(1);
	}
	printf("\nstart syntax analysis\n");
	initialize();
	yyparse();
	if (syntax_err) 
		exit(1);
	print_ast(root);

	printf("\nstart semantic analysis\n");
	semantic_analysis(root);
	if (semantic_err) 
		exit(1);

	//print_sem_ast(root);
	
	printf("start code generation\n");
	code_generation(root);
	printf("end code generation\n");

	exit(0);
}

```

* 시간관계상 sem ast 를 출력하는 print_sem_ast 는 출력에서 제외하였다.



#### 결과

test2.c:

```c
int main() {
   
   int a = 1;
   int b = 2;

   return 1;
}
```

![스크린샷 2017-12-22 오후 11.00.17](/Users/myZZUNG/Desktop/스크린샷 2017-12-22 오후 11.00.17.png)

* Syntax Tree 출력 완료
* Semtantic 분석 완료 및 분석 과정 출력 완료

![스크린샷 2017-12-22 오후 11.02.34](/Users/myZZUNG/Desktop/스크린샷 2017-12-22 오후 11.02.34.png)

* 결과 도출 완료