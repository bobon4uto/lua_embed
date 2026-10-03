#define METAM_IMPLEMENTATION
#include "metam.h"
int main (int argc, char** argv) {
  if (argc < 4) {
    printf("usage: stage0 input1.h input2.h -n module_name -o output.c\n");
    return 0;
  }

  DA_(char_ptr) sources = {0};
  char_ptr output = "a.out";
  StringView module_name = sv_from_str("module");

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == 'o') {
        if (i+1 < argc) {
          output = argv[++i];
        }
      } else if (argv[i][1] == 'n') {
        if (i+1 < argc) {
          module_name = sv_from_str(argv[++i]);
        }
      }
    } else {
      da_push(&sources, argv[i]);
    }
  }
  LOG("COMPILE");

  StringBuilder compiled = metam_full_sweep(sources, module_name);

  //printf("[\n%.*s\n]", SB_FMT(compiled) );
  sb_write_to_file(compiled, output);

  sb_free(&compiled);

  return 0;
}
