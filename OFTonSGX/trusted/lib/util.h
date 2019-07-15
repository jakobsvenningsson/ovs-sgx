/*
 * util.h
 *
 *  Created on: Apr 23, 2018
 *      Author: jorge
 */

#ifndef ENCLAVE_MYENCLAVE_TRUSTED_LIB_UTIL_H_
#define ENCLAVE_MYENCLAVE_TRUSTED_LIB_UTIL_H_

#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "compiler.h"
#include "sgx_htons.h"

#define typeof __typeof__

#ifndef va_copy
#ifdef __va_copy
#define va_copy __va_copy
#else
#define va_copy(dst, src) ((dst) = (src))
#endif
#endif

#ifdef __CHECKER__
#define BUILD_ASSERT(EXPR) ((void) 0)
#define BUILD_ASSERT_DECL(EXPR) extern int (*build_assert(void))[1]
#elif !defined(__cplusplus)
/* Build-time assertion building block. */
#define BUILD_ASSERT__(EXPR) \
        sizeof(struct { unsigned int build_assert_failed : (EXPR) ? 1 : -1; })

/* Build-time assertion for use in a statement context. */
#define BUILD_ASSERT(EXPR) (void) BUILD_ASSERT__(EXPR)

/* Build-time assertion for use in a declaration context. */
#define BUILD_ASSERT_DECL(EXPR) \
        extern int (*build_assert(void))[BUILD_ASSERT__(EXPR)]
#else /* __cplusplus */
#include <boost/static_assert.hpp>
#define BUILD_ASSERT BOOST_STATIC_ASSERT
#define BUILD_ASSERT_DECL BOOST_STATIC_ASSERT
#endif /* __cplusplus */

#ifdef __GNUC__
#define BUILD_ASSERT_GCCONLY(EXPR) BUILD_ASSERT(EXPR)
#define BUILD_ASSERT_DECL_GCCONLY(EXPR) BUILD_ASSERT_DECL(EXPR)
#else
#define BUILD_ASSERT_GCCONLY(EXPR) ((void) 0)
#define BUILD_ASSERT_DECL_GCCONLY(EXPR) ((void) 0)
#endif

/* Like the standard assert macro, except:
 *
 *   - Writes the failure message to the log.
 *
 *   - Not affected by NDEBUG. */


