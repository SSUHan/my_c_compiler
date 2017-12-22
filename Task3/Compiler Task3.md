# Compiler Task3

>  컴퓨터학부 4학년
>
> 20132429 이준수
>
> 2017.11.27



### 목적

YACC 프로그램과 LEX 프로그램을 이용하여, C 언어 파일을 Lex 파일의 문법에 맞도록 Parsing 하고, Yacc 파일의 문법에 맞도록 C언어 문법을 따르는 Parsing Tree 를 작성한다. 작성 후 이를 트리형태로 출력하여 내부의 구조를 확인할 수 있도록 돕는다.

### 테스트환경

Ubuntu 16.04

Yacc : Bison -> sudo apt install bison

lex : Flex -> sudo apt install flex

### 사용명령어

##### shell 스크립트

re_compile.sh : 

```sh
rm a.out
gcc y.tab.c lex.yy.c syntax1.c print.c treeToOriginal.c main1.c -ll
```

 기존에 있는 a.out 파일을 항상 제거하고 재컴파일(recompile)를 하는 로직이다.

 -ll 옵션은 Unix 기반의 프로그램인 Yacc 과 lex 를 링크(Linkage)하는 과정에서 한개의 함수가 내부호출함수명이 다르다. 따라서 생기는 링키지 오류를 보류하는 역할을 해준다는 것을 확인하였다.

unit_test.sh : 

```shell
./a.out < test1.c
./a.out < test2.c
./a.out < test3.c
```

Case Test 를 위한 자동화 스크립트이다. 어차피 테스트를 해야하는 경우는 파일로 들어오는데 이를 자동화하기 위해서 쉘스크립트를 작성하였다.

##### C 파일

main1.c : 

```c
#include <stdio.h>
#include <stdlib.h>
#include "type.h"
extern int syntax_err;
extern A_NODE *root;
void initialize();
void print_ast();
void rvt_statement_list();
int main()
{
	printf("\nstart syntax analysis\n");
	initialize();
	yyparse();
	if (syntax_err) 
		exit(1);
	print_ast(root);
	rvt_statement_list(root);
	exit(0);
}
```

 원시프로그램으로 복구를 하기위해서 호출하는 statement_list 함수는 내부적으로 사용됨에 있어서 prt_for_expression 함수등 print.c 파일에서 이미 중복선언이 되는 경우가 발생하였다. 따라서 모든 원시프로그램 복구 함수들은 이름앞에 prefix 로 "rvt_" 를 사용하였다.

treeToOriginal.c : 

