// Guillaume Valadon <guillaume@valadon.net>
// C based binutils and custom helpers

#include <config.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bfd.h>
#include <dis-asm.h>

void buffer_to_rust(char *buffer);


// Silly macro that helps removing the unused warnings
#define UNUSED_VARIABLE(id) id=id


/*** Generic helpers ***/

#define BUFFER_SIZE 64
char buffer_asm[BUFFER_SIZE];
void copy_buffer(void* useless, const char* format, ...) {
    /* Construct the final opcode into buffer_asm */
    UNUSED_VARIABLE(useless);

    va_list ap;

    va_start(ap, format);
    vsnprintf(buffer_asm, BUFFER_SIZE, format, ap);
    va_end(ap);
    buffer_to_rust(buffer_asm);
}

void show_buffer(struct disassemble_info *info) {
    printf("len=%d - vma=%lu\n", info->buffer_length, info->buffer_vma);
    printf("%p\n", info->buffer);
    printf("%x\n", info->buffer[0]);
    printf("%x\n", info->buffer[1]);
    printf("%x\n", info->buffer[2]);
    printf("%x\n", info->buffer[3]);
}

// do nothing
void styled_printf(void* useless, const char* format, ...){}

/*** disassemble_info structure helpers ***/

disassemble_info* new_disassemble_info() {
    /* Return a new structure */
    struct disassemble_info *info = malloc (sizeof(struct disassemble_info));
    return info;
}

bfd_boolean configure_disassemble_info(struct disassemble_info *info, asection *section, bfd *bfdFile) {
    /* Construct and configure the disassembler_info class using stdout */

    init_disassemble_info (info, stdout, (fprintf_ftype) copy_buffer, (fprintf_styled_ftype) styled_printf);
    info->arch = bfd_get_arch (bfdFile);
    info->mach = bfd_get_mach (bfdFile);
    info->section = section;

    info->buffer_vma = section->vma;
    info->buffer_length = section->size;

    return bfd_malloc_and_get_section (bfdFile, section, &info->buffer);
}

void configure_disassemble_info_buffer(struct disassemble_info *info, enum bfd_architecture arch, unsigned long mach) {
    /* A variant of configure_disassemble_info() for buffers */

    init_disassemble_info (info, stdout, (fprintf_ftype) copy_buffer, (fprintf_styled_ftype) styled_printf);
    info->arch = arch;
    info->mach = mach;
    info->read_memory_func = buffer_read_memory;
}

typedef void (*print_address_func) (bfd_vma addr, struct disassemble_info *dinfo);
void set_print_address_func(struct disassemble_info *info, print_address_func print_function) {
    info->print_address_func = print_function;
}

asection* set_buffer(struct disassemble_info *info, bfd_byte* buffer, unsigned int length, bfd_vma vma) {
    /* Configure the buffet that will be disassembled */
    info->buffer = buffer;
    info->buffer_length = length;
    info->buffer_vma = vma;

    asection *section = (asection*) malloc(sizeof(asection));
    if (section) {
        info->section = section;
        info->section->vma = vma;
    }

    return (asection*) section;
}

asection* get_disassemble_info_section(struct disassemble_info *info) {
  return info->section;
}

unsigned long get_disassemble_info_section_vma(struct disassemble_info *info) {
  return info->section->vma;
}

void free_disassemble_info(struct disassemble_info *info, bool free_section) {
  /* Free the structure and allocated variable */
  if (info) {
    if (free_section && info->section)
      free(info->section);
    free(info);
  }
}


/*** bfd structure helpers ***/

unsigned long get_start_address(bfd *bfdFile) {
    return bfdFile->start_address;
}

unsigned int macro_bfd_big_endian(bfd *bfdFile) {
    return bfd_big_endian(bfdFile);
}


/*** bfd_arch_info structure helpers ***/

enum bfd_architecture get_arch(struct bfd_arch_info *arch_info) {
  return arch_info->arch;
}

unsigned long get_mach(struct bfd_arch_info *arch_info) {
  return arch_info->mach;
}


/*** section structure helpers ***/

unsigned long get_section_size(asection *section) {
    return section->size;
}

/*** symbol wrappers ***/
long bfd_get_symtab_upper_bound_wrapper(bfd *bfdFile) {
    return bfd_get_symtab_upper_bound(bfdFile);
}

long bfd_canonicalize_symtab_wrapper(bfd *bfdFile, asymbol **symbol_table) {
    return bfd_canonicalize_symtab(bfdFile, symbol_table);
}

const char *bfd_asymbol_name_wrapper(asymbol *asym) {
    return bfd_asymbol_name(asym);
}

int bfd_decode_symclass_wrapper(asymbol *asym) { // func
    return bfd_decode_symclass(asym);
}

int bfd_asymbol_value_wrapper(asymbol *asym) { //inline
    return bfd_asymbol_value(asym);
}

bool bfd_is_undefined_symclass_wrapper(int symclass) { //func
    return bfd_is_undefined_symclass(symclass);
}

long bfd_get_dynamic_symtab_upper_bound_wrapper(bfd *bfdFile) {
    return bfd_get_dynamic_symtab_upper_bound(bfdFile);
}

long bfd_canonicalize_dynamic_symtab_wrapper(bfd *bfdFile, asymbol **symbol_table) {
    return bfd_canonicalize_dynamic_symtab(bfdFile, symbol_table);
}

int bfd_section_vma_wrapper(asection *sect) {
    return bfd_section_vma(sect);
}
int bfd_section_size_wrapper(asection *sect) {
    return bfd_section_size(sect);
}

int bfd_section_flags_wrapper(asection *sect) {
    return bfd_section_flags(sect);
}

int bfd_get_section_alignment_wrapper(asection *sect) {
    return bfd_section_alignment(sect);
}

const char *bfd_section_name_wrapper(asection *sect) {
    return bfd_section_name(sect);
}

// FIXME func
bool bfd_get_section_contents_wrapper(bfd *bfdFile, asection *sect, void *loc, int offset, int count) {
    return bfd_get_section_contents(bfdFile, sect, loc, offset, count);
}
