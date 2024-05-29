#include <string.h>
#include "Vlib.h"

char *strdup(string)
char *string;
{
     return(strcpy(Vmalloc(sizeof(char) * (strlen(string) + 1)),
		   string));
}
