/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ This is free and unencumbered software released into the public domain.      │
│                                                                              │
│ Anyone is free to copy, modify, publish, use, compile, sell, or              │
│ distribute this software, either in source code form or as a compiled        │
│ binary, for any purpose, commercial or non-commercial, and by any            │
│ means.                                                                       │
│                                                                              │
│ In jurisdictions that recognize copyright laws, the author or authors        │
│ of this software dedicate any and all copyright interest in the              │
│ software to the public domain. We make this dedication for the benefit       │
│ of the public at large and to the detriment of our heirs and                 │
│ successors. We intend this dedication to be an overt act of                  │
│ relinquishment in perpetuity of all present and future rights to this        │
│ software under copyright law.                                                │
│                                                                              │
│ THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,              │
│ EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF           │
│ MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.       │
│ IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR            │
│ OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,        │
│ ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR        │
│ OTHER DEALINGS IN THE SOFTWARE.                                              │
│                                                                              │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "libc/dns/ent.h"
#include "libc/dns/servicestxt.h"

struct servent *getservbyport(int port, const char *proto) {
  static struct servent *ptr1, se1;
  char name[DNS_NAME_MAX];
  char *localproto = proto;

  if (!ptr1) {
    se1.s_name = NULL;
    se1.s_aliases = (char **)malloc(sizeof(char *) * 1);
    if (!se1.s_aliases) return NULL;
    se1.s_aliases[0] = NULL;

    se1.s_port = 0;
    se1.s_proto = NULL;
    ptr1 = &se1;
  }

  if (LookupServicesByPort(port, &localproto, name, sizeof(name)) == -1) {
    // localproto got alloc'd during the lookup?
    if (!proto && localproto != proto) free(localproto);
    return NULL;
  }

  ptr1->s_port = port;
  if (ptr1->s_name) free(ptr1->s_name);
  ptr1->s_name = strdup(name);

  if (ptr1->s_proto) free(ptr1->s_proto);
  ptr1->s_proto = strdup(localproto);

  if (!proto && localproto != proto) free(localproto);

  return ptr1;
}
