#include <windows.h>

bool ns_oid_concat(char *destvar, char *sourcevar, unsigned int node);
void add_trap_registry(const char *name, const char *enterprise, unsigned int trapCode);

/* that stuff should be included in header file import-mib.tab.h ... */
void yyerror(char *s);
int yylex();
extern FILE * yyin;
