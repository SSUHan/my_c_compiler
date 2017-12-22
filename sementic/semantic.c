#include <stdio.h>
#include "type.h"

float	atof();

void		semantic_analysis(A_NODE *);
void		set_literal_address(A_NODE *);
int		put_literal(A_LITERAL, int);
void		sem_program(A_NODE *);

A_TYPE		*sem_expression(A_NODE *);
int		sem_statement(A_NODE *, int, A_TYPE *, BOOLEAN, BOOLEAN, BOOLEAN);
int		sem_statement_list(A_NODE *, int, A_TYPE *, BOOLEAN, BOOLEAN, BOOLEAN);
void		sem_for_expression(A_NODE *);
int		sem_A_TYPE(A_TYPE *) ;
int		sem_declaration_list(A_ID *, int);
int		sem_declaration(A_ID *,int);
void		sem_arg_expr_list(A_NODE *, A_ID *);
A_ID		*getStructFieldIdentifier(A_TYPE *, char *);
A_ID		*getPointerFieldIdentifier(A_TYPE *, char *);
A_NODE		*convertScalarToInteger(A_NODE *);
A_NODE		*convertUsualAssignmentConversion(A_TYPE *, A_NODE *);
A_NODE		*convertUsualUnaryConversion(A_NODE *);
A_TYPE		*convertUsualBinaryConversion(A_NODE *);
A_NODE		*convertCastingConversion(A_NODE *,A_TYPE *);
BOOLEAN		isAllowableAssignmentConversion(A_TYPE *, A_TYPE *, A_NODE *);
BOOLEAN		isAllowableCastingConversion(A_TYPE *, A_TYPE *);
BOOLEAN		isModifiableLvalue(A_NODE *);
BOOLEAN		isConstantZeroExp(A_NODE *);
BOOLEAN		isSameParameterType(A_ID *, A_ID *);
BOOLEAN		isNotSameType(A_TYPE *, A_TYPE *);
BOOLEAN		isCompatibleType(A_TYPE *, A_TYPE *);
BOOLEAN		isCompatiblePointerType(A_TYPE *, A_TYPE *);
BOOLEAN		isIntType(A_TYPE *);
BOOLEAN		isFloatType(A_TYPE *);
BOOLEAN		isArithmeticType(A_TYPE *);
BOOLEAN		isAnyIntegerType(A_TYPE *);
BOOLEAN		isIntegralType(A_TYPE *);
BOOLEAN		isStructOrUnionType(A_TYPE *);
BOOLEAN		isFunctionType(A_TYPE *);
BOOLEAN		isScalarType(A_TYPE *);
BOOLEAN		isPointerType(A_TYPE *);
BOOLEAN		isPointerOrArrayType(A_TYPE *);
BOOLEAN		isArrayType(A_TYPE *);
BOOLEAN		isStringType(A_TYPE *);
BOOLEAN		isVoidType(A_TYPE *);

A_LITERAL	checkTypeAndConvertLiteral(A_LITERAL,A_TYPE*, int);
A_LITERAL	getTypeAndValueOfExpression(A_NODE *);
A_TYPE		*setTypeElementType(A_TYPE *,A_TYPE *);
A_TYPE		*makeType(T_KIND);

void		setTypeSize(A_TYPE *,int);
void		semantic_warning(int, int);
void		semantic_error();
A_NODE		*makeNode(NODE_NAME, A_NODE *, A_NODE *,A_NODE *);

extern A_TYPE *int_type, *float_type, *char_type, *string_type, *void_type;

int global_address = 12;
int semantic_err = 0;

#define LIT_MAX 100
A_LITERAL literal_table[LIT_MAX];
int literal_no = 0;
int literal_size = 0;

void semantic_analysis(A_NODE *node)
{
	sem_program(node);
	set_literal_address(node);
}

void set_literal_address(A_NODE *node)
{
	int i;
	for (i=1;i<=literal_no; i++)
		literal_table[i].addr += node->value;
	node->value += literal_size;
}

void sem_program(A_NODE *node)
{
	int i;
	switch(node->name) {
	case N_PROGRAM :
		i=sem_declaration_list(node->clink, 12);
		node->value=global_address;
		break;
	default :
		semantic_error(90, node->line);
		break;
	}
}

int put_literal(A_LITERAL lit, int ll)
{
	float ff;
	if (literal_no >=LIT_MAX)
		semantic_error(93, ll);
	else
		literal_no++;
	literal_table[literal_no] = lit;
	literal_table[literal_no].addr = literal_size;
	if (lit.type->kind == T_ENUM)
		literal_size += 4;
	else if (isStringType(lit.type))
		literal_size += strlen(lit.value.s)+1;
	if (literal_size % 4)
		literal_size = literal_size/4*4 + 4;
	return(literal_no);
}