```c
//#include <stdio.h>
//#include <stdlib.h>
#include "type.h"

void rvt_statement(A_NODE *node);
void rvt_statement_list(A_NODE *node);
void rvt_expression(A_NODE *node);
void rvt_prt_expression(A_NODE *node);
void rvt_for_expression(A_NODE *node);

void rvt_statement(A_NODE *node) {
	switch (node->name){
		case N_STMT_LABEL_CASE:
			printf("case ");
			rvt_expression(node->llink);
			printf(": ");
			rvt_statement(node->rlink);
			break;
		case N_STMT_LABEL_DEFAULT:
			printf("default: ");
			rvt_statement(node->clink);
			break;
		case N_STMT_COMPOUND:
			printf("{ ");
			if(node->llink)
				rvt_A_ID_LIST(node->llink);
			rvt_statement_list(node->rlink);
			printf("} ");
			break;
		case N_STMT_EMPTY:
			printf("; ");
			break;
		case N_STMT_EXPRESSION:
			rvt_expression(node->clink);
			printf("; ");
			break;
		case N_STMT_IF_ELSE:
			printf("if ( ");
			rvt_expression(node->llink);
			printf(") ");
			rvt_statement(node->clink);
			printf("else ");
			rvt_statement(node->rlink);
			break;
		case N_STMT_IF:
			printf("if ( ");
			rvt_expression(node->llink);
			printf(") ");
			rvt_statement(node->rlink);
			break;
		case N_STMT_SWITCH:
			printf("switch ( ");
			rvt_expression(node->llink);
			printf(") ");
			rvt_statement(node->rlink);
			break;
		case N_STMT_WHILE:
			printf("while ( ");
			rvt_expression(node->llink);
			printf(") ");
			rvt_statement(node->rlink);
			break;
		case N_STMT_DO:
			printf("do ");
			rvt_statement(node->llink);
			printf("while ( ");
			rvt_expression(node->rlink);
			printf(") ");
			break;
		case N_STMT_FOR:
			printf("for ( ");
			rvt_for_expression(node->llink);
			printf(") ");
			rvt_statement(node->rlink);
			break;
		case N_STMT_CONTINUE: 
			printf("continue; ");
			break;
		case N_STMT_BREAK:
			printf("break; ");
			break;
		case N_STMT_RETURN:
			printf("return ");
			if (node->clink)
				rvt_prt_expression(node->clink);
			printf("; ");
			break;
	}
}

void rvt_statement_list(A_NODE *node){
	switch (node->name) {
		case N_STMT_LIST:
			rvt_statement(node->llink);
			rvt_statement_list(node->rlink);
			break;
		case N_STMT_LIST_NIL:
			break;
	}
}

void rvt_prt_expression(A_NODE *node) {
	switch (node->name) {
		case N_FOR_EXP:
			if(node->llink)
				rvt_expression(node->llink);
			printf("; ");
			if(node->clink)
				rvt_expression(node->clink);
			printf("; ");
			if(node->rlink)
				rvt_expression(node->rlink);
			break;
	}
}	

void rvt_for_expression(A_NODE *node){
	printf("[For Expression]");
}

void rvt_expression(A_NODE *node){
	printf("[Expression]");
}

void rvt_A_ID_LIST(A_NODE *node){
	printf("[A_ID_LIST]");
}

```

expression 과 A_ID_LIST 를 Revert 해서 출력하는 과정에서 약간의 이슈가 발생하여 계속 진행하고 있다.

syntax1.c : 중 일부

