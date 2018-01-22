/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2018 Baldur Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include "3rdparty/plthook/plthook.h"
#include "common/common.h"
#include "os/posix/posix_hook.h"

#include <android/dlext.h>
#include <dlfcn.h>
#include <jni.h>
#include <link.h>
#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>
#include <map>
#include <set>

void PosixHookInit()
{
}

bool PosixHookDetect(const char *identifier)
{
  return dlsym(RTLD_DEFAULT, identifier) != NULL;
}

std::map<std::string, void *> funchooks;
std::set<std::string> libhooks;

void PosixHookFunction(const char *name, void *hook)
{
  funchooks[name] = hook;
}

void PosixHookLibrary(const char *name, dlopenCallback cb)
{
  libhooks.insert(name);
  cb(dlopen(name, RTLD_NOW));
}

#if defined __x86_64__ || defined __x86_64
#define R_JUMP_SLOT R_X86_64_JUMP_SLOT
#define Elf_Rel ElfW(Rela)
#define ELF_R_TYPE ELF64_R_TYPE
#define ELF_R_SYM ELF64_R_SYM
#elif defined __i386__ || defined __i386
#define R_JUMP_SLOT R_386_JMP_SLOT
#define Elf_Rel ElfW(Rel)
#define ELF_R_TYPE ELF32_R_TYPE
#define ELF_R_SYM ELF32_R_SYM
#elif defined __arm__ || defined __arm
#define R_JUMP_SLOT R_ARM_JUMP_SLOT
#define Elf_Rel ElfW(Rel)
#define ELF_R_TYPE ELF32_R_TYPE
#define ELF_R_SYM ELF32_R_SYM
#elif defined __aarch64__ || defined __aarch64 /* ARM64 */
#define R_JUMP_SLOT R_AARCH64_JUMP_SLOT
#define Elf_Rel ElfW(Rela)
#define ELF_R_TYPE ELF64_R_TYPE
#define ELF_R_SYM ELF64_R_SYM
#else
#error unsupported OS
#endif

// uncomment the following to print (very verbose) debugging prints for the android PLT hooking
//#define HOOK_DEBUG_PRINT(...) RDCLOG(__VA_ARGS__)

#if !defined(HOOK_DEBUG_PRINT)
#define HOOK_DEBUG_PRINT(...) \
  do                          \
  {                           \
  } while(0)
#endif

std::set<std::string> hooked_already;
std::set<void *> hooked_handle_already;