A_TYPE *sem_expression(A_NODE *node)
{
	A_TYPE *result = NIL, *t, *t1, *t2;
	A_ID *id;
	A_LITERAL lit;
	int i;
	BOOLEAN lvalue = FALSE;
	switch(node->name) {
	case N_EXP_IDENT :
		id = node->clink;
		switch (id->kind) {
		// 명칭의 종류별로 타입과 lvalue 여부 계산
		case ID_VAR:
		case ID_PARM:
			result=id->type;
			if(!isArrayType(result))
				lvalue=TRUE;
			break;
		case ID_FUNC:
		case ID_ENUM_LITERAL:
			default: semantic_error(38,node->line,id->name);
			break;
		}
		break;
	case N_EXP_INT_CONST :
		result = int_type;
		break;
	case N_EXP_FLOAT_CONST :
		lit.type = float_type;
		lit.value.f = atof(node->clink);
		node->clink = put_literal(lit,node->line); // index of literal table
		result = float_type;
		break;
	case N_EXP_CHAR_CONST :
		result = char_type;
		break;
	case N_EXP_STRING_LITERAL :
		lit.type = string_type;
		lit.value.s = node->clink;
		node->clink = put_literal(lit,node->line); // index of literal table
		result = string_type;
		break;
	case N_EXP_ARRAY :
		t1 = sem_expression(node->llink);
		t2 = sem_expression(node->rlink);
		// usual binary conversion
		t = convertUsualBinaryConversion(node);
		// 배열참조수식의 경우, 좌측 아들이 배열타입이고 우측 아들은 정수형 타
		// 입인지 검사하고 배열참조식의 타입과 lvalue 계산
		t1=node->llink->type;
		t2=node->rlink->type;
		if (isPointerOrArrayType(t1))
			result=t1->element_type;
		else
			semantic_error(32,node->line);
		if(!isIntegralType(t2))
			semantic_error(29,node->line);
		if(!isArrayType(result))
			lvalue=TRUE;
		break;
	case N_EXP_STRUCT :
		t = sem_expression(node->llink);
		// 좌측 아들의 타입이 구조체이고 필드명칭이 있는지 검사
		id = getStructFieldIdentifier(t,node->rlink);
		// 올바른 경우 구조체 참조식의 타입과 lvalue 계산
		if (id) {
			result=id->type;
			if (node->llink->value && !isArrayType(result))
				lvalue=TRUE;
		}
		else
			semantic_error(37,node->line);
		node->rlink=id;
		break;
	case N_EXP_ARROW:
		t = sem_expression(node->llink);
		// 좌측 아들의 타입이 구조체 포인터이고 필드명칭이 있는지 검사
		id = getPointerFieldIdentifier(t,node->rlink);
		// 올바른 경우 구조체 참조식의 타입과 lvalue 계산
		if (id) {
			result=id->type;
			if(!isArrayType(result))
				lvalue=TRUE;
		}
		else
			semantic_error(37,node->line);
		node->rlink = id;
		break;
	case N_EXP_FUNCTION_CALL :
		t = sem_expression(node->llink);
		// usual unary conversion
		node->llink = convertUsualUnaryConversion(node->llink);
		t = node->llink->type;
		if (isPointerType(t) && isFunctionType(t->element_type)) {
			// 함수호출식의 파라미터들이 선언문의 파라미터 선언과 일치하는지 검사
			// 하고 함수의 리턴 타입 계산
			sem_arg_expr_list(node->rlink,t->element_type->field);
			result=t->element_type->element_type;
		}
		else
			semantic_error(21,node->line);
		break;
	case N_EXP_POST_INC :
	case N_EXP_POST_DEC :
		result = sem_expression(node->clink);
		// 아들수식의 타입이 스칼라 타입이고 modifiable lvalue 인지 검사
		if(!isScalarType(result))
			semantic_error(27,node->line);
		if(!isModifiableLvalue(node->clink))
			semantic_error(60,node->line);
		break;
	case N_EXP_CAST :
		result = node->llink;
		i = sem_A_TYPE(result);
		t = sem_expression(node->rlink);
		// check allowable casting conversion
		if (!isAllowableCastingConversion(result,t))
			semantic_error(58,node->line);
		break;
	case N_EXP_SIZE_TYPE :
		t = node->clink;
		i = sem_A_TYPE(t);
		// incomplete array, function, void 타입인지 검사
		if (isArrayType(t) && t->size==0 || isFunctionType(t) || isVoidType(t))
			semantic_error(39,node->line);
		else
			node->clink=i;
		result = int_type;
		break;
	case N_EXP_SIZE_EXP :
		t = sem_expression(node->clink);
		// incomplete array, function (for non parameter identifier)
		if ((node->clink->name!=N_EXP_IDENT ||
			((A_ID*)node->clink->clink)->kind!=ID_PARM) &&
			(isArrayType(t) && t->size==0 || isFunctionType(t)))
			semantic_error(39,node->line);
		else
			node->clink = t->size;
		result = int_type;
		break;
	case N_EXP_PLUS :
	case N_EXP_MINUS :
		t=sem_expression(node->clink);
		if (isArithmeticType(t)) {
			node->clink=convertUsualUnaryConversion(node->clink);
			result=node->clink->type;
		}
		else
			semantic_error(13,node->line);
		break;
	case N_EXP_NOT :
		t = sem_expression(node->clink);
		if (isScalarType(t)) {
			node->clink = convertUsualUnaryConversion(node->clink);
			result = int_type;
		}
		else
			semantic_error(27,node->line);
		break;
	case N_EXP_AMP :
		t = sem_expression(node->clink);
		// 아들이 lvalue 이거나 함수인 경우에만 수식의 타입결정
		if (node->clink->value==TRUE || isFunctionType(t)) {
			result=setTypeElementType(makeType(T_POINTER),t);
			result->size=4;
		}
		else
			semantic_error(60,node->line);
		break;
	case N_EXP_STAR :
		t = sem_expression(node->clink);
		node->clink = convertUsualUnaryConversion(node->clink);
		if (isPointerType(t)) {
			result = t->element_type;
			// lvalue if points to an object
			if (isStructOrUnionType(result) || isScalarType(result))
				lvalue = TRUE;
		}
		else
			semantic_error(31,node->line);
		break;
	case N_EXP_PRE_INC :
	case N_EXP_PRE_DEC :
		result = sem_expression(node->clink);
		// usual binary conversion between the expression and 1
		if (!isScalarType(result))
			semantic_error(27,node->line);
		// check if modifiable lvalue
		if (!isModifiableLvalue(node->clink))
			semantic_error(60,node->line);
		break;
	case N_EXP_MUL :
	case N_EXP_DIV :
		t1 = sem_expression(node->llink);
		t2 = sem_expression(node->rlink);
		if (isArithmeticType(t1) && isArithmeticType(t2)) {
			result=convertUsualBinaryConversion(node);
		}
		else
			semantic_error(28,node->line);
		break;
	case N_EXP_MOD :
		t1 = sem_expression(node->llink);
		t2 = sem_expression(node->rlink);
		if (isIntegralType(t1) && isIntegralType(t2))
			result=convertUsualBinaryConversion(node);
		else
			semantic_error(29,node->line);
		result = int_type;
		break;
	case N_EXP_ADD :
		t1 = sem_expression(node->llink);
		t2 = sem_expression(node->rlink);
		if (isArithmeticType(t1) && isArithmeticType(t2))
			result = convertUsualBinaryConversion(node);
		else if (isPointerType(t1) && isIntegralType(t2))
			result = t1;
		else if (isIntegralType(t1) && isPointerType(t2))
			result = t2;
		else
			semantic_error(24,node->line);
		break;
	case N_EXP_SUB :
		t1 = sem_expression(node->llink);
		t2 = sem_expression(node->rlink);
		// 빼기 연산이 가능한지 검사하고 타입에 따른 변환 작업 수행
		if (isArithmeticType(t1) && isArithmeticType(t2))
			result = convertUsualBinaryConversion(node);
		else if (isPointerType(t1) && isIntegralType(t2))
			result=t1;
		else if(isCompatiblePointerType(t1,t2))
			result=t1;
		else
			semantic_error(24,node->line);
		break;
	case N_EXP_LSS :
	case N_EXP_GTR :
	case N_EXP_LEQ :
	case N_EXP_GEQ :
		t1 = sem_expression(node->llink);
		t2 = sem_expression(node->rlink);
		if (isArithmeticType(t1) && isArithmeticType(t2))
			t = convertUsualBinaryConversion(node);
		else if (!isCompatiblePointerType(t1,t2))
			semantic_error(40,node->line);
		result = int_type;
		break;
	case N_EXP_NEQ :
	case N_EXP_EQL :
		t1 = sem_expression(node->llink);
		t2 = sem_expression(node->rlink);
		if (isArithmeticType(t1) && isArithmeticType(t2))
			t = convertUsualBinaryConversion(node);
		else if (!isCompatiblePointerType(t1,t2) &&
			(!isPointerType(t1) || !isConstantZeroExp(node->rlink)) &&
			(!isPointerType(t2) || !isConstantZeroExp(node->llink)))
			semantic_error(40,node->line);
		result = int_type;
		break;
	case N_EXP_AND :
	case N_EXP_OR :
		t = sem_expression(node->llink);
		if (isScalarType(t))
			node->llink = convertUsualUnaryConversion(node->llink);
		else
			semantic_error(27,node->line);
		t = sem_expression(node->rlink);
		if (isScalarType(t))
			node->rlink = convertUsualUnaryConversion(node->rlink);
		else
			semantic_error(27,node->line);
		result = int_type;
		break;
	case N_EXP_ASSIGN :
		result = sem_expression(node->llink);
		// check if modifiable lvalue
		if (!isModifiableLvalue(node->llink))
			semantic_error(60,node->line);
		t = sem_expression(node->rlink);
		node->rlink = convertUsualUnaryConversion(node->rlink);
		t = node->rlink->type;
		if (isAllowableAssignmentConversion(result,t,node->rlink)){
			if (isArithmeticType(result) && isArithmeticType(t))
				node->rlink = convertUsualAssignmentConversion(result,node->rlink);
		}
		else
			semantic_error(58,node->line);
		break;
	default :
		semantic_error(90,node->line);
		break;
	}
	node->type = result;
	node->value = lvalue;
	return (result);
}

