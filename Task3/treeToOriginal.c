#include <stdio.h>
#include <stdlib.h>
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
	printf("rvt_statement_list\n");
	switch (node->name) {
		printf("%d\n", node->name);
		case N_STMT_LIST:
			printf("N_STMT_LIST");
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
