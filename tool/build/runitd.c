/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2021 Justine Alexandra Roberts Tunney                              │
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
#include "libc/bits/bits.h"
#include "libc/calls/calls.h"
#include "libc/calls/sigbits.h"
#include "libc/errno.h"
#include "libc/fmt/conv.h"
#include "libc/log/check.h"
#include "libc/log/log.h"
#include "libc/nexgen32e/crc32.h"
#include "libc/runtime/gc.internal.h"
#include "libc/runtime/runtime.h"
#include "libc/sock/sock.h"
#include "libc/stdio/stdio.h"
#include "libc/str/str.h"
#include "libc/sysv/consts/af.h"
#include "libc/sysv/consts/ex.h"
#include "libc/sysv/consts/exit.h"
#include "libc/sysv/consts/f.h"
#include "libc/sysv/consts/fd.h"
#include "libc/sysv/consts/inaddr.h"
#include "libc/sysv/consts/ipproto.h"
#include "libc/sysv/consts/itimer.h"
#include "libc/sysv/consts/o.h"
#include "libc/sysv/consts/poll.h"
#include "libc/sysv/consts/sa.h"
#include "libc/sysv/consts/so.h"
#include "libc/sysv/consts/sock.h"
#include "libc/sysv/consts/sol.h"
#include "libc/sysv/consts/w.h"
#include "libc/time/time.h"
#include "libc/x/x.h"
#include "net/https/https.h"
#include "third_party/getopt/getopt.h"
#include "third_party/mbedtls/ssl.h"
#include "tool/build/lib/eztls.h"
#include "tool/build/lib/psk.h"
#include "tool/build/runit.h"

/**
 * @fileoverview Remote test runner daemon.
 * Delivers 10x latency improvement over SSH (100x if Debian defaults)
 *
 * Here's how it handles connections:
 *
 * 1. Receives atomically-written request header, comprised of:
 *
 *   - 4 byte nbo magic = 0xFEEDABEEu
 *   - 1 byte command = kRunitExecute
 *   - 4 byte nbo name length in bytes, e.g. "test1"
 *   - 4 byte nbo executable file length in bytes
 *   - <name bytes> (no NUL terminator)
 *   - <file bytes> (it's binary data)
 *
 * 2. Runs program, after verifying it came from the IP that spawned
 *    this program via SSH. Be sure to only run this over a trusted
 *    physically-wired network. To use this software on untrustworthy
 *    networks, wrap it with stunnel and use your own CA.
 *
 * 3. Sends stdout/stderr fragments, potentially multiple times:
 *
 *   - 4 byte nbo magic = 0xFEEDABEEu
 *   - 1 byte command = kRunitStdout/Stderr
 *   - 4 byte nbo byte length
 *   - <chunk bytes>
 *
 * 4. Sends process exit code:
 *
 *   - 4 byte nbo magic = 0xFEEDABEEu
 *   - 1 byte command = kRunitExit
 *   - 1 byte exit status
 */

#define DEATH_CLOCK_SECONDS 128

#define kLogFile     "o/runitd.log"
#define kLogMaxBytes (2 * 1000 * 1000)

bool use_ftrace;
bool use_strace;
char *g_exepath;
volatile bool g_interrupted;
struct sockaddr_in g_servaddr;
unsigned char g_buf[PAGESIZE];
bool g_daemonize, g_sendready, g_alarmed;
int g_timeout, g_devnullfd, g_servfd, g_clifd, g_exefd;

void OnInterrupt(int sig) {
  g_interrupted = true;
}

void OnChildTerminated(int sig) {
  int ws, pid;
  for (;;) {
    if ((pid = waitpid(-1, &ws, WNOHANG)) != -1) {
      if (pid) {
        if (WIFEXITED(ws)) {
          DEBUGF("worker %d exited with %d", pid, WEXITSTATUS(ws));
        } else {
          DEBUGF("worker %d terminated with %s", pid, strsignal(WTERMSIG(ws)));
        }
      } else {
        break;
      }
    } else {
      if (errno == EINTR) continue;
      if (errno == ECHILD) break;
      FATALF("waitpid failed in sigchld");
    }
  }
}

wontreturn void ShowUsage(FILE *f, int rc) {
  fprintf(f, "%s: %s %s\n", "Usage", program_invocation_name,
          "[-d] [-r] [-l LISTENIP] [-p PORT] [-t TIMEOUTMS]");
  exit(rc);
}

