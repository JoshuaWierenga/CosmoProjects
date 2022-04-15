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
#include "libc/calls/internal.h"
#include "libc/calls/sig.internal.h"
#include "libc/calls/strace.internal.h"
#include "libc/calls/struct/iovec.h"
#include "libc/errno.h"
#include "libc/intrin/kprintf.h"
#include "libc/limits.h"
#include "libc/nt/enum/wait.h"
#include "libc/nt/errors.h"
#include "libc/nt/files.h"
#include "libc/nt/ipc.h"
#include "libc/nt/runtime.h"
#include "libc/nt/struct/overlapped.h"
#include "libc/nt/synchronization.h"
#include "libc/sock/internal.h"
#include "libc/sysv/errfuns.h"

static textwindows ssize_t sys_read_nt_impl(struct Fd *fd, void *data,
                                            size_t size, ssize_t offset) {
  uint32_t err, got, avail;
  struct NtOverlapped overlap;
  if (fd->worker) {
    for (;;) {
      if (!PeekNamedPipe(fd->handle, 0, 0, 0, &avail, 0)) break;
      if (avail) break;
      if (SleepEx(__SIG_POLLING_INTERVAL_MS, true) == kNtWaitIoCompletion ||
          _check_interrupts(true, g_fds.p)) {
        return eintr();
      }
    }
  }
  if (ReadFile(fd->handle, data, _clampio(size), &got,
               _offset2overlap(fd->handle, offset, &overlap))) {
    return got;
  }
  err = GetLastError();
  // make sure read() returns 0 on broken pipe
  if (err == kNtErrorBrokenPipe) return 0;
  // make sure read() returns 0 on closing named pipe
  if (err == kNtErrorNoData) return 0;
  // make sure pread() returns 0 if we start reading after EOF
  if (err == kNtErrorHandleEof) return 0;
  return __winerr();
}

textwindows ssize_t sys_read_nt(struct Fd *fd, const struct iovec *iov,
                                size_t iovlen, ssize_t opt_offset) {
  ssize_t rc;
  uint32_t size;
  size_t i, total;
  if (_check_interrupts(true, fd)) return eintr();
  while (iovlen && !iov[0].iov_len) iov++, iovlen--;
  if (iovlen) {
    for (total = i = 0; i < iovlen; ++i) {
      if (!iov[i].iov_len) continue;
      rc = sys_read_nt_impl(fd, iov[i].iov_base, iov[i].iov_len, opt_offset);
      if (rc == -1) return -1;
      total += rc;
      if (opt_offset != -1) opt_offset += rc;
      if (rc < iov[i].iov_len) break;
    }
    return total;
  } else {
    return sys_read_nt_impl(fd, NULL, 0, opt_offset);
  }
}