// check argument-expression-list in function call expression
void sem_arg_expr_list(A_NODE *node, A_ID *id)
{
	A_TYPE *t;
	A_ID *a;
	int arg_size = 0;
	// 함수참조식의 실 매개변수(수식) 목록이 함수선언문의 가 매개변수 목록과 개수와
	// 각각의 타입이 일치하는지 검사 하고 매개변수의 크기 계산하여 저장
	switch(node->name) {
		case N_ARG_LIST:
			if(id==0)
				semantic_error(34,node->line);
			else {
				if(id->type){
					t=sem_expression(node->llink);
					node->llink=convertUsualUnaryConversion(node->llink);
					if(isAllowableCastingConversion(id->type,node->llink->type))
						node->llink=convertCastingConversion(node->llink,id->type);
					else
						semantic_error(59,node->line);
					sem_arg_expr_list(node->rlink,id->link);
				} else {
					t=sem_expression(node->llink);
					sem_arg_expr_list(node->rlink,id);
				}
				arg_size=node->llink->type->size+node->rlink->value;
			}
			break;
		case N_ARG_LIST_NIL:
			if(id && id->type)
				semantic_error(35,node->line);
			break;
		default:
			semantic_error(90,node->line);
			break;
	}
	if (arg_size%4)
		arg_size = arg_size/4*4+4;
	node->value = arg_size;
}

