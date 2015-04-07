/* Mini Calculator */
/* calc.y */

%{
#include "stdafx.h"
#include "AstHttp.h"


int yyerror(char *s);
int yylex(void);

%}

%union{
  int		int_val;
  char*		identifier_tok;
};

%start	input 

%token	<int_val>			DIGITS NEWLINE
%token	<identifier_tok>	IDENTIFIER METHODACTION DIGIT HEADERELEM URLPATH METHODVER ID2 URLPARAMS HOST EQUAL
							CONNECTION USERAGENT QUESTION AMPERSAND ANYTYPE ACCEPT 
%type	<int_val>			exp
%type	<identifier_tok>	delimiters

%type	<lpHttpdoc>	line 
%left	PLUS
%left	MULT

%%

input	: /* empty */
		| input line 
		;

line	: exp { cout << "Result: " << $1 << endl; }
		| line1 { printf("line1 seen\n"); }
		| host { printf("HOST seen\n"); }
		| connection { printf("CONNECTION seen\n"); }
		| accept
		| property_item_list { printf("property_item seen\n"); }
		| user_agent
		;

exp		: DIGITS	{ $$ = $1; }
		| exp PLUS exp	{ $$ = $1 + $3; }
		| exp MULT exp	{ $$ = $1 * $3; }
		;


line1	: METHODACTION		{ SetMethodAction($1, &g_Httpdoc); }
		| line1 URLPATH		{ SetUrl($2, &g_Httpdoc); }
		| line1 urlparams   { printf("urlparams whole seen\n"); }
		| line1 METHODVER   { SetMethodVersion($2, &g_Httpdoc); }
		;

urlparams : QUESTION								{ printf("urlparams 1 seen\n"); }
		  | urlparams IDENTIFIER EQUAL IDENTIFIER	{ printf("urlparams 2 seen\n"); }
		  | urlparams ID2 EQUAL ID2					{ printf("urlparams 3 seen\n"); }
		  | urlparams ID2 EQUAL DIGITS				{ printf("urlparams 4 seen\n"); }
		  | urlparams AMPERSAND						{ printf("urlparams 5 seen\n"); }
		  ;



host	: HOST
		| host ID2
		| host ":" ID2
		;

connection	: CONNECTION
			| connection ID2
			;

accept		: ACCEPT
			| accept ID2 
			| accept delimiters
			| accept DIGITS
			| accept ANYTYPE
			;

user_agent	: USERAGENT
			| user_agent ID2 
			| user_agent IDENTIFIER 
			| user_agent delimiters
			| user_agent DIGITS
			| user_agent ANYTYPE
			;

property_item_list : property_item
				   | property_item_list property_item
				   ;

property_item : ID2 ":" " " attriblist
			  ;

attriblist    : attriblist
			  | attriblist ANYTYPE
			  | attriblist ID2
			  | attriblist IDENTIFIER
			  | attriblist "/"
			  | attriblist "," 
			  | attriblist ":" 
			  | attriblist ";" 
			  | attriblist "." 
			  | attriblist MULT
			  | attriblist DIGITS
			  | attriblist PLUS
			  | attriblist "(" 
			  | attriblist ")" 
			  ;

delimiters    : ","
			  | ";"
			  | "/"
			  | PLUS
			  | "."
			  ;

%%



int yyerror(string s)
{
  extern int yylineno;	// defined and maintained in lex.c
  extern char *yytext;	// defined and maintained in lex.c
  
  cerr << "PARSER sERROR: " << s << " at symbol \"" << yytext;
  cerr << "\" on line " << yylineno << endl;
  //exit(1);
  return 1;
}

int yyerror(char *s)
{
  return yyerror(string(s));
}
