#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "errlog.h"

#if defined(__GNUC__)

#include "execinfo.h"

void print_trace(FILE *out) 
{
  void *array[20];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace (array, 20);
  strings = backtrace_symbols (array, size);

  fprintf (out, "Last %zd stack frames.\n", size);

  for (i = 0; i < size; i++)
     fprintf (out, "%s\n", strings[i]);

  free (strings);
}

#else 

void print_trace(FILE *out)
{
    fprintf(out, "Cannot print stack trace.\n");
}

#endif
