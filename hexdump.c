#include "project.h"

void
hexdump (void *_d, int len)
{
  uint8_t *d = (uint8_t *) _d;
  int i, j, k;
  int e;

  if (!d || len < 0)
    return;

  e = len + 15;
  e &= ~15;

  for (i = 0; i < e; i += 16)
    {
      printf (" %05x:", i);
      for (j = 0; j < 16; ++j)
        {
          k = i + j;

          if (k < len)
            printf (" %02x", d[k]);
          else
            printf ("   ");

          if (j == 7)
            printf (" ");
        }

      printf (" ");
      for (j = 0; j < 16; ++j)
        {
          k = i + j;
          if (k < len)
            {
              uint8_t c = d[k];
              if (c < 33)
                c = '.';
              if (c > 126)
                c = '.';
              printf ("%c", c);
            }
          if (j == 7)
            printf (" ");
        }
      printf ("\n");
    }
}