BOOLEAN isModifiableLvalue(A_NODE *node)
{
	if(node->value == FALSE
		|| isVoidType(node->type)
		|| isFunctionType(node->type))
		return (FALSE);
	else
		return (TRUE);
}

// check statement and return local variable size
int sem_statement(A_NODE *node, int addr, A_TYPE *ret, BOOLEAN sw, BOOLEAN brk, BOOLEAN cnt)
{
	int local_size = 0, i;
	A_LITERAL lit;
	A_TYPE *t;
	switch(node->name) {
	case N_STMT_LABEL_CASE :
		// case 레이블이 나와도 되는지 검사
		if (sw==FALSE)
			semantic_error(71,node->line);
		lit = getTypeAndValueOfExpression(node->llink);
		if (isIntegralType(lit.type))
			node->llink = lit.value.i;
		else
			semantic_error(51,node->line);
		local_size = sem_statement(node->rlink,addr,ret,sw,brk,cnt);
		break;
	case N_STMT_LABEL_DEFAULT :
		// case 레이블이 나와도 되는지 검사
		if (sw==FALSE)
			semantic_error(72,node->line);
		local_size = sem_statement(node->clink,addr,ret,sw,brk,cnt);
		break;
	case N_STMT_COMPOUND:
		if(node->llink)
			local_size = sem_declaration_list(node->llink,addr);
		local_size += sem_statement_list(node->rlink,local_size+addr,ret,sw,brk,cnt);
		break;
	case N_STMT_EMPTY:
		break;
	case N_STMT_EXPRESSION:
		t = sem_expression(node->clink);
		break;
	case N_STMT_IF:
		t = sem_expression(node->llink);
		if (isScalarType(t))
			node->llink=convertScalarToInteger(node->llink);
		else
			semantic_error(50,node->line);
		local_size = sem_statement(node->rlink,addr,ret,FALSE,brk,cnt);
		break;
	case N_STMT_IF_ELSE:
		t = sem_expression(node->llink);
		if (isScalarType(t))
			node->llink = convertScalarToInteger(node->llink);
		else
			semantic_error(50,node->line);
		local_size = sem_statement(node->clink,addr,ret,FALSE,brk,cnt);
		i = sem_statement(node->rlink,addr,ret,FALSE,brk,cnt);
		if (local_size < i)
			local_size = i;
		break;
	case N_STMT_SWITCH:
		t = sem_expression(node->llink);
		if (!isIntegralType(t))
			semantic_error(50,node->line);
		local_size = sem_statement(node->rlink,addr,ret,TRUE,TRUE,cnt);
		break;
	case N_STMT_WHILE:
		t = sem_expression(node->llink);
		if (isScalarType(t))
			node->llink = convertScalarToInteger(node->llink);
		else
			semantic_error(50,node->line);
		local_size = sem_statement(node->rlink,addr,ret,FALSE,TRUE,TRUE);
		break;
	case N_STMT_DO:
		local_size = sem_statement(node->llink,addr,ret,FALSE,TRUE,TRUE);
		t = sem_expression(node->rlink);
		if (isScalarType(t))
			node->rlink = convertScalarToInteger(node->rlink);
		else
			semantic_error(50,node->line);
		break;
	case N_STMT_FOR:
		sem_for_expression(node->llink);
		local_size = sem_statement(node->rlink,addr,ret,FALSE,TRUE,TRUE);
		break;
	case N_STMT_CONTINUE:
		if (cnt == FALSE)
			semantic_error(74,node->line);
		break;
	case N_STMT_BREAK:
		if (brk == FALSE)
			semantic_error(73,node->line);
		break;
	case N_STMT_RETURN:
		if (node->clink) {
			t = sem_expression(node->clink);
			if (isAllowableCastingConversion(ret,t))
				node->clink = convertCastingConversion(node->clink,ret);
			else
				semantic_error(57,node->line);
		}
		break;
	default:
		semantic_error(90,node->line);
		break;
	}
	node->value = local_size;
	return(local_size);
}

void sem_for_expression(A_NODE *node)
{
	A_TYPE *t;
	switch (node->name) {
	case N_FOR_EXP :
		if (node->llink)
			t = sem_expression(node->llink);
		if (node->clink) {
			t = sem_expression(node->clink);
			if (isScalarType(t))
				node->clink = convertScalarToInteger(node->clink);
			else
				semantic_error(49,node->line);
		}
		if(node->rlink)
			t = sem_expression(node->rlink);
		break;
	default :
		semantic_error(90,node->line);
		break;
	}
}

// check statement-list and return local variable size
int sem_statement_list(A_NODE *node, int addr, A_TYPE *ret, BOOLEAN sw, BOOLEAN brk, BOOLEAN cnt)
{
	int size, i;
	switch (node->name) {
	case N_STMT_LIST:
		size = sem_statement(node->llink, addr,ret,sw,brk,cnt);
		i = sem_statement_list(node->rlink, addr,ret,sw,brk,cnt);
		if(size < i)
			size = i;
		break;
	case N_STMT_LIST_NIL:
		size = 0;
		break;
	default :
		semantic_error(90,node->line);
		break;
	}
	node->value = size;
	return(size);
}