```c
// set declarator init (expression tree)
A_ID *setDeclaratorInit(A_ID *id, A_NODE *n) {
	id->init=n;
	return(id);
}

// set declarator kind
A_ID *setDeclaratorKind(A_ID *id, ID_KIND k) {
	A_ID *a;
	a=searchIdentifierAtCurrentLevel(id->name,id->prev);
	if (a)
		syntax_error(12,id->name);
	id->kind=k;
	return(id);
}

// set declarator type
A_ID *setDeclaratorType(A_ID *id, A_TYPE *t) {
	id->type=t;
	return(id);
}

// set declarator type (or element type)
A_ID *setDeclaratorElementType(A_ID *id, A_TYPE *t) {
	A_TYPE *tt;
	// 명칭 목록의 마지막 타입으로 t를 연결
	if (id->type==NIL)
		id->type=t;
	else {
		tt=id->type;
		while(tt->element_type)
			tt=tt->element_type;
		tt->element_type=t;
	}
	return (id);
}

// set declarator element type and kind
A_ID *setDeclaratorTypeAndKind(A_ID *id, A_TYPE *t,ID_KIND k) {
	id=setDeclaratorElementType(id,t);
	id=setDeclaratorKind(id,k);
	return(id);
}

// check function declarator and return type
A_ID *setFunctionDeclaratorSpecifier(A_ID *id, A_SPECIFIER *p) {
	A_ID *a;
	// storage class 검사
	if(p->stor)
		syntax_error(25);
	// specifier 가 생략된 경우 보정
	setDefaultSpecifier(p);
	// check function identifier immediately before '('
	// 함수의 리턴 타입을 완성하고 명칭의 종류를 ID_FUNC 로 지정한다
	if(id->type->kind!=T_FUNC){
		syntax_error(21);
		return(id);
	} else {
		id=setDeclaratorElementType(id,p->type);
		id->kind=ID_FUNC;
	}
	// 함수명칭으로 중복선언 검사
	a=searchIdentifierAtCurrentLevel(id->name,id->prev);
	if(a)
		if(a->kind!=ID_FUNC || a->type->expr)
			syntax_error(12,id->name);
		else { // 프로토타입이 있는경우 파라미터와 리턴 타입등 검사
			if(isNotSameFormalParameters(a->type->field,id->type->field))
				syntax_error(22,id->name);
			if(isNotSameType(a->type->element_type,id->type->element_type))
				syntax_error(26,a->name);
		}
	// 파라미터를 함수 내에서 사용할수 있게 스코프 조정	
	a=id->type->field;
	while(a){
		if(strlen(a->name))
			current_id=a;
		else if(a->type)
			syntax_error(23);
		a=a->link;
	}
	return(id);
}

A_ID *setFunctionDeclaratorBody(A_ID *id, A_NODE *n) {
	id->type->expr=n;
	return(id);
}

// set declarator_list type and kind based on storage class
A_ID *setDeclaratorListSpecifier(A_ID *id, A_SPECIFIER *p) {
	A_ID *a;
	setDefaultSpecifier(p);
	a=id;
	// 중복 선언 검사
	// 명칭의 타입 완성
	// 명칭의 종류로 ID_TYPE, ID_FUNC, 및 ID_VAR 등을 구분
	while (a) {
		if(strlen(a->name)&&searchIdentifierAtCurrentLevel(a->name,a->prev))
			syntax_error(12,a->name);
		a=setDeclaratorElementType(a,p->type);
		if(p->stor==S_TYPEDEF)
			a->kind=ID_TYPE;
		else if(a->type->kind==T_FUNC)
			a->kind=ID_FUNC;
		else
			a->kind=ID_VAR;
		a->specifier=p->stor;
		if (a->specifier==S_NULL)
			a->specifier=S_AUTO;
		a=a->link;
	}
	return(id);
}

// set declarator_list type and kind
A_ID *setParameterDeclaratorSpecifier(A_ID *id, A_SPECIFIER *p) {
	// 중복선언 검사
	if(searchIdentifierAtCurrentLevel(id->name,id->prev))
		syntax_error(12,id->name);
	// 파라미터의 storage class 와 void type 여부 검사
	if(p->stor || p->type==void_type)
		syntax_error(14);
	// 파라미터의 타입 완성
	// 명칭의 종류 결정
	setDefaultSpecifier(p);
	id=setDeclaratorElementType(id,p->type);
	id->kind=ID_PARM;
	return(id);
}

A_ID *setStructDeclaratorListSpecifier(A_ID *id, A_TYPE *t) {
	A_ID *a;
	a=id;
	while (a) {
		// 구조체 필드 명칭의 중복선언 검사
		// 필드명칭의 타입완성
		// 명칭의 종류 결정
		if(searchIdentifierAtCurrentLevel(a->name,a->prev))
			syntax_error(12,a->name);
		a=setDeclaratorElementType(a,t);
		a->kind=ID_FIELD;
		a = a->link;
	}
	return(id);
}

// set type name specifier
A_TYPE *setTypeNameSpecifier(A_TYPE *t, A_SPECIFIER *p) {
	// check storage class in type name
	if (p->stor)
		syntax_error(20);
	setDefaultSpecifier(p);
	t=setTypeElementType(t,p->type);
	return(t);
}

// set type element type
A_TYPE *setTypeElementType(A_TYPE *t, A_TYPE *s) {
	A_TYPE *q;
	// t 의 마지막 원소의 타입으로 s 타입을 연결
	if (t==NIL)
		return(s);
	q=t;
	while(q->element_type)
		q=q->element_type;
	q->element_type=s;
	return(t);
}

// set type field
A_TYPE *setTypeField(A_TYPE *t, A_ID *n) {
	t->field=n;
	return(t);
}

// set type initial value (expression tree)
A_TYPE *setTypeExpr(A_TYPE *t, A_NODE *n) {
	t->expr=n;
	return(t);
}

// set type of struct iIdentifier
A_TYPE *setTypeStructOrEnumIdentifier(T_KIND k, char *s, ID_KIND kk) {
	A_TYPE *t;
	A_ID *id, *a;
	// check redeclaration or forward declaration
	a=searchIdentifierAtCurrentLevel(s,current_id);
	if (a)
		if (a->kind==kk && a->type->kind==k)
			if (a->type->field)
				syntax_error(12,s);
			else
				return(a->type);
		else
			syntax_error(12,s);
	// make a new struct (or enum) identifier
	id=makeIdentifier(s);
	t=makeType(k);
	id->type=t;
	id->kind=kk;
	return(t);
}

// set type and kinf of identifier
A_TYPE *setTypeAndKindOfDeclarator(A_TYPE *t, ID_KIND k, A_ID *id) {
	if (searchIdentifierAtCurrentLevel(id->name,id->prev))
		syntax_error(12,id->name);
	id->type=t;
	id->kind=k;
	return(t);
}
```

