
#include <elf.h>
#include <dlfcn.h>
#include <link.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <vector>
#include "ProcessRuntimeUtility.h"
#include "elf_util.h"
#include "Logger.h"

using namespace SandHook;

static void file_mmap(const char *file_path, uint8_t **data_ptr, size_t *data_size_ptr) {
    uint8_t *mmap_data = NULL;
    size_t file_size = 0;

    int fd = open(file_path, O_RDONLY, 0);
    if (fd < 0) {
        LOGE("%s open failed", file_path);
        goto finished;
    }

    {
        struct stat s;
        int rt = fstat(fd, &s);
        if (rt != 0) {
            LOGE("mmap failed");
            goto finished;
        }
        file_size = s.st_size;
    }

    // auto align
    mmap_data = (uint8_t *) mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_PRIVATE, fd,
                                 0);
    if (mmap_data == MAP_FAILED) {
        LOGE("mmap failed");
        goto finished;
    }

    finished:
    close(fd);

    if (data_size_ptr)
        *data_size_ptr = file_size;
    if (data_ptr)
        *data_ptr = mmap_data;
}

static void file_unmap(void *data, size_t data_size) {
    int ret = munmap(data, data_size);
    if (ret != 0) {
        LOGE("munmap failed");
        return;
    }
}

typedef struct elf_ctx {
    void *header;

    uintptr_t load_bias;

    ElfW(Shdr) *sym_sh_;
    ElfW(Shdr) *dynsym_sh_;

    const char *strtab_;
    ElfW(Sym) *symtab_;

    const char *dynstrtab_;
    ElfW(Sym) *dynsymtab_;

    size_t nbucket_;
    size_t nchain_;
    uint32_t *bucket_;
    uint32_t *chain_;

    size_t gnu_nbucket_;
    uint32_t *gnu_bucket_;
    uint32_t *gnu_chain_;
    uint32_t gnu_maskwords_;
    uint32_t gnu_shift2_;
    ElfW(Addr) *gnu_bloom_filter_;
} elf_ctx_t;

static void get_syms(ElfW(Ehdr) *header, ElfW(Sym) **symtab_ptr, char **strtab_ptr, int *count_ptr) {
    ElfW(Shdr) *section_header = NULL;
    section_header = (ElfW(Shdr) *) ((uintptr_t) header + header->e_shoff);
 
    ElfW(Shdr) *section_strtab_section_header = NULL;
    section_strtab_section_header = (ElfW(Shdr) *) ((uintptr_t) section_header +header->e_shstrndx * header->e_shentsize);
    char *section_strtab = NULL;
    section_strtab = (char *) ((uintptr_t) header + section_strtab_section_header->sh_offset);

    for (int i = 0; i < header->e_shnum; ++i) {
        const char *section_name = (const char *) (section_strtab + section_header->sh_name);
        if (section_header->sh_type == SHT_SYMTAB && strcmp(section_name, ".symtab") == 0) {
            *symtab_ptr = (ElfW(Sym) *) ((uintptr_t) header + section_header->sh_offset);
            *count_ptr = section_header->sh_size / sizeof(ElfW(Sym));
        }

        if (section_header->sh_type == SHT_STRTAB && strcmp(section_name, ".strtab") == 0) {
            *strtab_ptr = (char *) ((uintptr_t) header + section_header->sh_offset);
        }
        section_header = (ElfW(Shdr) *) ((uintptr_t) section_header + header->e_shentsize);
    }
}

