#include <stdio.h>
#include <stdlib.h>
#include "type.h"

void statement(A_NODE *node);
void statement_list(A_NODE *node);
void expression(A_NODE *node);
void prt_for_expression(A_NODE *node);

void statement(A_NODE *node) {
	switch (node->name){
		case N_STMT_LABEL_CASE:
			printf("case ");
			expression(node->llink);
			printf(": ");
			statement(node->rlink);
			break;
		case N_STMT_LABEL_DEFAULT:
			printf("default: ");
			statement(node->clink);
			break;
		case N_STMT_COMPOUND:
			printf("{ ");
			if(node->llink)
				A_ID_LIST(node->llink);
			statement_list(node->rlink);
			printf("} ");
			break;
		case N_STMT_EMPTY:
			printf("; ");
			break;
		case N_STMT_EXPRESSION:
			expression(node->clink);
			printf("; ");
			break;
		case N_STMT_IF_ELSE:
			printf("if ( ");
			expression(node->llink);
			printf(") ");
			statement(node->clink);
			printf("else ");
			statement(node->rlink);
			break;
		case N_STMT_IF:
			printf("if ( ");
			expression(node->llink);
			printf(") ");
			statement(node->rlink);
			break;
		case N_STMT_SWITCH:
			printf("switch ( ");
			expression(node->llink);
			printf(") ");
			statement(node->rlink);
			break;
		case N_STMT_WHILE:
			printf("while ( ");
			expression(node->llink);
			printf(") ");
			statement(node->rlink);
			break;
		case N_STMT_DO:
			printf("do ");
			statement(node->llink);
			printf("while ( ");
			expression(node->rlink);
			printf(") ");
			break;
		case N_STMT_FOR:
			printf("for ( ");
			for_expression(node->llink);
			printf(") ");
			statement(node->rlink);
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
				prt_expression(node->clink);
			printf("; ");
			break;
	}
}

void statement_list(A_NODE *node){
	switch (node->name) {
		case N_STMT_LIST:
			statement(node->llink);
			statement_list(node->rlink);
			break;
		case N_STMT_LIST_NIL:
			break;
	}
}

void prt_for_expression(A_NODE *node) {
	switch (node->name) {
		case N_FOR_EXP:
			if(node->llink)
				expression(node->llink);
			printf("; ");
			if(node->clink)
				expression(node->clink);
			printf("; ");
			if(node->rlink)
				expression(node->rlink);
			break;
	}
}	

void expression(A_NODE *node){
	printf("[Expression]");
}