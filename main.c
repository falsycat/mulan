// No copyright
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

#include "mulan.h"

size_t line_n = 0;

_Noreturn void error(const char* msg) {
  fprintf(stderr, "line %zu: %s\n", line_n, msg);
  exit(1);
}

void get_quote_content(const char** str, size_t* n) {
  while (*n && **str != '"') ++*str, --*n;
  if (**str != '"') error("no quote found");
  if (*n == 0)      error("unexpected line end after first quote");
  ++*str, --*n;

  const char* end = *str;
  while (*n && *end != '"') ++end, --*n;
  if (*end != '"') error("no quote found");

  *n = end - *str;
}

void append_quote_content(
    char* dst, size_t* dstn, size_t maxdstn, const char* src, size_t srcn) {
  get_quote_content(&src, &srcn);
  if (*dstn+srcn > maxdstn) error("too long value");

  strncpy(dst+*dstn, src, srcn);
  *dstn += srcn;
}

uint8_t to_hex(char c) {
  if ('0' <= c && c <= '9') return c-'0';
  if ('A' <= c && c <= 'F') return c-'A';
  if ('a' <= c && c <= 'f') return c-'a';
  error("invalid hex char");
}

uint8_t parse_hex(char h, char l) {
  const int8_t high = to_hex(h);
  const int8_t low  = to_hex(l);
  return (high << 4) & low;
}

void unescape(char* str, size_t* n) {
  char* begin = str;

  char* src = str;
  char* end = str + *n;

  char* dst = str;

  enum {
    NORMAL,
    ESC,
    HEX,
  } state = NORMAL;

  char* left = str;
  while (src < end) {
    switch (state) {
    case NORMAL:
      if (*src == '\\') {
        state = ESC;
      } else {
        *(dst++) = *src;
      }
      break;

    case ESC:
      switch (*src) {
      case 'n':
        *(dst++) = '\n', state = NORMAL;
        break;
      case 'x':
        left = src, state = HEX;
        break;
      default:
        error("unknown escseq");
      }
      break;

    case HEX:
      if (left+2 == src) {
        *(dst++) = parse_hex(*(src-1), *src);
        state    = NORMAL;
      }
      break;
    }
    ++src;
  }

  if (state != NORMAL) {
    error("escseq aborted unexpectedly");
  }
  *n = dst - begin;
}

_Noreturn void compile(const char* out, const char* in) {
  FILE* ifp = fopen(in, "r");
  if (!ifp) {
    fprintf(stderr, "input file open error\n");
    exit(1);
  }
  FILE* ofp = fopen(out, "wb");
  if (!ofp) {
    fprintf(stderr, "output file open error\n");
    exit(1);
  }

  char line[1024];

  char msgid[1024];
  size_t msgid_len = 0;

  char msgstr[1024];
  size_t msgstr_len = 0;

  enum {
    INITIAL,
    MSGID,
    MSGSTR,
  } state = INITIAL;

  bool eof = false;
  while (++line_n, !eof) {
    line[0] = '\n';
    if (fgets(line, sizeof(line), ifp) == 0) {
      eof = true;
    }

    const size_t line_len = strlen(line);
    if (line[line_len-1] != '\n') {
      error("too long line");
    }

  switch (state) {
    case INITIAL:
      if (strncmp(line, "msgid ", sizeof("msgid ")-1) == 0) {
        state = MSGID;
        append_quote_content(
            msgid, &msgid_len, sizeof(msgid), line, line_len);
      }
      break;
    case MSGID:
      if (line_len && line[0] == '"') {
        append_quote_content(
            msgid, &msgid_len, sizeof(msgid), line, line_len);

      } else {
        if (strncmp(line, "msgstr ", sizeof("msgstr ")-1) == 0) {
          state = MSGSTR;
          append_quote_content(
              msgstr, &msgstr_len, sizeof(msgstr), line, line_len);
        }
      }
      break;
    case MSGSTR:
      if (line_len && line[0] == '"') {
        append_quote_content(
            msgstr, &msgstr_len, sizeof(msgstr), line, line_len);
      } else {
        unescape(msgid,  &msgid_len);
        unescape(msgstr, &msgstr_len);

        if (msgstr_len >= UINT16_MAX) {
          error("too long msgstr");
        }

        uint8_t buf[MULAN_HDR];
        pack_header(buf, hash_n(msgid, msgid_len), (uint16_t) msgstr_len);
        if (fwrite(buf, sizeof(buf), 1, ofp) != 1) {
          error("header write error");
        }
        if (fwrite(msgstr, msgstr_len, 1, ofp) != 1) {
          error("body write error");
        }

        msgid_len  = 0;
        msgstr_len = 0;
        state = INITIAL;
      }
      break;
    }
  }
  fclose(ofp);
  fclose(ifp);

  exit(0);
}


_Noreturn void read_bin(const char* in) {
  FILE* fp = fopen(in, "rb");
  if (!fp) {
    fprintf(stderr, "output file open error\n");
    exit(1);
  }

  for (;;) {
    uint8_t hdr[10];
    if (!fread(hdr, sizeof(hdr), 1, fp)) break;

    uint64_t id;
    uint16_t strn;
    unpack_header(hdr, &id, &strn);

    char str[256];

    size_t strn_read = strn;
    if (strn_read >= sizeof(str)) {
      strn_read = sizeof(str);
    }
    if (!fread(str, strn_read, 1, fp)) {
      fprintf(stderr, "unexpected EOF\n");
      exit(1);
    }
    if (fseek(fp, strn-strn_read, SEEK_CUR)) {
      fprintf(stderr, "fseek failure\n");
      exit(1);
    }

    printf("[0x%"PRIX64"]\n", id);
    printf("%.*s\n\n", (int) strn_read, str);
  }
  exit(0);
}


int main(int argc, const char** args) {
  if (argc < 2) {
    fprintf(stderr, "invalid args\n");
    return 1;
  }
  if (argc < 2 || strcmp(args[1], "help") == 0) {
    printf("usage: %s [command]\n", args[0]);
    printf("  command:\n");
    printf("    help                      show this message\n");
    printf("    compile [output] [input]  compiles [input] into [output]\n");
    printf("    read [input]              reads all text from the compiled [input]\n");
    return 0;
  }

  if (strcmp(args[1], "compile") == 0) {
    if (argc != 4) {
      fprintf(stderr, "invalid args\n");
      return 1;
    }
    compile(args[2], args[3]);
  }
  if (strcmp(args[1], "read") == 0) {
    if (argc != 3) {
      fprintf(stderr, "invalid args\n");
      return 1;
    }
    read_bin(args[2]);
  }

  fprintf(stderr, "unknown command '%s'\n", args[1]);
  return 1;
}
