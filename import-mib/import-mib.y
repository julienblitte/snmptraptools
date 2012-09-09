%{
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

#include "..\core\snmptraptools_config.h"
#include "..\core\trapSnmp.h"
#include "namespace.h"
#include "registry.h"
#include "import-mib.h"

HKEY registry;

%}

%token TOKEN_DEFINITION TOKEN_IMPORTS TOKEN_BEGIN TOKEN_END TOKEN_OBJECT TOKEN_OBJECT_MACRO TOKEN_TRAP_MACRO TOKEN_ENTERPRISE
%token OPERATOR_ASSIGN OPERATOR_DOTCOMMA OPERATOR_MISC
%token TEXT NAME NUMBER BRACKET_OPEN BRACKET_CLOSE

%start MIB
%%

MIB:
	MIB_HEADER MIB_CONTENT MIB_FOOTER
	;

MIB_HEADER:
	NAME TOKEN_DEFINITION OPERATOR_ASSIGN TOKEN_BEGIN
;

MIB_FOOTER:
	TOKEN_END
;

MIB_CONTENT:
	/* empty */
	|MIB_CONTENT MIB_ENTRY
;

MIB_ENTRY:
	MIB_IMPORT
	| MIB_OID_MACRO
	| MIB_OID
	| MIB_TRAP_MACRO
	| MIB_TYPE
	| error
;

MIB_IMPORT:
	TOKEN_IMPORTS MIB_IMPORT_CONTENT OPERATOR_DOTCOMMA
;

MIB_IMPORT_CONTENT:
	/* empty */
	|MIB_IMPORT_CONTENT MIB_IMPORT_ENTRY
;

MIB_IMPORT_ENTRY:
	NAME
	|NUMBER
	|TEXT
	|BRACKET_OPEN
	|BRACKET_CLOSE
	|OPERATOR_MISC
	|TOKEN_OBJECT_MACRO
	|TOKEN_TRAP_MACRO
;

MIB_OID_MACRO:
	NAME TOKEN_OBJECT_MACRO MIB_ITEM_CONTENT OPERATOR_ASSIGN BRACKET_OPEN NAME NUMBER BRACKET_CLOSE  { ns_oid_concat((char *)$1, (char *)$6, $7); }
;

MIB_OID:
	NAME TOKEN_OBJECT MIB_ITEM_CONTENT OPERATOR_ASSIGN BRACKET_OPEN NAME NUMBER BRACKET_CLOSE  { ns_oid_concat((char *)$1, (char *)$6, $7); }
;

MIB_TRAP_MACRO:
	NAME TOKEN_TRAP_MACRO TOKEN_ENTERPRISE NAME MIB_ITEM_CONTENT OPERATOR_ASSIGN NUMBER { add_trap_registry((char *)$1, (char *)$4, $7); }
;

MIB_TYPE:
	NAME OPERATOR_ASSIGN NAME BRACKET_OPEN MIB_ITEM_CONTENT BRACKET_CLOSE
;

MIB_ITEM_CONTENT:
	/* empty */
	|MIB_ITEM_CONTENT MIB_ITEM_ENTRY
;

MIB_ITEM_ENTRY:
	NAME
	|NUMBER
	|TEXT
	|BRACKET_OPEN
	|BRACKET_CLOSE
	|OPERATOR_MISC
;

%%

void yyerror(char *s)
{
	fprintf(stderr, "%s\n", s);
}

bool ns_oid_concat(char *destvar, char *sourcevar, unsigned int node)
{
	static char buffer[MAX_OID_LEN];
	const char *sourcevalue;

	sourcevalue = ns_find(sourcevar);
	if (sourcevalue == NULL)
	{
		return false;
	}

	snprintf(buffer, sizeof(buffer), "%s%c%u", sourcevalue, OID_SEPARATOR, node);
	ns_add(destvar, buffer);

	return true;
}

void add_trap_registry(const char *name, const char *enterprise, unsigned int trapCode)
{
    const char *oid;
    static char description[MAX_DESCRIPTION_LEN];

    oid = ns_find(enterprise);
    if (oid == NULL)
    {
        if (snmpoid_valid(enterprise) == TRUE)
        {
            oid = enterprise;
        }
        else
        {
            fprintf(stderr, "Symbol '%s' in unknown!\n", enterprise);
            return;
        }
    }

    snprintf(description, sizeof(description), "%s ${*}", name);

    if (registryAddOid(registry, (LPCTSTR)oid, (DWORD)trapCode, (LPCTSTR)description) != TRUE)
    {
        fprintf(stderr, "Registry access error!\n");
    }
    else
    {
        printf("added: %s (%u, %u)\n", oid, SPECIFIC_TYPE_GENERIC, trapCode);
    }
}


int main(int argc, char *argv[])
{
	unsigned int i;

	if (argc == 1)
	{
		fprintf(stderr, "Usage: %s <file1.mib> [file2.mib ...]\n", argv[0]);
		return EXIT_FAILURE;
	}
	else
	{
		if (MessageBox(NULL, "You are going to import MIB data to snmptraptools.\n\nPlease, verify that Configure tool is not running.\n", "Snmptraptools mib import", MB_ICONWARNING|MB_OKCANCEL) != IDOK)
		{
			fprintf(stderr, "Aborted by user.\n");
			return EXIT_SUCCESS;
		}
	}

    registry = registryOpen();
    if (registry == NULL)
    {
        fprintf(stderr, "Registry error!\n");
        return EXIT_FAILURE;
    }

	// add common defined values
	ns_add("iso", "1");
	ns_add("org", "1.3");
	ns_add("dod", "1.3.6");
	ns_add("internet", "1.3.6.1");

	ns_add("directory", "1.3.6.1.1");

	ns_add("mgmt", "1.3.6.1.2");
	ns_add("mib-2", "1.3.6.1.2.1");

	ns_add("experimental", "1.3.6.1.3");

	ns_add("private", "1.3.6.1.4");
	ns_add("enterprises", "1.3.6.1.4.1");

	for(i=1; i < argc; i++)
	{
		yyin = fopen(argv[i], "r");
		if (yyin != NULL)
		{
			yyparse();
			fclose(yyin);
		}
		else
		{
			fprintf(stderr, "Unable to open file '%s'!\n", argv[i]);
		}
	}

	registryClose(registry);

	MessageBox(NULL, "Import complete.\n\nYou need to restart service to load new configuration.\n", "Operation complete", MB_ICONINFORMATION|MB_OK);

    return EXIT_SUCCESS;
}
