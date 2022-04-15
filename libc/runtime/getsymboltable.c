/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2020 Justine Alexandra Roberts Tunney                              │
│                                                                              │
│ Permission to use, copy, modify, and/or distribute this software for         │
│ any purpose with or without fee is hereby granted, provided that the         │
│ above copyright notice and this permission notice appear in all copies.      │
│                                                                              │
│ THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL                │
│ WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED                │
│ WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE             │
│ AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL         │
│ DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR        │
│ PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER               │
│ TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR             │
│ PERFORMANCE OF THIS SOFTWARE.                                                │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "libc/assert.h"
#include "libc/bits/bits.h"
#include "libc/bits/weaken.h"
#include "libc/calls/strace.internal.h"
#include "libc/macros.internal.h"
#include "libc/runtime/runtime.h"
#include "libc/runtime/symbols.internal.h"
#include "libc/str/str.h"
#include "libc/str/undeflate.h"
#include "libc/x/x.h"
#include "libc/zip.h"
#include "libc/zipos/zipos.internal.h"

/**
 * Looks for `.symtab` in zip central directory.
 */
noasan static ssize_t FindSymtabInZip(struct Zipos *zipos) {
  size_t i, n, c;
  c = GetZipCdirOffset(zipos->cdir);
  n = GetZipCdirRecords(zipos->cdir);
  for (i = 0; i < n; ++i, c += ZIP_CFILE_HDRSIZE(zipos->map + c)) {
    if (ZIP_CFILE_NAMESIZE(zipos->map + c) == 7 &&
        READ32LE(ZIP_CFILE_NAME(zipos->map + c + 0)) == READ32LE(".sym") &&
        READ16LE(ZIP_CFILE_NAME(zipos->map + c + 4)) == READ16LE("ta") &&
        *ZIP_CFILE_NAME(zipos->map + c + 6) == 'b') {
      return c;
    }
  }
  return -1;
}

/**
 * Reads symbol table from zip directory.
 * @note This code can't depend on dlmalloc()
 */
noasan static struct SymbolTable *GetSymbolTableFromZip(struct Zipos *zipos) {
  ssize_t cf, lf;
  size_t size, size2;
  struct DeflateState ds;
  struct SymbolTable *res = 0;
  if ((cf = FindSymtabInZip(zipos)) != -1) {
    lf = GetZipCfileOffset(zipos->map + cf);
    size = GetZipLfileUncompressedSize(zipos->map + lf);
    size2 = ROUNDUP(size, FRAMESIZE);
    if ((res = mapanon(size2))) {
      switch (ZIP_LFILE_COMPRESSIONMETHOD(zipos->map + lf)) {
        case kZipCompressionNone:
          memcpy(res, (void *)ZIP_LFILE_CONTENT(zipos->map + lf), size);
          break;
        case kZipCompressionDeflate:
          if (undeflate(res, size, (void *)ZIP_LFILE_CONTENT(zipos->map + lf),
                        GetZipLfileCompressedSize(zipos->map + lf),
                        &ds) == -1) {
            munmap(res, size2);
            res = 0;
          }
          break;
        default:
          munmap(res, size2);
          res = 0;
          break;
      }
    }
  }
  STRACE("GetSymbolTableFromZip() → %p", res);
  return res;
}

/**
 * Reads symbol table from .com.dbg file.
 * @note This code can't depend on dlmalloc()
 */
noasan static struct SymbolTable *GetSymbolTableFromElf(void) {
  return OpenSymbolTable(FindDebugBinary());
}

/**
 * Returns symbol table singleton.
 *
 * This uses multiple strategies to find the symbol table. The first
 * strategy, depends on whether or not the following is linked:
 *
 *     STATIC_YOINK("__zipos_get");
 *
 * In that case, the symbol table may be read from `/zip/.symtab` which
 * is generated by `o//tool/build/symtab.com`. The second strategy is to
 * look for the concomitant `.com.dbg` executable, which may very well
 * be the one currently executing, or it could be placed in the same
 * folder as your `.com` binary, or lastly, it could be explicitly
 * specified via the `COMDBG` environment variable.
 *
 * Function tracing is disabled throughout the duration of this call.
 * Backtraces and other core runtime functionality depend on this.
 *
 * @return symbol table, or NULL w/ errno on first call
 */
noasan struct SymbolTable *GetSymbolTable(void) {
  int ft, st;
  struct Zipos *z;
  static struct SymbolTable *t;
  if (!t) {
    ft = g_ftrace, g_ftrace = 0;
    st = __strace, __strace = 0;
    if (weaken(__zipos_get) && (z = weaken(__zipos_get)())) {
      if ((t = GetSymbolTableFromZip(z))) {
        t->names = (uint32_t *)((char *)t + t->names_offset);
        t->name_base = (char *)((char *)t + t->name_base_offset);
      }
    }
    if (!t) {
      t = GetSymbolTableFromElf();
    }
    g_ftrace = ft;
    __strace = st;
  }
  return t;
}