static int callback(struct dl_phdr_info *info, size_t size, void *data)
{
  if(info->dlpi_name == NULL)
  {
    HOOK_DEBUG_PRINT("Skipping NULL entry!");
    return 0;
  }
  std::string soname = info->dlpi_name;

  if(hooked_already.find(soname) != hooked_already.end())
    return 0;

  // above will be absolute path, allow substring matches
  for(const std::string &fn : hooked_already)
  {
    if(soname.find(fn) != std::string::npos)
    {
      HOOK_DEBUG_PRINT("Not (re-)hooking %s", soname.c_str());
      return 0;
    }
  }

  HOOK_DEBUG_PRINT("Hooking %s", soname.c_str());
  hooked_already.insert(soname);

  for(int j = 0; j < info->dlpi_phnum; j++)
  {
    if(info->dlpi_phdr[j].p_type != PT_DYNAMIC)
      continue;

    ElfW(Dyn) *dynamic = (ElfW(Dyn) *)(info->dlpi_addr + info->dlpi_phdr[j].p_vaddr);

    ElfW(Sym) *dynsym = NULL;
    const char *strtab = NULL;
    size_t strtabcount = 0;
    Elf_Rel *pltbase = NULL;
    ElfW(Sword) pltcount = 0;

    while(dynamic->d_tag != DT_NULL)
    {
      if(dynamic->d_tag == DT_SYMTAB)
        dynsym = (ElfW(Sym) *)(info->dlpi_addr + dynamic->d_un.d_ptr);
      else if(dynamic->d_tag == DT_STRTAB)
        strtab = (const char *)(info->dlpi_addr + dynamic->d_un.d_ptr);
      else if(dynamic->d_tag == DT_STRSZ)
        strtabcount = dynamic->d_un.d_val;
      else if(dynamic->d_tag == DT_JMPREL)
        pltbase = (Elf_Rel *)(info->dlpi_addr + dynamic->d_un.d_ptr);
      else if(dynamic->d_tag == DT_PLTRELSZ)
        pltcount = dynamic->d_un.d_val / sizeof(Elf_Rel);

      /*
      if(dynamic->d_tag == DT_NEEDED)
        HOOK_DEBUG_PRINT("NEEDED [%i, %s]", dynamic->d_un.d_val, strtab + dynamic->d_un.d_val);
        */

      dynamic++;
    }

    if(!dynsym || !strtab || !pltbase || pltcount == 0 || strtabcount == 0)
    {
      RDCWARN("Missing required section to hook %s", info->dlpi_name);
      continue;
    }

    void **relro_base = NULL;
    void **relro_end = NULL;

    FILE *f = FileIO::fopen(info->dlpi_name, "r");

    // read the file on disk to get the .relro section
    if(f)
    {
      ElfW(Ehdr) ehdr;
      size_t read = FileIO::fread(&ehdr, sizeof(ehdr), 1, f);

      if(read == 1 && ehdr.e_ident[0] == ELFMAG0 && ehdr.e_ident[1] == 'E' &&
         ehdr.e_ident[2] == 'L' && ehdr.e_ident[3] == 'F')
      {
        FileIO::fseek64(f, ehdr.e_phoff, SEEK_SET);
        for(ElfW(Half) idx = 0; idx < ehdr.e_phnum; idx++)
        {
          ElfW(Phdr) phdr;
          read = FileIO::fread(&phdr, sizeof(phdr), 1, f);
          if(read != 1)
          {
            RDCWARN("Failed reading section");
            break;
          }

          if(phdr.p_type == PT_GNU_RELRO)
          {
            relro_base = (void **)(info->dlpi_addr + phdr.p_vaddr);
            relro_end = (void **)(info->dlpi_addr + phdr.p_vaddr + phdr.p_memsz);
          }
        }
      }
      else
      {
        RDCWARN("Didn't get valid ELF header");
      }

      FileIO::fclose(f);
    }
    else
    {
      RDCWARN("Couldn't open file to look for relro!");
    }

    if(relro_base)
      HOOK_DEBUG_PRINT("Got relro %p -> %p", relro_base, relro_end);
    HOOK_DEBUG_PRINT("Got %i PLT entries", pltcount);

    int pagesize = sysconf(_SC_PAGE_SIZE);

    for(ElfW(Sword) i = 0; i < pltcount; i++)
    {
      Elf_Rel *plt = pltbase + i;
      if(ELF_R_TYPE(plt->r_info) != R_JUMP_SLOT)
      {
        HOOK_DEBUG_PRINT("[%i]: Mismatched type %i vs %i", i, ELF_R_TYPE(plt->r_info), R_JUMP_SLOT);
        continue;
      }

      size_t idx = ELF_R_SYM(plt->r_info);
      size_t name = dynsym[idx].st_name;
      if(name + 1 > strtabcount)
      {
        HOOK_DEBUG_PRINT("[%i] name out of boundstoo big section header string table index: %zu", i,
                         name);
        continue;
      }

      const char *importname = strtab + name;
      void **import = (void **)(info->dlpi_addr + plt->r_offset);

      HOOK_DEBUG_PRINT("[%i] %s at %p (ptr to %p)", i, importname, import, *import);

      void *repl = funchooks[importname];
      if(repl)
      {
        HOOK_DEBUG_PRINT("replacing %s!", importname);

        uintptr_t pagebase = 0;

        if(relro_base <= import && import <= relro_end)
        {
          HOOK_DEBUG_PRINT("In relro range - %p <= %p <= %p", relro_base, import, relro_end);
          pagebase = uintptr_t(import) & ~(pagesize - 1);

          int ret = mprotect((void *)pagebase, pagesize, PROT_READ | PROT_WRITE);
          if(ret != 0)
          {
            RDCERR("Couldn't read/write the page: %d %d", ret, errno);
            return 0;
          }

          HOOK_DEBUG_PRINT("Marked page read/write");
        }
        else
        {
          HOOK_DEBUG_PRINT("Not in relro! - %p vs %p vs %p", relro_base, import, relro_end);
        }

        *import = repl;

        if(pagebase)
        {
          HOOK_DEBUG_PRINT("Moving back to read-only");
          mprotect((void *)pagebase, pagesize, PROT_READ);
        }

        HOOK_DEBUG_PRINT("[%i*] %s at %p (ptr to %p)", i, importname, import, *import);
      }
    }
  }

  return 0;
}

extern "C" __attribute__((visibility("default"))) void *hooked_dlopen(const char *filename, int flag);
extern "C" __attribute__((visibility("default"))) void *hooked_dlsym(void *handle,
                                                                     const char *symbol);
extern "C" __attribute__((visibility("default"))) void *hooked_android_dlopen_ext(
    const char *__filename, int __flags, const android_dlextinfo *__info);

// android has a special dlopen that passes the caller address in.
typedef void *(*pfn__loader_dlopen)(const char *filename, int flags, const void *caller_addr);

pfn__loader_dlopen loader_dlopen = NULL;

uint64_t suppressTLS = 0;