// check type and return its size (size of incomplete type is 0)
int sem_A_TYPE(A_TYPE *t)
{
	A_ID *id;
	A_TYPE *tt;
	A_LITERAL lit;
	int result = 0, i;
	if (t->check)
		return(t->size);
	t->check = 1;
	switch (t->kind) {
	case T_NULL:
		semantic_error(80,t->line);
		break;
	case T_ENUM:
		i = 0;
		id = t->field;
		while (id) { // enumerators
			if (id->init) {
				// 명칭상수에 딸린 수식의 값과 타입을 계산
				lit=getTypeAndValueOfExpression(id->init);
				if(!isIntType(lit.type))
					semantic_error(81,id->line);
				i=lit.value.i;
			}
			id->init = i++;
			id = id->link;
		}
		result = 4;
		break;
	case T_ARRAY:
		if (t->expr) {
			lit = getTypeAndValueOfExpression(t->expr);
			if (!isIntType(lit.type) || lit.value.i<=0) {
				semantic_error(82,t->line);
				t->expr=0;
			}
			else
				t->expr=lit.value.i;
		}
		i = sem_A_TYPE(t->element_type)*(int)t->expr;
		if (isVoidType(t->element_type) || isFunctionType(t->element_type))
			semantic_error(83,t->line);
		else
			result = i;
		break;
	case T_STRUCT:
		id = t->field;
		while (id) {
			result += sem_declaration(id,result);
			id = id->link; 
		}
		break;
	case T_UNION:
		id = t->field;
		while (id) {
			i = sem_declaration(id,0);
			if (i > result) {
				result = i;
				id = id->link;
			}
		}
		break;
	case T_FUNC:
		tt = t->element_type;
		i = sem_A_TYPE(tt);
		if (isArrayType(tt) || isFunctionType(tt)) // check function return type
			semantic_error(85,t->line);
		i = sem_declaration_list(t->field,12) + 12; // parameter type and size
		if (t->expr) { // skip prototype declaration
			i = i + sem_statement(t->expr,i,t->element_type,FALSE,FALSE,FALSE);
		}
		t->local_var_size = i;
		break;
	case T_POINTER:
		i = sem_A_TYPE(t->element_type);
		result = 4;
		break;
	case T_VOID:
		break;
	default:
		semantic_error(90,t->line);
		break;
	}
	t->size = result;
	return(result);
}

// set variable address in declaration-list, and return its total variable size
int sem_declaration_list(A_ID *id, int addr)
{
	int i = addr;
	while (id) {
		addr += sem_declaration(id,addr);
		id = id->link;
	}
	return(addr - i);
}

// check declaration (identifier), set address, and return its size
int sem_declaration(A_ID *id,int addr)
{
	A_TYPE *t;
	int size = 0, i;
	A_LITERAL lit;
	switch (id->kind) {
	case ID_VAR:
		i = sem_A_TYPE(id->type);
		// check empty array
		if(isArrayType(id->type) && id->type->expr==NIL)
			semantic_error(86,id->line);
		if (i % 4)
			i = i/4*4 + 4;
		if (id->specifier == S_STATIC)
			id->level=0;
		if (id->level == 0) {
			id->address = global_address;
			global_address += i;
		}
		else {
			id->address=addr;
			size=i;
		}
		break;
	case ID_FIELD:
		i = sem_A_TYPE(id->type);
		if (isFunctionType(id->type)
			|| isVoidType(id->type))
			semantic_error(84,id->line);
		if (i % 4)
			i = i/4*4 + 4;
		id->address = addr;
		size = i;
		break;
	case ID_FUNC:
		i = sem_A_TYPE(id->type);
		break;
	case ID_PARM:
		if (id->type) {
			size = sem_A_TYPE(id->type);
			// usual unary conversion of parm type
			if (id->type == char_type)
				id->type = int_type;
			else if (isArrayType(id->type)) {
				id->type->kind = T_POINTER;
				id->type->size = 4;
			}
			else if (isFunctionType(id->type)) {
				t = makeType(T_POINTER);
				t->element_type = id->type;
				t->size = 4;
				id->type = t;
			}
			size = id->type->size;
			if (size % 4)
				size = size/4*4 + 4;
			id->address=addr;
		}
		break;
	case ID_TYPE:
		i = sem_A_TYPE(id->type);
		break;
	default:
		semantic_error(89, id->line, id->name);
		break;
	}
	return (size);
}

A_ID *getStructFieldIdentifier(A_TYPE *t, char *s)
{
	A_ID *id = NIL;
	if (isStructOrUnionType(t)) {
		id = t->field;
		while (id) {
			if (strcmp(id->name,s) == 0)
				break;
			id=id->link;
		}
	}
	return(id);
}

A_ID *getPointerFieldIdentifier(A_TYPE *t, char *s)
{
	A_ID *id = NIL;
	if (t && t->kind == T_POINTER) {
		t = t->element_type;
		if (isStructOrUnionType(t)) {
			id = t->field;
			while (id) {
				if (strcmp(id->name,s) == 0)
					break;
				id=id->link;
			}
		}
	}
	return(id);
}

BOOLEAN isSameParameterType(A_ID *a, A_ID *b)
{
	while (a) {
		if (b==NIL || isNotSameType(a->type,b->type))
			return (FALSE);
		a = a->link;
		b = b->link;
	}
	if (b)
		return (FALSE);
	else
		return (TRUE);
}

BOOLEAN isCompatibleType(A_TYPE *t1, A_TYPE *t2)
{
	// 두 개의 타입 t1 과 t2 가 compatible 한지를 검사
	if (isArrayType(t1) && isArrayType(t2))
		if (t1->size==0 || t2->size==0 || t1->size==t2->size)
			return(isCompatibleType(t1->element_type,t2->element_type));
		else
			return(FALSE);
	else if (isFunctionType(t1) && isFunctionType(t2))
		if (isSameParameterType(t1->field,t2->field))
			return(isCompatibleType(t1->element_type,t2->element_type));
		else
			return(FALSE);
	else if (isPointerType(t1) && isPointerType(t2))
		return(isCompatibleType(t1->element_type,t2->element_type));
	else
		return(t1==t2);
}

