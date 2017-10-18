%{
#include "y.tab.h"	
extern int yylval;
%}

digit [0-9]
letter [a-z]
delim [ \t]
ws [delim]+

%%
{ws}		{}
"+" 		{	return (PLUS);	}
"*"			{	return (STAR);	}
"\n"		{	return (NEW_LINE); 	}
"("			{	return (LPAREN);	}
")"			{	return (RPAREN);	}
{digit}+	{
				yylval = atoi(yytext);
				return (NUM);
			}
%%

