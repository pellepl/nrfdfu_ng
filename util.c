#include "project.h"


void *
xmalloc (size_t s)
{
  char *ret = malloc (s);
  if (!ret)
    abort ();
  return ret;
}

void *
xrealloc (void *p, size_t s)
{
  char *ret = realloc (p, s);
  if (!ret)
    abort ();
  return ret;
}
