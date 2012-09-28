#include <stdio.h>
#include <windows.h>
#include "..\core\pluginEngine.h"
#include "..\core\snmptraptools_config.h"

#ifdef DEBUG

#define DEBUG_PLUGIN_TEST_TRAP_HANDLE				1
#define DEBUG_PLUGIN_TEST_PLUGIN_DEFAULT_VALUE		2
#define DEBUG_PLUGIN_TEST_PLUGIN_CONFIG_EDITOR		4

#define DEBUG_PLUGIN_TEST_READ_CONFIGURATION		8
#define DEBUG_PLUGIN_TEST_WRITE_CONFIGURATION		16
#define DEBUG_PLUGIN_TEST_LOAD_CONFIGURATION		32


bool debug_plugin_confirm(const char *brief)
{
	char q;

	fflush(stdin);
	printf("%s [y/N]?", brief);
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
	char buffer[MAX_PLUGIN_CONFIG_LEN];
	uint32_t buffer_size;
	void *shared_data;

	unsigned int tests;

	debug_plugin_confirm_test("Get and save plugin default settings", &tests, DEBUG_PLUGIN_TEST_PLUGIN_DEFAULT_VALUE);

	debug_plugin_confirm_test("Test plugin read configuration", &tests, DEBUG_PLUGIN_TEST_READ_CONFIGURATION);
	debug_plugin_confirm_test("Test plugin configuration editor", &tests, DEBUG_PLUGIN_TEST_PLUGIN_CONFIG_EDITOR);
	debug_plugin_confirm_test("Test plugin write configuration", &tests, DEBUG_PLUGIN_TEST_WRITE_CONFIGURATION);
	debug_plugin_confirm_test("Test plugin load configuration", &tests, DEBUG_PLUGIN_TEST_LOAD_CONFIGURATION);

	debug_plugin_confirm_test("Test plugin trap handling", &tests, DEBUG_PLUGIN_TEST_TRAP_HANDLE);

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
				if (tests & DEBUG_PLUGIN_TEST_PLUGIN_DEFAULT_VALUE)
				{
					printf("\tT: get plugin default value: ");
					// ask for default value
					shared_data = p[i].EditConfig(NULL, &buffer_size);  // default value
					if (shared_data != NULL)
					{
						memcpy(buffer, shared_data, buffer_size);
						printf("%d bytes read\n", buffer_size);
					}
					else
					{
						printf("failed!\n");
					}

					printf("\tT: write plugin configuration");
					if (plugin_set_configuration(buffer, buffer_size, p[i].GetName()))		// set_configuration test
					{
						printf("%d bytes written\n", buffer_size);
					}
					else
					{
						printf("failed!\n");
					}
				}
				if (tests & DEBUG_PLUGIN_TEST_READ_CONFIGURATION)
				{
					printf("\tT: read plugin configuration: ");
					buffer_size = sizeof(buffer);
					if (plugin_get_configuration(buffer, &buffer_size, p[i].GetName()))
					{
						printf("%d bytes read\n", buffer_size);
					}
					else
					{
						printf("failed!\n");
					}
				}
				printf("buffer_size=%d\n", buffer_size);
				if (tests & DEBUG_PLUGIN_TEST_PLUGIN_CONFIG_EDITOR)
				{
					printf("\tT: run plugin configuration editor: ");
					shared_data = p[i].EditConfig(buffer, &buffer_size);  // edit config
					if (shared_data != NULL)
					{
						memcpy(buffer, shared_data, buffer_size);
						printf("%d bytes read\n", buffer_size);
					}
					else
					{
						printf("no changes.\n");
					}
				}
				printf("buffer_size=%d\n", buffer_size);
				if (tests & DEBUG_PLUGIN_TEST_WRITE_CONFIGURATION)
				{
					printf("\tT: write plugin configuration: ");
					if (plugin_set_configuration(buffer, buffer_size, p[i].GetName()))		// set_configuration test
					{
						printf("%d bytes written\n", buffer_size);
					}
					else
					{
						printf("failed!\n");
					}
				}
				printf("buffer_size=%d\n", buffer_size);
				if (tests & DEBUG_PLUGIN_TEST_LOAD_CONFIGURATION)
				{
					printf("\tT: uploading configuration to plugin\n");
					p[i].LoadConfig(buffer, buffer_size);
				}
				if (tests & DEBUG_PLUGIN_TEST_TRAP_HANDLE)
				{
					printf("\tT: Send trap to plugin\n");
					plugin_emit_sample(&p[i]);
				}
			}
			else
			{
				printf("unable to load plugin!\n");
			}
		}
		else
		{
			printf("\tSkipped.\n");
		}
		i++;
	}

    return 0;
}
#endif // DEBUG