void GetOpts(int argc, char *argv[]) {
  int opt;
  g_timeout = RUNITD_TIMEOUT_MS;
  g_servaddr.sin_family = AF_INET;
  g_servaddr.sin_port = htons(RUNITD_PORT);
  g_servaddr.sin_addr.s_addr = INADDR_ANY;
  while ((opt = getopt(argc, argv, "fqhvsdrl:p:t:w:")) != -1) {
    switch (opt) {
      case 'f':
        use_ftrace = true;
        break;
      case 's':
        use_strace = true;
        break;
      case 'q':
        --__log_level;
        break;
      case 'v':
        ++__log_level;
        break;
      case 'd':
        g_daemonize = true;
        break;
      case 'r':
        g_sendready = true;
        break;
      case 't':
        g_timeout = atoi(optarg);
        break;
      case 'p':
        CHECK_NE(0xFFFF, (g_servaddr.sin_port = htons(parseport(optarg))));
        break;
      case 'l':
        CHECK_EQ(1, inet_pton(AF_INET, optarg, &g_servaddr.sin_addr));
        break;
      case 'h':
        ShowUsage(stdout, EXIT_SUCCESS);
        unreachable;
      default:
        ShowUsage(stderr, EX_USAGE);
        unreachable;
    }
  }
}

dontdiscard char *DescribeAddress(struct sockaddr_in *addr) {
  char ip4buf[16];
  return xasprintf("%s:%hu",
                   inet_ntop(addr->sin_family, &addr->sin_addr.s_addr, ip4buf,
                             sizeof(ip4buf)),
                   ntohs(addr->sin_port));
}

void StartTcpServer(void) {
  int yes = true;
  uint32_t asize;

  /*
   * TODO: How can we make close(serversocket) on Windows go fast?
   *       That way we can put back SOCK_CLOEXEC.
   */
  CHECK_NE(-1, (g_servfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)));
  CHECK_NE(-1, dup2(g_servfd, 10));
  CHECK_NE(-1, close(g_servfd));
  g_servfd = 10;

  LOGIFNEG1(setsockopt(g_servfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)));
  if (bind(g_servfd, &g_servaddr, sizeof(g_servaddr)) == -1) {
    if (g_servaddr.sin_port != 0) {
      g_servaddr.sin_port = 0;
      StartTcpServer();
      return;
    } else {
      FATALF("bind failed %m");
    }
  }
  CHECK_NE(-1, listen(g_servfd, 10));
  asize = sizeof(g_servaddr);
  CHECK_NE(-1, getsockname(g_servfd, &g_servaddr, &asize));
  INFOF("%s:%s", "listening on tcp", gc(DescribeAddress(&g_servaddr)));
  if (g_sendready) {
    printf("ready %hu\n", ntohs(g_servaddr.sin_port));
    fflush(stdout);
    fclose(stdout);
    dup2(g_devnullfd, stdout->fd);
  }
}

void SendExitMessage(int rc) {
  unsigned char msg[4 + 1 + 1];
  msg[0 + 0] = (RUNITD_MAGIC & 0xff000000) >> 030;
  msg[0 + 1] = (RUNITD_MAGIC & 0x00ff0000) >> 020;
  msg[0 + 2] = (RUNITD_MAGIC & 0x0000ff00) >> 010;
  msg[0 + 3] = (RUNITD_MAGIC & 0x000000ff) >> 000;
  msg[4] = kRunitExit;
  msg[5] = rc;
  CHECK_EQ(sizeof(msg), mbedtls_ssl_write(&ezssl, msg, sizeof(msg)));
  CHECK_EQ(0, EzTlsFlush(&ezbio, 0, 0));
}

void SendOutputFragmentMessage(enum RunitCommand kind, unsigned char *buf,
                               size_t size) {
  ssize_t rc;
  size_t sent;
  unsigned char msg[4 + 1 + 4];
  msg[0 + 0] = (RUNITD_MAGIC & 0xff000000) >> 030;
  msg[0 + 1] = (RUNITD_MAGIC & 0x00ff0000) >> 020;
  msg[0 + 2] = (RUNITD_MAGIC & 0x0000ff00) >> 010;
  msg[0 + 3] = (RUNITD_MAGIC & 0x000000ff) >> 000;
  msg[4 + 0] = kind;
  msg[5 + 0] = (size & 0xff000000) >> 030;
  msg[5 + 1] = (size & 0x00ff0000) >> 020;
  msg[5 + 2] = (size & 0x0000ff00) >> 010;
  msg[5 + 3] = (size & 0x000000ff) >> 000;
  CHECK_EQ(sizeof(msg), mbedtls_ssl_write(&ezssl, msg, sizeof(msg)));
  while (size) {
    CHECK_NE(-1, (rc = mbedtls_ssl_write(&ezssl, buf, size)));
    CHECK_LE((sent = (size_t)rc), size);
    size -= sent;
    buf += sent;
  }
  CHECK_EQ(0, EzTlsFlush(&ezbio, 0, 0));
}

