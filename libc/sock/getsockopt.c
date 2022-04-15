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
#include "libc/calls/internal.h"
#include "libc/calls/strace.internal.h"
#include "libc/dce.h"
#include "libc/sock/internal.h"
#include "libc/sock/sock.h"
#include "libc/sysv/errfuns.h"

/**
 * Retrieves socket setting.
 *
 * @param level can be SOL_SOCKET, IPPROTO_TCP, etc.
 * @param optname can be SO_{REUSE{PORT,ADDR},KEEPALIVE,etc.} etc.
 * @return 0 on success, or -1 w/ errno
 * @error ENOPROTOOPT for unknown (level,optname)
 * @see libc/sysv/consts.sh for tuning catalogue
 * @see setsockopt()
 */
int getsockopt(int fd, int level, int optname, void *out_opt_optval,
               uint32_t *out_optlen) {
  int rc;
  if (!level || !optname) {
    rc = enoprotoopt(); /* our sysvconsts definition */
  } else if (optname == -1) {
    rc = 0; /* our sysvconsts definition */
  } else if (!IsWindows()) {
    rc = sys_getsockopt(fd, level, optname, out_opt_optval, out_optlen);
  } else if (__isfdkind(fd, kFdSocket)) {
    rc = sys_getsockopt_nt(&g_fds.p[fd], level, optname, out_opt_optval,
                           out_optlen);
  } else {
    rc = ebadf();
  }
  STRACE("getsockopt(%d, %#x, %#x, %p, %p) → %d% lm", fd, level, optname,
         out_opt_optval, out_optlen, rc);
  return rc;
}