int elf_ctx_init(elf_ctx_t *ctx, void *header_) {
    ElfW(Ehdr) *ehdr = (ElfW(Ehdr) *) header_;
    ctx->header = ehdr;

    ElfW(Addr) ehdr_addr = (ElfW(Addr)) ehdr;

    // Handle dynamic segment
    {
        ElfW(Addr) addr = 0;
        ElfW(Dyn) *dyn = NULL;
        ElfW(Phdr) *phdr = reinterpret_cast<ElfW(Phdr) *>(ehdr_addr + ehdr->e_phoff);
        for (size_t i = 0; i < ehdr->e_phnum; i++) {
            if (phdr[i].p_type == PT_DYNAMIC) {
                dyn = reinterpret_cast<ElfW(Dyn) *>(ehdr_addr + phdr[i].p_offset);
            } else if (phdr[i].p_type == PT_LOAD) {
                addr = ehdr_addr + phdr[i].p_offset - phdr[i].p_vaddr;
                if (ctx->load_bias == 0)
                    ctx->load_bias = ehdr_addr - (phdr[i].p_vaddr - phdr[i].p_offset);
            } else if (phdr[i].p_type == PT_PHDR) {
                ctx->load_bias = (ElfW(Addr)) phdr - phdr[i].p_vaddr;
            }
        }
    }

    // Handle section
    {
        ElfW(Shdr) *dynsym_sh, *dynstr_sh;
        ElfW(Shdr) *sym_sh, *str_sh;

        ElfW(Shdr) *shdr = reinterpret_cast<ElfW(Shdr) *>(ehdr_addr + ehdr->e_shoff);

        ElfW(Shdr) *shstr_sh = NULL;
        shstr_sh = &shdr[ehdr->e_shstrndx];
        char *shstrtab = NULL;
        shstrtab = (char *) ((uintptr_t) ehdr_addr + shstr_sh->sh_offset);

        for (size_t i = 0; i < ehdr->e_shnum; i++) {
            if (shdr[i].sh_type == SHT_SYMTAB) {
                sym_sh = &shdr[i];
                ctx->sym_sh_ = sym_sh;
                ctx->symtab_ = (ElfW(Sym) *) (ehdr_addr + shdr[i].sh_offset);
            } else if (shdr[i].sh_type == SHT_STRTAB &&
                       strcmp(shstrtab + shdr[i].sh_name, ".strtab") == 0) {
                str_sh = &shdr[i];
                ctx->strtab_ = (const char *) (ehdr_addr + shdr[i].sh_offset);
            } else if (shdr[i].sh_type == SHT_DYNSYM) {
                dynsym_sh = &shdr[i];
                ctx->dynsym_sh_ = dynsym_sh;
                ctx->dynsymtab_ = (ElfW(Sym) *) (ehdr_addr + shdr[i].sh_offset);
            } else if (shdr[i].sh_type == SHT_STRTAB &&
                       strcmp(shstrtab + shdr[i].sh_name, ".dynstr") == 0) {
                dynstr_sh = &shdr[i];
                ctx->dynstrtab_ = (const char *) (ehdr_addr + shdr[i].sh_offset);
            }
        }
    }

    return 0;
}

static void *
iterate_symbol_table_impl(const char *symbol_name, ElfW(Sym) *symtab, const char *strtab,
                          int count) {
    for (int i = 0; i < count; ++i) {
        ElfW(Sym) *sym = symtab + i;
        const char *symbol_name_ = strtab + sym->st_name;
        if (strcmp(symbol_name_, symbol_name) == 0) {
            return (void *) sym->st_value;
        }
    }
    return NULL;
}

void *elf_ctx_iterate_symbol_table(elf_ctx_t *ctx, const char *symbol_name) {
    void *result = NULL;
    if (ctx->symtab_ && ctx->strtab_) {
        size_t count = ctx->sym_sh_->sh_size / sizeof(ElfW(Sym));
        result = iterate_symbol_table_impl(symbol_name, ctx->symtab_, ctx->strtab_, count);
        if (result)
            return result;
    }

    if (ctx->dynsymtab_ && ctx->dynstrtab_) {
        size_t count = ctx->dynsym_sh_->sh_size / sizeof(ElfW(Sym));
        result = iterate_symbol_table_impl(symbol_name, ctx->dynsymtab_, ctx->dynstrtab_, count);
        if (result)
            return result;
    }
    return NULL;
}

void *resolve_elf_internal_symbol(const char *library_name, const char *symbol_name) {
    void *result = NULL;

    if (library_name) {
        RuntimeModule module = ProcessRuntimeUtility::GetProcessModule(library_name);
        uint8_t *file_mem = NULL;
        size_t file_mem_size = 0;
        if (module.load_address)
            file_mmap(module.path, &file_mem, &file_mem_size);
        elf_ctx_t ctx;
        memset(&ctx, 0, sizeof(elf_ctx_t));
        if (file_mem) {
            elf_ctx_init(&ctx, file_mem);
            result = elf_ctx_iterate_symbol_table(&ctx, symbol_name);
        }
        if (result)
            result = (void *) ((uintptr_t) result + (uintptr_t) module.load_address - ((uintptr_t) file_mem - (uintptr_t) ctx.load_bias));

        if (file_mem)
            file_unmap(file_mem, file_mem_size);
    }

    if (!result) {
        std::vector<RuntimeModule> ProcessModuleMap = ProcessRuntimeUtility::GetProcessModuleMap();
        for (auto module: ProcessModuleMap) {
            uint8_t *file_mem = NULL;
            size_t file_mem_size = 0;
            if (module.load_address)
                file_mmap(module.path, &file_mem, &file_mem_size);
            elf_ctx_t ctx;
            memset(&ctx, 0, sizeof(elf_ctx_t));
            if (file_mem) {
                elf_ctx_init(&ctx, file_mem);
                result = elf_ctx_iterate_symbol_table(&ctx, symbol_name);
            }
            if (result)
                result = (void *) ((uintptr_t) result + (uintptr_t) module.load_address - ((uintptr_t) file_mem - (uintptr_t) ctx.load_bias));

            if (file_mem)
                file_unmap(file_mem, file_mem_size);

            if (result)
                break;
        }
    }
    return result;
}


void * DobbySymbolResolver(const char *image_name, const char *symbol_name_pattern) {
    void *result = nullptr;
    result = dlsym(RTLD_DEFAULT, symbol_name_pattern);
    if (result)
        return result;
    result = resolve_elf_internal_symbol(image_name, symbol_name_pattern);
    return result;
}