void OnAlarm(int sig) {
  g_alarmed = true;
}

void SetDeadline(int seconds, int micros) {
  g_alarmed = false;
  LOGIFNEG1(
      sigaction(SIGALRM, &(struct sigaction){.sa_handler = OnAlarm}, NULL));
  LOGIFNEG1(setitimer(
      ITIMER_REAL, &(const struct itimerval){{0, 0}, {seconds, micros}}, NULL));
}

void Recv(void *p, size_t n) {
  size_t i, rc;
  for (i = 0; i < n; i += rc) {
    do {
      rc = mbedtls_ssl_read(&ezssl, (char *)p + i, n - i);
      DEBUGF("read(%ld)", rc);
    } while (rc == MBEDTLS_ERR_SSL_WANT_READ);
    if (rc <= 0) TlsDie("read failed", rc);
  }
  DEBUGF("Recv(%ld)", n);
}

void HandleClient(void) {
  const size_t kMaxNameSize = 128;
  const size_t kMaxFileSize = 10 * 1024 * 1024;
  uint32_t crc;
  ssize_t got, wrote;
  struct sockaddr_in addr;
  sigset_t chldmask, savemask;
  char *addrstr, *exename, *exe;
  unsigned char msg[4 + 1 + 4 + 4 + 4];
  struct sigaction ignore, saveint, savequit;
  int rc, exitcode, wstatus, child, pipefds[2];
  uint32_t addrsize, namesize, filesize, remaining;

  /* read request to run program */
  addrsize = sizeof(addr);
  CHECK_NE(-1, (g_clifd = accept4(g_servfd, &addr, &addrsize, SOCK_CLOEXEC)));
  if (fork()) {
    close(g_clifd);
    return;
  }
  EzFd(g_clifd);
  EzHandshake();
  addrstr = gc(DescribeAddress(&addr));
  DEBUGF("%s %s %s", gc(DescribeAddress(&g_servaddr)), "accepted", addrstr);
  Recv(msg, sizeof(msg));
  CHECK_EQ(RUNITD_MAGIC, READ32BE(msg));
  CHECK_EQ(kRunitExecute, msg[4]);
  namesize = READ32BE(msg + 5);
  filesize = READ32BE(msg + 9);
  crc = READ32BE(msg + 13);
  exename = gc(calloc(1, namesize + 1));
  Recv(exename, namesize);
  g_exepath = gc(xasprintf("o/%d.%s", getpid(), basename(exename)));
  INFOF("%s asked we run %`'s (%,u bytes @ %`'s)", addrstr, exename, filesize,
        g_exepath);

  exe = malloc(filesize);
  Recv(exe, filesize);
  if (crc32_z(0, exe, filesize) != crc) {
    FATALF("%s crc mismatch! %`'s", addrstr, exename);
  }
  CHECK_NE(-1, (g_exefd = creat(g_exepath, 0700)));
  LOGIFNEG1(ftruncate(g_exefd, filesize));
  CHECK_NE(-1, xwrite(g_exefd, exe, filesize));
  LOGIFNEG1(close(g_exefd));

  /* run program, tee'ing stderr to both log and client */
  DEBUGF("spawning %s", exename);
  SetDeadline(DEATH_CLOCK_SECONDS, 0);
  ignore.sa_flags = 0;
  ignore.sa_handler = SIG_IGN;
  LOGIFNEG1(sigemptyset(&ignore.sa_mask));
  LOGIFNEG1(sigaction(SIGINT, &ignore, &saveint));
  LOGIFNEG1(sigaction(SIGQUIT, &ignore, &savequit));
  LOGIFNEG1(sigemptyset(&chldmask));
  LOGIFNEG1(sigaddset(&chldmask, SIGCHLD));
  LOGIFNEG1(sigprocmask(SIG_BLOCK, &chldmask, &savemask));
  CHECK_NE(-1, pipe2(pipefds, O_CLOEXEC));
  CHECK_NE(-1, (child = fork()));
  if (!child) {
    sigaction(SIGINT, &saveint, NULL);
    sigaction(SIGQUIT, &savequit, NULL);
    sigprocmask(SIG_SETMASK, &savemask, NULL);
    dup2(g_devnullfd, 0);
    dup2(pipefds[1], 1);
    dup2(pipefds[1], 2);
    if (pipefds[0] > 2) close(pipefds[1]);
    if (g_devnullfd > 2) close(g_devnullfd);
    int i = 0;
    char *args[4] = {0};
    args[i++] = g_exepath;
    if (use_strace) args[i++] = "--strace";
    if (use_ftrace) args[i++] = "--ftrace";
    execv(g_exepath, args);
    _exit(127);
  }
  LOGIFNEG1(close(pipefds[1]));
  DEBUGF("communicating %s[%d]", exename, child);
  while (!g_alarmed) {
    got = read(pipefds[0], g_buf, sizeof(g_buf));
    if (got != -1) {
      if (!got) {
        close(pipefds[0]);
        break;
      }
      fwrite(g_buf, got, 1, stderr);
      SendOutputFragmentMessage(kRunitStderr, g_buf, got);
    } else {
      CHECK_EQ(EINTR, errno);
    }
  }
  for (;;) {
    if (g_alarmed) {
      WARNF("killing %s which timed out");
      LOGIFNEG1(kill(child, SIGKILL));
      g_alarmed = false;
    }
    if (waitpid(child, &wstatus, 0) != -1) {
      break;
    } else {
      CHECK_EQ(EINTR, errno);
    }
  }
  if (WIFEXITED(wstatus)) {
    if (WEXITSTATUS(wstatus)) {
      WARNF("%s exited with %d", exename, WEXITSTATUS(wstatus));
    } else {
      DEBUGF("%s exited with %d", exename, WEXITSTATUS(wstatus));
    }
    exitcode = WEXITSTATUS(wstatus);
  } else {
    WARNF("%s terminated with %s", exename, strsignal(WTERMSIG(wstatus)));
    exitcode = 128 + WTERMSIG(wstatus);
  }
  LOGIFNEG1(sigaction(SIGINT, &saveint, NULL));
  LOGIFNEG1(sigaction(SIGQUIT, &savequit, NULL));
  LOGIFNEG1(sigprocmask(SIG_SETMASK, &savemask, NULL));

  /* let client know how it went */
  if (unlink(g_exepath) == -1) {
    WARNF("failed to delete executable %`'s", g_exepath);
  }
  SendExitMessage(exitcode);
  mbedtls_ssl_close_notify(&ezssl);
  LOGIFNEG1(close(g_clifd));
  _exit(0);
}

