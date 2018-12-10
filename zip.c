#include "project.h"

void
fatal_zip (struct zip *zip)
{
  fprintf (stderr, "zip error: %s\n", zip_strerror (zip));
  exit (EXIT_FAILURE);
}





struct zip *
open_zip (const char *fn)
{
  struct zip *ret;
  int fd;
  int error;


  fd = open (fn, O_RDONLY);
  if (fd < 0)
    {
      perror ("open");
      exit (EXIT_FAILURE);
    }

  ret = zip_fdopen (fd, 0, &error);

  if (!ret)
    {
      fprintf (stderr, "filed to open zip file %s: %d\n", fn, error);
      exit (EXIT_FAILURE);
    }

  return ret;
}


size_t
read_file_from_zip (struct zip * zip, const char *fn, void *_buf)
{
  struct zip_file *f;
  size_t size = 0, buf_sz = 1024;
  ssize_t red;
  uint8_t *buf;

  f = zip_fopen (zip, fn, 0);

  if (!f)
    fatal_zip (zip);

  buf = xmalloc (buf_sz + 1);

  for (;;)
    {

      red = zip_fread (f, buf + size, buf_sz - size);

      if (red < 0)
        fatal_zip (zip);

      if (!red)
        break;

      size += red;
      if (size == buf_sz)
        {
          buf_sz <<= 1;
          buf = xrealloc (buf, buf_sz);
        }

    }

  buf[size] = 0;

  zip_fclose (f);

  *(void **) _buf = (void *) buf;

  return size;
}
