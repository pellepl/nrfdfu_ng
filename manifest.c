#include "project.h"
#include <string.h>

json_object *
_json_object_object_get (json_object * obj, const char *name)
{
  json_object *sub;
  return json_object_object_get_ex (obj, name, &sub) ? sub : NULL;
}


static void
dump_manifest (struct manifest *m)
{
  printf ("Manifest:\n");
  if (m->hasSDBootloader){
    printf("  == SD+Bootloader ==\n");
    printf("  dat_file : %s\n", m->sdBootloaderDatFileName);
    printf("  bin_file : %s\n", m->sdBootloaderBinFileName);
    printf("\n");
  }
  if (m->hasApplication){
    printf("  == Application ==\n");
    printf("  dat_file : %s\n", m->applicationDatFileName);
    printf("  bin_file : %s\n", m->applicationBinFileName);
    printf("\n");
  }
}


const char *DFU_VERSION="0";
struct manifest *
parse_manifest (const char *str)
{
  json_object *json, *manifest;
  enum json_tokener_error jerr = json_tokener_success;
  struct manifest *m;


  m = xmalloc (sizeof (*m)); //notice: memory leak
  memset (m, 0, sizeof (*m));

  json = json_tokener_parse_verbose (str, &jerr);

  manifest = _json_object_object_get (json, "manifest");


  
  json_object_object_foreach (manifest, key, val)
  {
    if (strcmp(key,"application")==0){
      m->hasApplication = 1;
      m->applicationBinFileName = json_object_get_string (_json_object_object_get (val, "bin_file"));
      m->applicationDatFileName = json_object_get_string (_json_object_object_get (val, "dat_file"));
    }
    else if (strcmp(key,"softdevice_bootloader")==0){
      m->hasSDBootloader = 1;
      m->sdBootloaderBinFileName = json_object_get_string (_json_object_object_get (val, "bin_file"));
      m->sdBootloaderDatFileName = json_object_get_string (_json_object_object_get (val, "dat_file"));
    }
    else {
      fprintf (stderr, "Unhandled manifest content '");
      fprintf (stderr, key);
      fprintf (stderr, "' this tool needs to be updated to support this\n");
      exit (EXIT_FAILURE);
    }
  }


  dump_manifest (m);


  if ((m->hasApplication  && ((!m->applicationDatFileName)  || (!m->applicationBinFileName))) ||
      (m->hasSDBootloader && ((!m->sdBootloaderDatFileName) || (!m->sdBootloaderBinFileName)))) {
    fprintf (stderr, "Failed to process manifest\n");
    exit (EXIT_FAILURE);
  }

  return m;
}