int Poll(void) {
  int i, wait, evcount;
  struct pollfd fds[1];
TryAgain:
  if (g_interrupted) return 0;
  fds[0].fd = g_servfd;
  fds[0].events = POLLIN;
  wait = MIN(1000, g_timeout);
  evcount = poll(fds, ARRAYLEN(fds), wait);
  if (!evcount) g_timeout -= wait;
  if (evcount == -1 && errno == EINTR) goto TryAgain;
  CHECK_NE(-1, evcount);
  for (i = 0; i < evcount; ++i) {
    CHECK(fds[i].revents & POLLIN);
    HandleClient();
  }
  /* manually do this because of nt */
  while (waitpid(-1, NULL, WNOHANG) > 0) donothing;
  return evcount;
}

int Serve(void) {
  StartTcpServer();
  sigaction(SIGINT, (&(struct sigaction){.sa_handler = (void *)OnInterrupt}),
            NULL);
  sigaction(SIGCHLD,
            (&(struct sigaction){.sa_handler = (void *)OnChildTerminated,
                                 .sa_flags = SA_RESTART}),
            NULL);
  for (;;) {
    if (!Poll() && (!g_timeout || g_interrupted)) break;
  }
  close(g_servfd);
  if (!g_timeout) {
    INFOF("timeout expired, shutting down");
  } else {
    INFOF("got ctrl-c, shutting down");
  }
  return 0;
}

void Daemonize(void) {
  struct stat st;
  if (fork() > 0) _exit(0);
  setsid();
  if (fork() > 0) _exit(0);
  dup2(g_devnullfd, stdin->fd);
  if (!g_sendready) dup2(g_devnullfd, stdout->fd);
  freopen(kLogFile, "ae", stderr);
  if (fstat(fileno(stderr), &st) != -1 && st.st_size > kLogMaxBytes) {
    ftruncate(fileno(stderr), 0);
  }
}

int main(int argc, char *argv[]) {
  ShowCrashReports();
  SetupPresharedKeySsl(MBEDTLS_SSL_IS_SERVER, GetRunitPsk());
  /* __log_level = kLogDebug; */
  GetOpts(argc, argv);
  CHECK_EQ(3, (g_devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC)));
  defer(close_s, &g_devnullfd);
  if (!isdirectory("o")) CHECK_NE(-1, mkdir("o", 0700));
  if (g_daemonize) Daemonize();
  return Serve();
}
