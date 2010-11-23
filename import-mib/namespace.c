#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_NAMESPACE_ENTRY		4096

typedef struct
{
	char *name;
	char *value;
} entry;

static entry entry_list[MAX_NAMESPACE_ENTRY];
static unsigned int entry_count = 0;

void ns_add(const char *name, const char *value)
{
	if (entry_count >= MAX_NAMESPACE_ENTRY)
	{
		return;
	}

	entry_list[entry_count].name = strdup(name);
	entry_list[entry_count].value = strdup(value);
	entry_count++;
}

const char *ns_find(const char *name)
{
	unsigned int i = 0;

	while(i < entry_count)
	{
		if (strcmp(name, entry_list[i].name) == 0)
		{
			return entry_list[i].value;
		}
		i++;
	}

	return NULL;
}

void ns_free()
{
	while(entry_count > 0)
	{
		entry_count--;

		if (entry_list[entry_count].name != NULL)
		{
			free(entry_list[entry_count].name);
		}

		if (entry_list[entry_count].value != NULL)
		{
			free(entry_list[entry_count].value);
		}
	}
}
