#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int strnreplace(char *subject, const char *search, const char *replace, const size_t subject_size)
{
	int i, j;
	int lreplace, lsearch;
	char *source;
	int count;

	source = strdup(subject);
	if (source == NULL)
	{
		return 0;
	}

	lsearch = strlen(search);
	lreplace = strlen(replace);

	i = 0;
	j = 0;
	count = 0;
	while(source[i] != '\0' && j <= subject_size)
	{
		if (strncmp(source+i, search, lsearch) == 0)
		{
			strncpy(subject+j, replace, subject_size-j);
			count++;

			i += lsearch;
			j += lreplace;
		}
		else
		{
			subject[j] = source[i];
			i++;
			j++;
		}
	}

	free(source);

	if (j >= subject_size)
	{
		subject[subject_size-1] = '\0';
	}

	return count;
}
