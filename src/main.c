#include <assert.h>
#include <fcntl.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

/* parse object file */
void *parse_file(const char *filename, off_t *size);

/* find function in object file */
void *find_function(void *obj, const char *function);

/* find section number */
uint8_t find_function_section_number(char **currentOffset, void *obj,
                                     const char *function);

/* get sections */
struct section_64 *get_sections(char **currentOffset);

int main(void) {
  /* load object file into memory */
  off_t fsize;
  void *obj = parse_file("lib.o", &fsize);

  /* find function */
  int (*add)(int, int) = (int (*)(int, int))find_function(obj, "_add");

  /* the important calculation */
  printf("%d\n", add(2, 2));

  munmap(obj, fsize);

  return 0;
}

void *parse_file(const char *filename, off_t *fsize) {
  int fd = open(filename, O_RDONLY);
  struct stat st;

  int success = fstat(fd, &st);
  assert(success == 0);

  *fsize = st.st_size;

  /* copy the file into memory */
  void *obj = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

  assert(obj != MAP_FAILED);

  close(fd);

  /* mark the memory readable and executable but no writable */
  success = mprotect(obj, st.st_size, PROT_READ | PROT_EXEC);

  assert(success == 0);

  return obj;
}

void *find_function(void *obj, const char *function) {
  char *currentOffset = obj;

  /* check mach-o header */
  struct mach_header_64 *header = (struct mach_header_64 *)currentOffset;
  currentOffset += sizeof *header;

  assert(header->magic == MH_MAGIC_64);

  struct section_64 *sections = NULL;
  uint8_t sectionNumber = 0;

  for (int i = 0; i < header->ncmds; ++i) {
    /* check load command */
    struct load_command *load = (struct load_command *)currentOffset;

    if (load->cmd == LC_SYMTAB) {
      sectionNumber =
          find_function_section_number(&currentOffset, obj, function);
    } else if (load->cmd == LC_SEGMENT_64) {
      sections = get_sections(&currentOffset);
    } else {
      currentOffset += load->cmdsize;
    }
  }

  assert(sectionNumber != 0);
  assert(sections != NULL);

  struct section_64 *sect =
      sections + sectionNumber - 1; /* sectionNumber are 1-based */

  char *section = obj + sect->offset;

  return section;
}

uint8_t find_function_section_number(char **currentOffset, void *obj,
                                     const char *function) {
  /* check symtab */
  struct symtab_command *symtab = (struct symtab_command *)*currentOffset;

  *currentOffset += sizeof *symtab;

  /* get nlist */
  struct nlist_64 *ns = (struct nlist_64 *)(obj + symtab->symoff);
  char *strtable = obj + symtab->stroff;

  uint8_t sectionNumber = 0;

  for (int j = 0; j < symtab->nsyms; ++j) {
    if (strcmp(function, strtable + ns[j].n_un.n_strx) == 0) {
      /* assure that type is section */
      assert((ns[j].n_type & N_TYPE) == N_SECT);

      sectionNumber = ns[j].n_sect;
      break;
    }
  }

  return sectionNumber;
}

struct section_64 *get_sections(char **currentOffset) {
  /* check segment */
  struct segment_command_64 *segment =
      (struct segment_command_64 *)*currentOffset;

  *currentOffset += sizeof *segment;

  uint8_t numberOfSections = segment->nsects;
  struct section_64 *sections = (struct section_64 *)*currentOffset;

  *currentOffset += numberOfSections * sizeof *sections;

  return sections;
}