BOOLEAN isConstantZeroExp(A_NODE *node)
{
	if (node->name == N_EXP_INT_CONST && node->clink==0)
		return (TRUE);
	else
		return (FALSE);
}

BOOLEAN isCompatiblePointerType(A_TYPE *t1, A_TYPE *t2)
{
	if (isPointerType(t1) && isPointerType(t2))
		return(isCompatibleType(t1->element_type, t2->element_type));
	else
		return(FALSE);
}

A_NODE *convertScalarToInteger(A_NODE *node)
{
	if (isFloatType(node->type)) {
		semantic_warning(16,node->line);
		node = makeNode(N_EXP_CAST,int_type,NIL,node);
	}
	node->type = int_type;
	return(node);
}

A_NODE *convertUsualAssignmentConversion(A_TYPE *t1, A_NODE *node)
{
	A_TYPE *t2;
	t2 = node->type;
	if (!isCompatibleType(t1,t2)) {
		semantic_warning(11,node->line);
		node = makeNode(N_EXP_CAST,t1,NIL,node);
		node->type = t1;
	}
	return (node);
}

A_NODE *convertUsualUnaryConversion(A_NODE *node)
{
	A_TYPE *t;
	t = node->type;
	if (t == char_type) {
		t = int_type;
		node = makeNode(N_EXP_CAST,t,NIL,node);
		node->type = t;
	}
	else if (isArrayType(t)) {
		t = setTypeElementType(makeType(T_POINTER),t->element_type);
		t->size = 4;
		node = makeNode(N_EXP_CAST,t,NIL,node);
		node->type = t;
	}
	else if (isFunctionType(t)) {
		t = setTypeElementType(makeType(T_POINTER),t);
		t->size = 4;
		node = makeNode(N_EXP_AMP,NIL,node,NIL);
		node->type = t;
	}
	return (node);
}

A_TYPE *convertUsualBinaryConversion(A_NODE *node)
{
	A_TYPE *t1, *t2, *result = NIL;
	t1 = node->llink->type;
	t2 = node->rlink->type;
	if (isFloatType(t1) && !isFloatType(t2)) {
		semantic_warning(14,node->line);
		node->rlink = makeNode(N_EXP_CAST,t1,NIL,node->rlink);
		node->rlink->type = t1;
		result = t1;
	}
	else if (!isFloatType(t1) && isFloatType(t2)) {
		semantic_warning(14,node->line);
		node->llink = makeNode(N_EXP_CAST,t2,NIL,node->llink);
		node->llink->type = t2;
		result = t2;
	}
	else if (t1 == t2)
		result = t1;
	else
		result = int_type;
	return (result);
}

A_NODE *convertCastingConversion(A_NODE *node,A_TYPE *t1)
{
	A_TYPE *t2;
	t2 = node->type;
	if (!isCompatibleType(t1,t2)) {
		semantic_warning(12,node->line);
		node = makeNode(N_EXP_CAST,t1,NIL,node);
		node->type = t1;
	}
	return (node);
}

BOOLEAN isAllowableAssignmentConversion(A_TYPE *t1, A_TYPE *t2, A_NODE *node) // t1 <--- t2
{
	if (isArithmeticType(t1) && isArithmeticType(t2))
		return (TRUE);
	else if (isStructOrUnionType(t1) && isCompatibleType(t1,t2))
		return (TRUE);
	else if (isPointerType(t1) && (isConstantZeroExp(node) ||
		isCompatiblePointerType(t1,t2)))
		return (TRUE);
	else
		return (FALSE);
}

BOOLEAN isAllowableCastingConversion(A_TYPE *t1, A_TYPE *t2) // t1 <--- t2
{
	if (isAnyIntegerType(t1)
		&& (isAnyIntegerType(t2)
		|| isFloatType(t2)
		|| isPointerType(t2)))
		return (TRUE);
	else if (isFloatType(t1) && isArithmeticType(t2))
		return (TRUE);
	else if (isPointerType(t1) && (isAnyIntegerType(t2) || isPointerType(t2)))
		return (TRUE);
	else if (isVoidType(t1))
		return (TRUE);
	else
		return (FALSE);
}

BOOLEAN isFloatType(A_TYPE *t)
{
	if (t == float_type)
		return(TRUE);
	else
		return(FALSE);
}

BOOLEAN isArithmeticType(A_TYPE *t)
{
	if (t && t->kind == T_ENUM)
		return(TRUE);
	else
		return(FALSE);
}

BOOLEAN isScalarType(A_TYPE *t)
{
	if (t && ((t->kind == T_ENUM) || (t->kind == T_POINTER)))
		return(TRUE);
	else
		return(FALSE);
}

BOOLEAN isAnyIntegerType(A_TYPE *t)
{
	if (t && (t == int_type || t == char_type))
		return(TRUE);
	else
		return(FALSE);
}

BOOLEAN isIntegralType(A_TYPE *t)
{
	if (t && t->kind == T_ENUM && t != float_type)
		return(TRUE);
	else
		return(FALSE);
}

BOOLEAN isFunctionType(A_TYPE *t)
{
	if (t && t->kind == T_FUNC)
		return(TRUE);
	else
		return(FALSE);
}

BOOLEAN isStructOrUnionType(A_TYPE *t)
{
	if (t && (t->kind == T_STRUCT || t->kind == T_UNION))
		return(TRUE);
	else
		return(FALSE);
}