#define ovs_assert(CONDITION)                                           \
    if (!OVS_LIKELY(CONDITION)) {                                       \
        ovs_assert_failure(SOURCE_LOCATOR, __func__, #CONDITION);       \
    }
void ovs_assert_failure(const char *, const char *, const char *) NO_RETURN;

/* Casts 'pointer' to 'type' and issues a compiler warning if the cast changes
 anything other than an outermost "const" or "volatile" qualifier.
 The cast to int is present only to suppress an "expression using sizeof
  bool" warning from "sparse" (see
http://permalink.gmane.org/gmane.comp.parsers.sparse/2967).*/



#define CONST_CAST(TYPE, POINTER)                               \
    ((void) sizeof ((int) ((POINTER) == (TYPE) (POINTER))),     \
     (TYPE) (POINTER))

/* Jorge Medina
extern const char *program_name;
extern const char *subprogram_name;
*/

/* Returns the number of elements in ARRAY. */
#define ARRAY_SIZE(ARRAY) (sizeof ARRAY / sizeof *ARRAY)

/* Returns X / Y, rounding up.  X must be nonnegative to round correctly. */
#define DIV_ROUND_UP(X, Y) (((X) + ((Y) - 1)) / (Y))

/* Returns X rounded up to the nearest multiple of Y. */
#define ROUND_UP(X, Y) (DIV_ROUND_UP(X, Y) * (Y))

/* Returns X rounded down to the nearest multiple of Y. */
#define ROUND_DOWN(X, Y) ((X) / (Y) * (Y))

/* Returns true if X is a power of 2, otherwise false. */
#define IS_POW2(X) ((X) && !((X) & ((X) - 1)))

static inline bool
is_pow2(uintmax_t x)
{
    return IS_POW2(x);
}

#ifndef MIN
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef MAX
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#endif

#define NOT_REACHED() abort()

/* Expands to a string that looks like "<file>:<line>", e.g. "tmp.c:10".
 *
 * See http://c-faq.com/ansi/stringize.html for an explanation of STRINGIZE and
 * STRINGIZE2. */

#define SOURCE_LOCATOR __FILE__ ":" STRINGIZE(__LINE__)
#define STRINGIZE(ARG) STRINGIZE2(ARG)
#define STRINGIZE2(ARG) #ARG

/* Given a pointer-typed lvalue OBJECT, expands to a pointer type that may be
 * assigned to OBJECT. */
#ifdef __GNUC__
#define OVS_TYPEOF(OBJECT) typeof(OBJECT)
#else
#define OVS_TYPEOF(OBJECT) void *
#endif

/* Given OBJECT of type pointer-to-structure, expands to the offset of MEMBER
 * within an instance of the structure.
 *
 * The GCC-specific version avoids the technicality of undefined behavior if
 * OBJECT is null, invalid, or not yet initialized.  This makes some static
 * checkers (like Coverity) happier.  But the non-GCC version does not actually
 * dereference any pointer, so it would be surprising for it to cause any
 * problems in practice.
 */
#ifdef __GNUC__
#define OBJECT_OFFSETOF(OBJECT, MEMBER) offsetof(typeof(*(OBJECT)), MEMBER)
#else
#define OBJECT_OFFSETOF(OBJECT, MEMBER) \
    ((char *) &(OBJECT)->MEMBER - (char *) (OBJECT))
#endif

/* Given POINTER, the address of the given MEMBER in a STRUCT object, returns
   the STRUCT object. */
#define CONTAINER_OF(POINTER, STRUCT, MEMBER)                           \
        ((STRUCT *) (void *) ((char *) (POINTER) - offsetof (STRUCT, MEMBER)))

/* Given POINTER, the address of the given MEMBER within an object of the type
 * that that OBJECT points to, returns OBJECT as an assignment-compatible
 * pointer type (either the correct pointer type or "void *").  OBJECT must be
 * an lvalue.
 *
 * This is the same as CONTAINER_OF except that it infers the structure type
 * from the type of '*OBJECT'. */
#define OBJECT_CONTAINING(POINTER, OBJECT, MEMBER)                      \
    ((OVS_TYPEOF(OBJECT)) (void *)                                      \
     ((char *) (POINTER) - OBJECT_OFFSETOF(OBJECT, MEMBER)))

/* Given POINTER, the address of the given MEMBER within an object of the type
 * that that OBJECT points to, assigns the address of the outer object to
 * OBJECT, which must be an lvalue.
 *
 * Evaluates to 1. */
#define ASSIGN_CONTAINER(OBJECT, POINTER, MEMBER) \
    ((OBJECT) = OBJECT_CONTAINING(POINTER, OBJECT, MEMBER), 1)

#ifdef  __cplusplus
extern "C" {

#endif

void *xcalloc(size_t, size_t) MALLOC_LIKE;
void *xzalloc(size_t) MALLOC_LIKE;
char *xmemdup0(const char *, size_t) MALLOC_LIKE;
void *xmemdup(const void *, size_t) MALLOC_LIKE;
void *xrealloc(void *, size_t);

//lines 241
/* Bitwise tests. */

/* Returns the number of trailing 0-bits in 'n'.  Undefined if 'n' == 0.
 *
 * This compiles to a single machine instruction ("bsf") with GCC on x86. */
#if !defined(UINT_MAX) || !defined(UINT32_MAX)
#error "Someone screwed up the #includes."
#elif __GNUC__ >= 4 && UINT_MAX == UINT32_MAX
static inline int
raw_ctz(uint32_t n)
{
    return __builtin_ctz(n);
}
#else
/* Defined in util.c. */
int raw_ctz(uint32_t n);
#endif

//258

/* Returns the rightmost 1-bit in 'x' (e.g. 01011000 => 00001000), or 0 if 'x'
 * is 0. */
static inline uintmax_t
rightmost_1bit(uintmax_t x)
{
    return x & -x;
}

/* Returns 'x' with its rightmost 1-bit changed to a zero (e.g. 01011000 =>
 * 01010000), or 0 if 'x' is 0. */
static inline uintmax_t
zero_rightmost_1bit(uintmax_t x)
{
    return x & (x - 1);
}

static inline uint32_t get_unaligned_u32(const uint32_t *p_)
{
    const uint8_t *p = (const uint8_t *) p_;
    return ntohl((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

void bitwise_zero(void *dst_, unsigned int dst_len, unsigned dst_ofs,
                  unsigned int n_bits);



static inline void *
xmalloc(size_t size)
{
  void *p = malloc(size ? size : 1);
  //COVERAGE_INC(util_xalloc);
  if (p == NULL) {
      abort();
  }
  return p;
}

#endif /* ENCLAVE_MYENCLAVE_TRUSTED_LIB_UTIL_H_ */
