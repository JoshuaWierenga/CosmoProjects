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
#include "libc/bits/weaken.h"
#include "libc/calls/internal.h"
#include "libc/calls/sig.internal.h"
#include "libc/calls/strace.internal.h"
#include "libc/intrin/kprintf.h"
#include "libc/log/backtrace.internal.h"
#include "libc/nt/enum/wait.h"
#include "libc/nt/errors.h"
#include "libc/nt/struct/overlapped.h"
#include "libc/nt/winsock.h"
#include "libc/sock/internal.h"
#include "libc/sock/yoink.inc"
#include "libc/sysv/errfuns.h"

/**
 * Performs stream socket receive on New Technology.
 *
 * @param fd must be a socket
 * @return number of bytes received, or -1 w/ errno
 */
textwindows ssize_t sys_recv_nt(struct Fd *fd, const struct iovec *iov,
                                size_t iovlen, uint32_t flags) {
  ssize_t rc;
  uint32_t i, got = 0;
  struct NtIovec iovnt[16];
  struct NtOverlapped overlapped = {.hEvent = WSACreateEvent()};

  if (_check_interrupts(true, g_fds.p)) return eintr();

  if (!WSARecv(fd->handle, iovnt, __iovec2nt(iovnt, iov, iovlen), &got, &flags,
               &overlapped, NULL)) {
    rc = got;
    goto Finished;
  }

  if (WSAGetLastError() != kNtErrorIoPending) {
    STRACE("WSARecv failed %lm");
    rc = __winsockerr();
    goto Finished;
  }

  for (;;) {
    i = WSAWaitForMultipleEvents(1, &overlapped.hEvent, true,
                                 __SIG_POLLING_INTERVAL_MS, true);
    if (i == kNtWaitFailed) {
      STRACE("WSAWaitForMultipleEvents failed %lm");
      rc = __winsockerr();
      goto Finished;
    } else if (i == kNtWaitTimeout) {
      if (_check_interrupts(true, g_fds.p)) {
        rc = eintr();
        goto Finished;
      }
    } else if (i == kNtWaitIoCompletion) {
      STRACE("IOCP TRIGGERED EINTR");
    } else {
      break;
    }
  }

  if (!WSAGetOverlappedResult(fd->handle, &overlapped, &got, false, &flags)) {
    STRACE("WSAGetOverlappedResult failed %lm");
    rc = __winsockerr();
    goto Finished;
  }

  rc = got;
Finished:
  WSACloseEvent(overlapped.hEvent);
  return rc;
}