존재하지않아 오류를 발생했던 함수를 추가로 정의하였다.

```c
BOOLEAN	isPointerOrArrayType(A_TYPE *t) {
	if (t && ( t->kind == T_POINTER || t->kind == T_ARRAY))
		return(TRUE);
	else
		return(FALSE);
}
```

이는 syntax1.c 에 함께 이어 선언하였다. 이후 Sementic 분석에서도 사용될 것으로 예상된다.

### 결과

```
sh unit_test.sh

start syntax analysis
=======syntax tree ==========
N_PROGRAM (0,0)
|(ID="main") TYPE:2180b00 KIND:FUNC SPEC=NULL LEV=0 VAL=0 ADDR=0
|| TYPE
|||FUNCTION
|||| PARAMETER
|||| TYPE
|||||(int)
|||| BODY
|||||N_STMT_COMPOUND (0,0)
||||||(ID="a") TYPE:217b470 KIND:VAR SPEC=AUTO LEV=1 VAL=0 ADDR=0
||||||| TYPE
||||||||(int)
||||||| INIT
||||||||N_INIT_LIST_ONE (0,0)
|||||||||N_EXP_INT_CONST (0,0)
||||||||||0
||||||(ID="b") TYPE:217b470 KIND:VAR SPEC=AUTO LEV=1 VAL=0 ADDR=0
||||||| TYPE
||||||||(int)
||||||| INIT
||||||||N_INIT_LIST_ONE (0,0)
|||||||||N_EXP_INT_CONST (0,0)
||||||||||1
||||||(ID="c") TYPE:217b470 KIND:VAR SPEC=AUTO LEV=1 VAL=0 ADDR=0
||||||| TYPE
||||||||(int)
||||||| INIT
||||||||N_INIT_LIST_ONE (0,0)
|||||||||N_EXP_ADD (0,0)
||||||||||N_EXP_IDENT (0,0)
|||||||||||(ID="a") TYPE:217b470 KIND:VAR SPEC=AUTO LEV=1 VAL=0 ADDR=0
||||||||||N_EXP_IDENT (0,0)
|||||||||||(ID="b") TYPE:217b470 KIND:VAR SPEC=AUTO LEV=1 VAL=0 ADDR=0
||||||N_STMT_LIST (0,0)
|||||||N_STMT_IF_ELSE (0,0)
||||||||N_EXP_EQL (0,0)
|||||||||N_EXP_IDENT (0,0)
||||||||||(ID="a") TYPE:217b470 KIND:VAR SPEC=AUTO LEV=1 VAL=0 ADDR=0
|||||||||N_EXP_INT_CONST (0,0)
||||||||||0
||||||||N_STMT_RETURN (0,0)
|||||||||N_EXP_INT_CONST (0,0)
||||||||||1
||||||||N_STMT_RETURN (0,0)
|||||||||N_EXP_INT_CONST (0,0)
||||||||||0
|||||||N_STMT_LIST_NIL (0,0)
rvt_statement_list
```

### 이슈(문제)사항

treeToOrignal.c 작업을 함에 있어서 type.h 헤더파일에 선언한 enum 변수들에 제대로 접근을 하지 못하거나 어떠한 이유에 의해서 switch 문의 분기에 의도대로 동작하는 문제가 발생하였다. 이를 지금 해결하기 위해서 작업이 추가로 필요하다.