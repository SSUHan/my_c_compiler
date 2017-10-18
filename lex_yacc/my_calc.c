#include <stdio.h>
#include "y.tab.h"

int yywrap(){
  return(1);
}

void yyerror(char * error_str){
	fprintf(stderr, "%s\n", error_str);
}

int main(){
	yyparse();
	return 0;
}