BOOLEAN isPointerType(A_TYPE *t)
{
	if (t && t->kind == T_POINTER)
		return(TRUE);
	else
		return(FALSE);
}


BOOLEAN isIntType(A_TYPE *t)
{
	if (t && t == int_type)
		return(TRUE);
	else
		return(FALSE);
}

BOOLEAN isVoidType(A_TYPE *t)
{
	if (t && t == void_type)
		return(TRUE);
	else
		return(FALSE);
}

BOOLEAN isArrayType(A_TYPE *t)
{
	if (t && t->kind == T_ARRAY)
		return(TRUE);
	else
		return(FALSE);
}

BOOLEAN isStringType(A_TYPE *t)
{
	if (t && (t->kind == T_POINTER || t->kind == T_ARRAY) && t->element_type == char_type)
		return(TRUE);
	else
		return(FALSE);
}

// convert literal type
A_LITERAL checkTypeAndConvertLiteral(A_LITERAL result,A_TYPE *t, int ll)
{
	if (result.type == int_type 
		&& t == int_type 
		|| result.type == char_type 
		&& t == char_type 
		|| result.type == float_type 
		&& t == float_type )
		;
	else if (result.type == int_type && t == float_type) {
		result.type = float_type;
		result.value.f = result.value.i;
	}
	else if (result.type == int_type && t == char_type) {
		result.type = char_type;
		result.value.c = result.value.i;
	}
	else if (result.type == float_type && t == int_type) {
		result.type = int_type;
		result.value.i = result.value.f;
	}
	else if (result.type == char_type && t == int_type) {
		result.type = int_type;
		result.value.i = result.value.c;
	}
	else
		semantic_error(41,ll);
	return (result);
}

A_LITERAL getTypeAndValueOfExpression(A_NODE *node)
{
	A_TYPE *t;
	A_ID *id;
	A_LITERAL result,r;
	result.type = NIL;
	switch(node->name) {
	case N_EXP_IDENT :
		id = node->clink;
		if (id->kind != ID_ENUM_LITERAL)
			semantic_error(19, node->line, id->name);
		else {
			result.type = int_type;
			result.value.i = id->init;
		}
		break;
	case N_EXP_INT_CONST :
		result.type = int_type;
		result.value.i = (int)node->clink;
		break;
	case N_EXP_CHAR_CONST :
		result.type = char_type;
		result.value.c = (char)node->clink;
		break;
	case N_EXP_FLOAT_CONST :
		result.type = float_type;
		result.value.f = atof(node->clink);
		break;
	case N_EXP_STRING_LITERAL :
	case N_EXP_ARRAY :
	case N_EXP_FUNCTION_CALL :
	case N_EXP_STRUCT :
	case N_EXP_ARROW :
	case N_EXP_POST_INC :
	case N_EXP_PRE_INC :
	case N_EXP_POST_DEC :
	case N_EXP_PRE_DEC :
	case N_EXP_AMP :
	case N_EXP_STAR :
	case N_EXP_NOT :
		semantic_error(18, node->line);
		break;
	case N_EXP_MINUS :
		result = getTypeAndValueOfExpression(node->clink);
		if (result.type == int_type)
			result.value.i = -result.value.i;
		else if (result.type==float_type)
			result.value.f = -result.value.f;
		else
			semantic_error(18,node->line);
		break;
	case N_EXP_SIZE_EXP :
		t = sem_expression(node->clink);
		result.type = int_type;
		result.value.i = t->size;
		break;
	case N_EXP_SIZE_TYPE :
		result.type = int_type;
		result.value.i = sem_A_TYPE(node->clink);
		break;
	case N_EXP_CAST :
		result = getTypeAndValueOfExpression(node->rlink);
		result = checkTypeAndConvertLiteral(result, (A_TYPE*)node->llink, node->line);
		break;
	case N_EXP_MUL :
		result = getTypeAndValueOfExpression(node->llink);
		r = getTypeAndValueOfExpression(node->rlink);
		if (result.type == int_type && r.type == int_type) {
			result.type = int_type;
			result.value.i = result.value.i * r.value.i;
		}
		else if (result.type == int_type && r.type == float_type) {
			result.type = float_type;
			result.value.f = result.value.i * r.value.f;
		}
		else if (result.type == float_type && r.type == int_type) {
			result.type = float_type;
			result.value.f = result.value.f * r.value.i;
		}
		else if (result.type == float_type && r.type == float_type) {
			result.type = float_type;
			result.value.f = result.value.f * r.value.f;
		}
		else
			semantic_error(18,node->line);
		break;
	case N_EXP_DIV :
		result	= getTypeAndValueOfExpression(node->llink);
		r = getTypeAndValueOfExpression(node->rlink);
		if (result.type == int_type && r.type == int_type) {
			result.type = int_type;
			result.value.i = result.value.i / r.value.i;
		}
		else if (result.type == int_type && r.type == float_type) {
			result.type = float_type;
			result.value.f = result.value.i / r.value.f;
		}
		else if (result.type == float_type && r.type == int_type) {
			result.type = float_type;
			result.value.f = result.value.f / r.value.i;
		}
		else if (result.type == float_type && r.type == float_type) {
			result.type = float_type;
			result.value.f = result.value.f / r.value.f;
		}
		else
			semantic_error(18,node->line);
		break;
	case N_EXP_MOD :
		result = getTypeAndValueOfExpression(node->llink);
		r = getTypeAndValueOfExpression(node->rlink);
		if (result.type == int_type && r.type == int_type)
			result.value.i = result.value.i % r.value.i;
		else
			semantic_error(18,node->line);
		break;
	case N_EXP_ADD :
		result = getTypeAndValueOfExpression(node->llink);
		r = getTypeAndValueOfExpression(node->rlink);
		if (result.type == int_type && r.type == int_type) {
			result.type = int_type;
			result.value.i = result.value.i + r.value.i;
		}
		else if (result.type == int_type && r.type == float_type) {
			result.type = float_type;
			result.value.f = result.value.i + r.value.f;
		}
		else if (result.type == float_type && r.type == int_type) {
			result.type = float_type;
			result.value.f = result.value.f + r.value.i;
		}
		else if (result.type == float_type && r.type == float_type) {
			result.type = float_type;
			result.value.f = result.value.f + r.value.f;
		}
		else
			semantic_error(18,node->line);
		break;
	case N_EXP_SUB :
		result = getTypeAndValueOfExpression(node->llink);
		r = getTypeAndValueOfExpression(node->rlink);
		if (result.type == int_type && r.type == int_type) {
			result.type = int_type;
			result.value.i = result.value.i - r.value.i;
		}
		else if (result.type == int_type && r.type == float_type) {
			result.type = float_type;
			result.value.f = result.value.i - r.value.f;
		}
		else if (result.type == float_type && r.type == int_type) {
			result.type = float_type;
			result.value.f = result.value.f - r.value.i;
		}
		else if (result.type == float_type && r.type == float_type){
			result.type = float_type;
			result.value.f = result.value.f - r.value.f;
		}
		else
			semantic_error(18,node->line);
		break;
	case N_EXP_LSS :
	case N_EXP_GTR :
	case N_EXP_LEQ :
	case N_EXP_GEQ :
	case N_EXP_NEQ :
	case N_EXP_EQL :
	case N_EXP_AND :
	case N_EXP_OR :
	case N_EXP_ASSIGN :
		semantic_error(18,node->line);
		break;
	default :
		semantic_error(90,node->line);
		break;
	}
	return (result);
}

