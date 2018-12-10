#include "project.h"
#include "dfu.h"

static void
usage (char *name)
{
  fprintf (stderr, "Usage: %s -b bdaddr -p pkg_file\n", name);
  exit (EXIT_FAILURE);
}











int
main (int argc, char *argv[])
{
  char *bdaddr = NULL;
  char *pkg_fn = NULL;
  int opt;
  struct zip *zip;
  char *m_str;
  struct manifest *m;

  uint8_t *bin;
  size_t bin_size;

  uint8_t *dat;
  size_t dat_size;



  while ((opt = getopt (argc, argv, "b:p:")) != -1)
    {
      switch (opt)
        {
        case 'b':
          bdaddr = optarg;
          break;
        case 'p':
          pkg_fn = optarg;
          break;
        default:               /* '?' */
          usage (argv[0]);
        }
    }

  if ((!bdaddr) || (!pkg_fn))
    usage (argv[0]);


  zip = open_zip (pkg_fn);

  read_file_from_zip (zip, "manifest.json", &m_str);

  m = parse_manifest (m_str);

  if (m->hasSDBootloader) {
    dat_size = read_file_from_zip (zip, m->sdBootloaderDatFileName, &dat);
    bin_size = read_file_from_zip (zip, m->sdBootloaderBinFileName, &bin);
    
    printf ("%u bytes init_data, %u bytes SD+bootloader\n\n", (unsigned) dat_size, (unsigned) bin_size);

    if (dfu(bdaddr, dat, dat_size, bin, bin_size) != BLE_DFU_RESP_VAL_SUCCESS){
      return EXIT_FAILURE;  
    }
    sleep(5);
  }


  if (m->hasApplication) {
    dat_size = read_file_from_zip (zip, m->applicationDatFileName, &dat);
    bin_size = read_file_from_zip (zip, m->applicationBinFileName, &bin);
    
    printf ("%u bytes init_data, %u bytes firmware\n\n", (unsigned) dat_size, (unsigned) bin_size);

    if (dfu(bdaddr, dat, dat_size, bin, bin_size) != BLE_DFU_RESP_VAL_SUCCESS){
      return EXIT_FAILURE;  
    }
  }


  return EXIT_SUCCESS;
}
