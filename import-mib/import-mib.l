%{
#include <stdio.h>
#include <string.h>
#include "import-mib.tab.h"
%}

%%
--.*$				/* comment */
[\t\r\n ]+			/* spaces */
DEFINITIONS			{ yylval = TOKEN_DEFINITION; return TOKEN_DEFINITION; }
IMPORTS				{ yylval = TOKEN_IMPORTS; return TOKEN_IMPORTS; }
ENTERPRISE			{ yylval = TOKEN_ENTERPRISE; return TOKEN_ENTERPRISE; }
::=					{ yylval = OPERATOR_ASSIGN; return OPERATOR_ASSIGN; }
BEGIN				{ yylval = TOKEN_BEGIN; return TOKEN_BEGIN; }
END					{ yylval = TOKEN_END; return TOKEN_END; }
OBJECT				{ yylval = TOKEN_OBJECT; return TOKEN_OBJECT; }
OBJECT-TYPE			{ yylval = TOKEN_OBJECT_MACRO; return TOKEN_OBJECT_MACRO; }
TRAP-TYPE			{ yylval = TOKEN_TRAP_MACRO; return TOKEN_TRAP_MACRO; }
\"[^\"]+\"			{ yylval = TEXT; return TEXT; }
;					{ yylval = OPERATOR_DOTCOMMA; return OPERATOR_DOTCOMMA; }
\{					{ yylval = BRACKET_OPEN; return BRACKET_OPEN; }
\}					{ yylval = BRACKET_CLOSE; return BRACKET_CLOSE; }
\(|\)|,|\.\.		{ yylval = OPERATOR_MISC; return OPERATOR_MISC; }
[a-zA-Z][a-zA-Z0-9_-]*	{ yylval = (YYSTYPE)strdup((char *)yytext); return NAME; }
[0-9]+					{ sscanf(yytext, "%u", &yylval); return NUMBER; }
