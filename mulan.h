// No copyright
#pragma once

#include <stdint.h>
#include <string.h>


#define MULAN_HDR 10

#if defined(__cplusplus)
# define MULAN_CONSTEXPR_ constexpr
#else
# define MULAN_CONSTEXPR_
#endif

#if defined(__cplusplus)
namespace mulan {
#endif

static inline MULAN_CONSTEXPR_ uint64_t hash_n(const char* str, size_t n) {
  if (n == 0) return 0;

  uint64_t ret = 0;
  size_t   i   = 0;

  switch (n%8) {
  case 0: do { ret ^= ((uint64_t) str[i++] << (0*8));  /* FALL THROUGH */
  case 7:      ret ^= ((uint64_t) str[i++] << (7*8));  /* FALL THROUGH */
  case 6:      ret ^= ((uint64_t) str[i++] << (6*8));  /* FALL THROUGH */
  case 5:      ret ^= ((uint64_t) str[i++] << (5*8));  /* FALL THROUGH */
  case 4:      ret ^= ((uint64_t) str[i++] << (4*8));  /* FALL THROUGH */
  case 3:      ret ^= ((uint64_t) str[i++] << (3*8));  /* FALL THROUGH */
  case 2:      ret ^= ((uint64_t) str[i++] << (2*8));  /* FALL THROUGH */
  case 1:      ret ^= ((uint64_t) str[i++] << (1*8));  /* FALL THROUGH */
          } while (i < n);
  }
  return ret;
}
static inline MULAN_CONSTEXPR_ uint64_t hash(const char* str) {
  return hash_n(str, strlen(str));
}

static inline void unpack_header(
    const char src[MULAN_HDR], uint64_t* id, uint16_t* strn) {
  *id =
      ((uint64_t) src[7] << (7*8)) |
      ((uint64_t) src[6] << (6*8)) |
      ((uint64_t) src[5] << (5*8)) |
      ((uint64_t) src[4] << (4*8)) |
      ((uint64_t) src[3] << (3*8)) |
      ((uint64_t) src[2] << (2*8)) |
      ((uint64_t) src[1] << (1*8)) |
      ((uint64_t) src[0] << (0*8));
  *strn =
      ((uint16_t) src[9] << 8) |
      ((uint16_t) src[8] << 0);
}

static inline void pack_header(
    char dst[MULAN_HDR], uint64_t id, uint16_t strn) {
  dst[0] = (id >> (0*8)) & 0xFF;
  dst[1] = (id >> (1*8)) & 0xFF;
  dst[2] = (id >> (2*8)) & 0xFF;
  dst[3] = (id >> (3*8)) & 0xFF;
  dst[4] = (id >> (4*8)) & 0xFF;
  dst[5] = (id >> (5*8)) & 0xFF;
  dst[6] = (id >> (6*8)) & 0xFF;
  dst[7] = (id >> (7*8)) & 0xFF;

  dst[8] = (strn >> (0*8)) & 0xFF;
  dst[9] = (strn >> (1*8)) & 0xFF;
}

#if defined(__cplusplus)
}  // namespace mulan
#endif

#undef MULAN_CONSTEXPR_