void PosixHookApply()
{
  RDCLOG("Applying hooks");

  suppressTLS = Threading::AllocateTLSSlot();

  // blacklist hooking certain system libraries or ourselves
  hooked_already.insert("libVkLayer_GLES_RenderDoc.so");
  hooked_already.insert("libc.so");
  hooked_already.insert("libvndksupport.so");

  libhooks.insert("libVkLayer_GLES_RenderDoc.so");

  loader_dlopen = (pfn__loader_dlopen)dlsym(RTLD_NEXT, "__loader_dlopen");

  if(loader_dlopen)
  {
    PosixHookFunction("dlopen", (void *)&hooked_dlopen);
  }
  else
  {
    RDCWARN("Couldn't find __loader_dlopen, falling back to slow path for dlopen hooking");
    PosixHookFunction("dlsym", (void *)&hooked_dlsym);
  }

  PosixHookFunction("android_dlopen_ext", (void *)&hooked_android_dlopen_ext);

  dl_iterate_phdr(callback, NULL);
}

void *intercept_dlopen(const char *filename, int flag)
{
  std::string search = filename;
  for(const std::string &fn : libhooks)
  {
    if(search.find(fn) != std::string::npos)
    {
      HOOK_DEBUG_PRINT("Intercepting and returning ourselves for %s (matches %s)", filename,
                       fn.c_str());
      return dlopen("libVkLayer_GLES_RenderDoc.so", flag);
    }
  }

  return NULL;
}

void process_dlopen(const char *filename, int flag)
{
  if(hooked_already.find(filename) == hooked_already.end())
  {
    dl_iterate_phdr(callback, NULL);

    // insert here if it wasn't inserted in the callback
    hooked_already.insert(filename);
  }
  else
  {
    HOOK_DEBUG_PRINT("Ignoring");
  }
}

extern "C" __attribute__((visibility("default"))) void *hooked_dlopen(const char *filename, int flag)
{
  // get caller address immediately.
  const void *caller_addr = __builtin_return_address(0);

  HOOK_DEBUG_PRINT("hooked_dlopen for %s | %d", filename, flag);
  void *ret = intercept_dlopen(filename, flag);

  // if we intercepted, return immediately
  if(ret)
    return ret;

  ret = loader_dlopen(filename, flag, caller_addr);
  HOOK_DEBUG_PRINT("Got %p", ret);

  if(filename && ret)
    process_dlopen(filename, flag);

  return ret;
}

extern "C" __attribute__((visibility("default"))) void *hooked_android_dlopen_ext(
    const char *__filename, int __flags, const android_dlextinfo *__info)
{
  HOOK_DEBUG_PRINT("hooked_android_dlopen_ext for %s | %d", __filename, __flags);

  void *ret = intercept_dlopen(__filename, __flags);

  // if we intercepted, return immediately
  if(ret)
    return ret;

  ret = android_dlopen_ext(__filename, __flags, __info);
  HOOK_DEBUG_PRINT("Got %p", ret);

  if(__filename && ret)
    process_dlopen(__filename, __flags);

  return ret;
}

PosixScopedSuppressHooking::PosixScopedSuppressHooking()
{
  if(suppressTLS == 0)
    return;

  uintptr_t old = (uintptr_t)Threading::GetTLSValue(suppressTLS);
  Threading::SetTLSValue(suppressTLS, (void *)(old + 1));
}

PosixScopedSuppressHooking::~PosixScopedSuppressHooking()
{
  if(suppressTLS == 0)
    return;

  uintptr_t old = (uintptr_t)Threading::GetTLSValue(suppressTLS);
  Threading::SetTLSValue(suppressTLS, (void *)(old - 1));
}

bool hooks_suppressed()
{
  return (uintptr_t)Threading::GetTLSValue(suppressTLS) > 0;
}

extern "C" __attribute__((visibility("default"))) void *hooked_dlsym(void *handle, const char *symbol)
{
  if(handle == NULL || symbol == NULL || hooks_suppressed())
    return dlsym(handle, symbol);

  void *repl = funchooks[symbol];

  if(repl == NULL)
    return dlsym(handle, symbol);

  if(hooked_handle_already.find(handle) == hooked_handle_already.end())
  {
    hooked_handle_already.insert(handle);
    dl_iterate_phdr(callback, NULL);
  }

  HOOK_DEBUG_PRINT("Got dlsym for %s which we want in %p...", symbol, handle);

  for(const std::string &lib : libhooks)
  {
    void *libHandle = dlopen(lib.c_str(), RTLD_NOLOAD);
    HOOK_DEBUG_PRINT("%s is %p", lib.c_str(), libHandle);
    if(libHandle == handle)
    {
      HOOK_DEBUG_PRINT("identified dlsym(%s) we want to interpose! returning %p", symbol, repl);
      return repl;
    }
  }

  void *ret = dlsym(handle, symbol);
  Dl_info info = {};
  dladdr(ret, &info);
  HOOK_DEBUG_PRINT("real ret is %p in %s", ret, info.dli_fname);
  return ret;
}