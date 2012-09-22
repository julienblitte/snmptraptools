#include <stdio.h>
#include <windows.h>
#include "..\core\pluginEngine.h"
#include "..\core\snmptraptools_config.h"

#ifdef DEBUG

#define DEBUG_PLUGIN_TEST_TRAP	1
#define DEBUG_PLUGIN_TEST_PLUGIN_CONFIG_EDIT	2
#define DEBUG_PLUGIN_TEST_PLUGIN_CONFIG_SET		4
#define DEBUG_PLUGIN_TEST_PLUGIN_CONFIG_GET		8

bool debug_plugin_confirm(const char *brief)
{
	char q;

	fflush(stdin);
	printf("%s [Y/N]?", brief);
	scanf("%c", &q);
	fflush(stdin);

	return (q == 'y' || q == 'Y');
}

void debug_plugin_confirm_test(const char *brief, unsigned int *test_set, const unsigned int test)
{
	if (debug_plugin_confirm(brief))
	{
		*test_set |= test;
	}
	else
	{
		*test_set &= ~test;
	}
}

int main()
{
	char **list = plugin_find();
	unsigned int i;
	plugin_set p[MAX_PLUGINS];
	char buffer[256];
	uint32_t buffer_size;

	unsigned int tests;

	debug_plugin_confirm_test("Test trap handling", &tests, DEBUG_PLUGIN_TEST_TRAP);
	debug_plugin_confirm_test("Test plugin configuration", &tests, DEBUG_PLUGIN_TEST_PLUGIN_CONFIG_EDIT);
	debug_plugin_confirm_test("Test plugin set routine", &tests, DEBUG_PLUGIN_TEST_PLUGIN_CONFIG_SET);
	debug_plugin_confirm_test("Test plugin get routine", &tests, DEBUG_PLUGIN_TEST_PLUGIN_CONFIG_GET);

	i=0;
	while (list[i] && i < MAX_PLUGINS)
	{
		printf("\nFound \"%s\", ", list[i]);
		if (debug_plugin_confirm("proceed"))
		{
			if (plugin_load(list[i], &p[i]))
			{
				printf("\tplugin \"%s\" loaded at @%p\n", p[i].GetName(), p[i].dll);
				printf("\tunique identifier is %08x\n", p[i].GetUID());

				if (tests & DEBUG_PLUGIN_TEST_TRAP)
				{
					printf("\tT: emit trap to plugin\n");
					plugin_emit_sample(&p[i]);
				}
				if (tests & DEBUG_PLUGIN_TEST_PLUGIN_CONFIG_EDIT)
				{
					printf("\tT: run plugin configuration editor\n");
					p[i].EditConfig(NULL, 0);  // EditConfig test
				}
				if (tests & DEBUG_PLUGIN_TEST_PLUGIN_CONFIG_GET)
				{
					printf("\tT: read plugin configuration: ");
					buffer_size = sizeof(buffer);
					plugin_get_configuration(buffer, &buffer_size, p[i].GetName()); 	// get_configuration test
					buffer[buffer_size]='\0';
					printf("%d bytes read\n", buffer_size);
				}
				if (tests & DEBUG_PLUGIN_TEST_PLUGIN_CONFIG_SET)
				{
					printf("\tT: write plugin configuration: ");
					strcpy(buffer, p[i].GetName());
					buffer_size = strlen(buffer);
					if (plugin_set_configuration(buffer, buffer_size, p[i].GetName()))		// set_configuration test
					{
						printf("%d bytes written\n", buffer_size);
					}
					else
					{
						printf("failed!\n");
					}
				}
			}
			else
			{
				printf("unable to load plugin!\n");
			}
		}
		else
		{
			printf("skipped.\n");
		}
		i++;
	}

    return 0;
}
#endif // DEBUG