void semantic_error(int i, int ll, char *s)
{
	semantic_err++;
	printf("*** semantic error at line %d: ",ll);
	switch (i) {
	// errors in expression
	case 13:
		printf("arithmetic type expression required in unary operation\n");
		break;
	case 18:
		printf("illegal constant expression \n");
		break;
	case 19:
		printf("illegal identifier %s in constant expression\n",s);
		break;
	case 21:
		printf("illegal type in function call expression\n");
		break;
	case 24:
		printf("incompatible type in additive expression\n");
		break;
	case 27:
		printf("scalar type expression required in unary or logial-or operation\n");
		break;
	case 28:
		printf("arithmetic type expression required in binary operation\n");
		break;
	case 29:
		printf("integral type expression required in array subscript or binary operation\n");
		break;
	case 31:
		printf("pointer type expression required in pointer operation\n");
		break;
	case 32:
		printf("array type required in array expression\n");
		break;
	case 34:
		printf("too many arguments in function call\n");
		break;
	case 35:
		printf("too few arguments in function call\n");
		break;
	case 37:
		printf("illegal struct field identifier in struct reference expression\n");
		break;
	case 38:
		printf("illegal kind of identifier %s in expression\n");
		break;
	case 39:
		printf("illegal type size in sizeof operation\n");
		break;
	case 40:
		printf("illegal expression type in relational operation");
		break;
//	case 41:
//		printf("incompatible type in literal\n");
//		break;
	case 49:
		printf("scalar type expression required in middle of for-expression\n");
		break;
	case 50:
		printf("integral type expression required in selective or iterative statement\n");
		break;
	case 51:
		printf("illegal expression type in case label\n");
		break;
	case 57:
		printf("not permitted type conversion in return expression\n");
		break;
	case 58:
		printf("not permitted type casting in assignment or type casting expression\n");
		break;
	case 59:
		printf("not permitted type conversion in argument\n");
		break;
	case 60:
		printf("expression is not an lvalue \n");
		break;
	case 71:
		printf("case label not within a switch statement \n");
		break;
	case 72:
		printf("default label not within a switch statement \n");
		break;
	case 73:
		printf("break statement not within loop or switch statement \n");
		break;
	case 74:
		printf("continue statement not within a loop \n");
		break;
	case 80:
		printf("undefined type\n");
		break;
	case 81:
		printf("integer type expression required in enumerator\n");
		break;
	case 82:
		printf("illegal array size or type\n");
		break;
	case 83:
		printf("illegal element type of array declarator\n");
		break;
	case 84:
		printf("illegal type in struct or union field\n");
		break;
	case 85:
		printf("invalid function return type\n");
		break;
	case 86:
		printf("illegal array size or empty array \n");
		break;
	case 89:
		printf("unknown identifier kind: %s\n",s);
		break;
	case 90:
		printf("fatal compiler error in parse result\n");
		break;
	case 93:
		printf("too many literals in source program \n");
		break;
	default:
		printf("unknown \n");
		break;
	}
}

void semantic_warning(int i, int ll)
{
	printf("--- warning at line %d:",ll);
	switch (i) {
	case 11:
		printf("incompatible types in assignment expression\n");
		break;
	case 12:
		printf("incompatible types in argument or return expression\n");
		break;
	case 14:
		printf("incompatible types in binary expression\n");
		break;
	case 16:
		printf("integer type expression is required\n");
		break;
	default:
		printf("unknown\n");
		break;
	}
}
