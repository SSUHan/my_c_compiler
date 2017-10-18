%{
#include <stdio.h>
#include <stdlib.h>
%}

%start S
%token NUM, PLUS, STAR, LPAREN, RPAREN, NEW_LINE

%%
S 	: 	E NEW_LINE 			{	printf("%d\n", $1);	exit(0); }
	;

E	:	E PLUS T 			{	$$ = $1 + $3;	}
	|	T 					{	$$ = $1;		}
	;

T 	:	T STAR F 			{	$$ = $1 * $3;	}
	|	F 					{	$$ = $1;		}
	;

F	:	LPAREN E RPAREN 	{	$$ = $2;		}
	|	NUM					{ 	$$ = $1;		}
	;

%%