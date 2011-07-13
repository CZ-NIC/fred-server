/*
 * Copyright (c) 2008-2010 Stefan Krah. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include "mpdecimal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "basearith.h"
#include "bits.h"
#include "convolute.h"
#include "crt.h"
#include "errno.h"
#include "memory.h"
#include "typearith.h"
#include "umodarith.h"
#include "mptest.h"
#include "mptypes.h"

#ifdef PPRO
  #if defined(_MSC_VER)
    #include <float.h>
    #pragma fenv_access(on)
  #elif !defined(__OpenBSD__) && !defined(__NetBSD__)
    /* C99 */
    #include <fenv.h>
    #pragma STDC FENV_ACCESS ON
  #endif
#endif

#if defined(__x86_64__) && defined(__GLIBC__) && !defined(__INTEL_COMPILER)
  #define USE_80BIT_LONG_DOUBLE
#endif

#if defined(_MSC_VER)
  #define ALWAYS_INLINE __forceinline
#else
  #ifdef TEST_COVERAGE
    #define ALWAYS_INLINE
  #else
    #define ALWAYS_INLINE inline __attribute__ ((always_inline))
  #endif
#endif


#define MPD_NEWTONDIV_CUTOFF 1024L

#define MPD_NEW_STATIC(name, flags, exp, digits, len) \
        mpd_uint_t name##_data[MPD_MINALLOC_MAX];                    \
        mpd_t name = {flags|MPD_STATIC|MPD_STATIC_DATA, exp, digits, \
                      len, MPD_MINALLOC_MAX, name##_data}

#define MPD_NEW_CONST(name, flags, exp, digits, len, alloc, initval) \
        mpd_uint_t name##_data[alloc] = {initval};                   \
        mpd_t name = {flags|MPD_STATIC|MPD_CONST_DATA, exp, digits,  \
                      len, alloc, name##_data}

#define MPD_NEW_SHARED(name, a) \
        mpd_t name = {(a->flags&~MPD_DATAFLAGS)|MPD_STATIC|MPD_SHARED_DATA, \
                      a->exp, a->digits, a->len, a->alloc, a->data}


static mpd_uint_t data_one[1] = {1};
static mpd_uint_t data_zero[1] = {0};
static const mpd_t one = {MPD_STATIC|MPD_CONST_DATA, 0, 1, 1, 1, data_one};
static const mpd_t minus_one = {MPD_NEG|MPD_STATIC|MPD_CONST_DATA, 0, 1, 1, 1,
                                data_one};
static const mpd_t zero = {MPD_STATIC|MPD_CONST_DATA, 0, 1, 1, 1, data_zero};

static inline void _mpd_check_exp(mpd_t *dec, const mpd_context_t *ctx,
                                  uint32_t *status);
static void _settriple(mpd_t *result, uint8_t sign, mpd_uint_t a,
                       mpd_ssize_t exp);
static inline mpd_ssize_t _mpd_real_size(mpd_uint_t *data, mpd_ssize_t size);

static void _mpd_qadd(mpd_t *result, const mpd_t *a, const mpd_t *b,
                      const mpd_context_t *ctx, uint32_t *status);
static inline void _mpd_qmul(mpd_t *result, const mpd_t *a, const mpd_t *b,
                             const mpd_context_t *ctx, uint32_t *status);
static void _mpd_qbarrett_divmod(mpd_t *q, mpd_t *r, const mpd_t *a,
                                 const mpd_t *b, uint32_t *status);
static inline void _mpd_qpow_uint(mpd_t *result, mpd_t *base, mpd_uint_t exp,
                uint8_t resultsign, const mpd_context_t *ctx, uint32_t *status);

mpd_uint_t mpd_qsshiftr(mpd_t *result, const mpd_t *a, mpd_ssize_t n);


/******************************************************************************/
/*                  Performance critical inline functions                     */
/******************************************************************************/

#ifdef CONFIG_64
/* Digits in a word, primarily useful for the most significant word. */
ALWAYS_INLINE int
mpd_word_digits(mpd_uint_t word)
{
	if (word < mpd_pow10[9]) {
		if (word < mpd_pow10[4]) {
			if (word < mpd_pow10[2]) {
				return (word < mpd_pow10[1]) ? 1 : 2;
			}
			return (word < mpd_pow10[3]) ? 3 : 4;
		}
		if (word < mpd_pow10[6]) {
			return (word < mpd_pow10[5]) ? 5 : 6;
		}
		if (word < mpd_pow10[8]) {
			return (word < mpd_pow10[7]) ? 7 : 8;
		}
		return 9;
	}
	if (word < mpd_pow10[14]) {
		if (word < mpd_pow10[11]) {
			return (word < mpd_pow10[10]) ? 10 : 11;
		}
		if (word < mpd_pow10[13]) {
			return (word < mpd_pow10[12]) ? 12 : 13;
		}
		return 14;
	}
	if (word < mpd_pow10[17]) {
		if (word < mpd_pow10[16]) {
			return (word < mpd_pow10[15]) ? 15 : 16;
		}
		return 17;
	}

	return (word < mpd_pow10[18]) ? 18 : 19;
}
#else
ALWAYS_INLINE int
mpd_word_digits(mpd_uint_t word)
{
	if (word < mpd_pow10[4]) {
		if (word < mpd_pow10[2]) {
			return (word < mpd_pow10[1]) ? 1 : 2;
		}
		return (word < mpd_pow10[3]) ? 3 : 4;
	}
	if (word < mpd_pow10[6]) {
		return (word < mpd_pow10[5]) ? 5 : 6;
	}
	if (word < mpd_pow10[8]) {
		return (word < mpd_pow10[7]) ? 7 : 8;
	}

	return (word < mpd_pow10[9]) ? 9 : 10;
}
#endif


/* Adjusted exponent */
ALWAYS_INLINE mpd_ssize_t
mpd_adjexp(const mpd_t *dec)
{
	return (dec->exp + dec->digits) - 1;
}

/* Etiny */
ALWAYS_INLINE mpd_ssize_t
mpd_etiny(const mpd_context_t *ctx)
{
	return ctx->emin - (ctx->prec - 1);
}

/* Etop: used for folding down in IEEE clamping */
ALWAYS_INLINE mpd_ssize_t
mpd_etop(const mpd_context_t *ctx)
{
	return ctx->emax - (ctx->prec - 1);
}

/* Most significant word */
ALWAYS_INLINE mpd_uint_t
mpd_msword(const mpd_t *dec)
{
	assert(dec->len > 0);
	return dec->data[dec->len-1];
}

/* Most significant digit of a word */
inline mpd_uint_t
mpd_msd(mpd_uint_t word)
{
	int n;

	n = mpd_word_digits(word);
	return word / mpd_pow10[n-1];
}

/* Least significant digit of a word */
ALWAYS_INLINE mpd_uint_t
mpd_lsd(mpd_uint_t word)
{
	return word % 10;
}

/* Coefficient size needed to store 'digits' */
ALWAYS_INLINE mpd_ssize_t
mpd_digits_to_size(mpd_ssize_t digits)
{
	mpd_ssize_t q, r;

	_mpd_idiv_word(&q, &r, digits, MPD_RDIGITS);
	return (r == 0) ? q : q+1;
}

/* Number of digits in the exponent. Not defined for MPD_SSIZE_MIN. */
inline int
mpd_exp_digits(mpd_ssize_t exp)
{
	exp = (exp < 0) ? -exp : exp;
	return mpd_word_digits(exp);
}

/* Canonical */
ALWAYS_INLINE int
mpd_iscanonical(const mpd_t *dec UNUSED)
{
	return 1;
}

/* Finite */
ALWAYS_INLINE int
mpd_isfinite(const mpd_t *dec)
{
	return !(dec->flags & MPD_SPECIAL);
}

/* Infinite */
ALWAYS_INLINE int
mpd_isinfinite(const mpd_t *dec)
{
	return dec->flags & MPD_INF;
}

/* NaN */
ALWAYS_INLINE int
mpd_isnan(const mpd_t *dec)
{
	return dec->flags & (MPD_NAN|MPD_SNAN);
}

/* Negative */
ALWAYS_INLINE int
mpd_isnegative(const mpd_t *dec)
{
	return dec->flags & MPD_NEG;
}

/* Positive */
ALWAYS_INLINE int
mpd_ispositive(const mpd_t *dec)
{
	return !(dec->flags & MPD_NEG);
}

/* qNaN */
ALWAYS_INLINE int
mpd_isqnan(const mpd_t *dec)
{
	return dec->flags & MPD_NAN;
}

/* Signed */
ALWAYS_INLINE int
mpd_issigned(const mpd_t *dec)
{
	return dec->flags & MPD_NEG;
}

/* sNaN */
ALWAYS_INLINE int
mpd_issnan(const mpd_t *dec)
{
	return dec->flags & MPD_SNAN;
}

/* Special */
ALWAYS_INLINE int
mpd_isspecial(const mpd_t *dec)
{
	return dec->flags & MPD_SPECIAL;
}

/* Zero */
ALWAYS_INLINE int
mpd_iszero(const mpd_t *dec)
{
	return !mpd_isspecial(dec) && mpd_msword(dec) == 0;
}

/* Test for zero when specials have been ruled out already */
ALWAYS_INLINE int
mpd_iszerocoeff(const mpd_t *dec)
{
	return mpd_msword(dec) == 0;
}

/* Normal */
inline int
mpd_isnormal(const mpd_t *dec, const mpd_context_t *ctx)
{
	if (mpd_isspecial(dec)) return 0;
	if (mpd_iszerocoeff(dec)) return 0;

	return mpd_adjexp(dec) >= ctx->emin;
}

/* Subnormal */
inline int
mpd_issubnormal(const mpd_t *dec, const mpd_context_t *ctx)
{
	if (mpd_isspecial(dec)) return 0;
	if (mpd_iszerocoeff(dec)) return 0;

	return mpd_adjexp(dec) < ctx->emin;
}

/* Odd word */
ALWAYS_INLINE int
mpd_isoddword(mpd_uint_t word)
{
	return word & 1;
}

/* Odd coefficient */
ALWAYS_INLINE int
mpd_isoddcoeff(const mpd_t *dec)
{
	return mpd_isoddword(dec->data[0]);
}

/* 0 if dec is positive, 1 if dec is negative */
ALWAYS_INLINE uint8_t
mpd_sign(const mpd_t *dec)
{
	return dec->flags & MPD_NEG;
}

/* 1 if dec is positive, -1 if dec is negative */
ALWAYS_INLINE int
mpd_arith_sign(const mpd_t *dec)
{
	return 1 - 2 * mpd_isnegative(dec);
}

/* Radix */
ALWAYS_INLINE long
mpd_radix(void)
{
	return 10;
}

/* Dynamic decimal */
ALWAYS_INLINE int
mpd_isdynamic(mpd_t *dec)
{
	return !(dec->flags & MPD_STATIC);
}

/* Static decimal */
ALWAYS_INLINE int
mpd_isstatic(mpd_t *dec)
{
	return dec->flags & MPD_STATIC;
}

/* Data of decimal is dynamic */
ALWAYS_INLINE int
mpd_isdynamic_data(mpd_t *dec)
{
	return !(dec->flags & MPD_DATAFLAGS);
}

/* Data of decimal is static */
ALWAYS_INLINE int
mpd_isstatic_data(mpd_t *dec)
{
	return dec->flags & MPD_STATIC_DATA;
}

/* Data of decimal is shared */
ALWAYS_INLINE int
mpd_isshared_data(mpd_t *dec)
{
	return dec->flags & MPD_SHARED_DATA;
}

/* Data of decimal is const */
ALWAYS_INLINE int
mpd_isconst_data(mpd_t *dec)
{
	return dec->flags & MPD_CONST_DATA;
}


/******************************************************************************/
/*                         Inline memory handling                             */
/******************************************************************************/

/* Fill destination with zeros */
ALWAYS_INLINE void
mpd_uint_zero(mpd_uint_t *dest, mpd_size_t len)
{
	mpd_size_t i;

	for (i = 0; i < len; i++) {
		dest[i] = 0;
	}
}

/* Free a decimal */
ALWAYS_INLINE void
mpd_del(mpd_t *dec)
{
	if (mpd_isdynamic_data(dec)) {
		mpd_free(dec->data);
	}
	if (mpd_isdynamic(dec)) {
		mpd_free(dec);
	}
}

/*
 * Update the memory size for the coefficient. Existing data up to size is
 * left untouched.
 *
 * Error handling: When relloc fails, result->data will still be a valid pointer
 * to the old memory area of size result->len. If the requested size is less than
 * result->len, we can continue normally, so we treat the failure as a soft error.
 * If the requested size is greater than the old area, MPD_Malloc_error is
 * set and the result will be a NaN.
 */
ALWAYS_INLINE int
mpd_qresize(mpd_t *result, mpd_ssize_t size, uint32_t *status)
{
	assert(!mpd_isconst_data(result)); /* illegal operation for a const */
	assert(!mpd_isshared_data(result)); /* illegal operation for a shared */

	if (mpd_isstatic_data(result)) {
		if (size > result->alloc) {
			return mpd_switch_to_dyn(result, size, status);
		}
	}
	else if (size != result->alloc && size >= MPD_MINALLOC) {
		return mpd_realloc_dyn(result, size, status);
	}

	return 1;
}

/* Same as mpd_qresize, but the complete coefficient (including the old
 * memory area!) is initialized to zero. */
ALWAYS_INLINE int
mpd_qresize_zero(mpd_t *result, mpd_ssize_t size, uint32_t *status)
{
	assert(!mpd_isconst_data(result)); /* illegal operation for a const */
	assert(!mpd_isshared_data(result)); /* illegal operation for a shared */

	if (mpd_isstatic_data(result)) {
		if (size > result->alloc) {
			return mpd_switch_to_dyn_zero(result, size, status);
		}
	}
	else if (size != result->alloc && size >= MPD_MINALLOC) {
		if (!mpd_realloc_dyn(result, size, status)) {
			return 0;
		}
	}

	mpd_uint_zero(result->data, size);

	return 1;
}

/*
 * Reduce memory size for the coefficient to MPD_MINALLOC. In theory,
 * realloc may fail even when reducing the memory size. But in that case
 * the old memory area is always big enough, so checking for MPD_Malloc_error
 * is not imperative.
 */
ALWAYS_INLINE void
mpd_minalloc(mpd_t *result)
{
	assert(!mpd_isconst_data(result)); /* illegal operation for a const */
	assert(!mpd_isshared_data(result)); /* illegal operation for a shared */

	if (!mpd_isstatic_data(result) && result->alloc > MPD_MINALLOC) {
		uint8_t err = 0;
		result->data = mpd_realloc(result->data, MPD_MINALLOC,
		                           sizeof *result->data, &err);
		if (!err) {
			result->alloc = MPD_MINALLOC;
		}
	}
}

int
mpd_resize(mpd_t *result, mpd_ssize_t size, mpd_context_t *ctx)
{
	uint32_t status = 0;
	if (!mpd_qresize(result, size, &status)) {
		mpd_addstatus_raise(ctx, status);
		return 0;
	}
	return 1;
}

int
mpd_resize_zero(mpd_t *result, mpd_ssize_t size, mpd_context_t *ctx)
{
	uint32_t status = 0;
	if (!mpd_qresize_zero(result, size, &status)) {
		mpd_addstatus_raise(ctx, status);
		return 0;
	}
	return 1;
}


/******************************************************************************/
/*                       Set attributes of a decimal                          */
/******************************************************************************/

/* Set digits. result->len is assumed to be correct. */
inline void
mpd_setdigits(mpd_t *result)
{
	mpd_ssize_t wdigits = mpd_word_digits(mpd_msword(result));
	result->digits = wdigits + (result->len-1) * MPD_RDIGITS;
}

/* Set sign */
ALWAYS_INLINE void
mpd_set_sign(mpd_t *result, uint8_t sign)
{
	result->flags &= ~MPD_NEG;
	result->flags |= sign;
}

/* Copy sign from another decimal */
ALWAYS_INLINE void
mpd_signcpy(mpd_t *result, mpd_t *a)
{
	uint8_t sign = a->flags&MPD_NEG;

	result->flags &= ~MPD_NEG;
	result->flags |= sign;
}

/* Set infinity */
ALWAYS_INLINE void
mpd_set_infinity(mpd_t *result)
{
	result->flags &= ~MPD_SPECIAL;
	result->flags |= MPD_INF;
}

/* Set qNaN */
ALWAYS_INLINE void
mpd_set_qnan(mpd_t *result)
{
	result->flags &= ~MPD_SPECIAL;
	result->flags |= MPD_NAN;
}

/* Set sNaN */
ALWAYS_INLINE void
mpd_set_snan(mpd_t *result)
{
	result->flags &= ~MPD_SPECIAL;
	result->flags |= MPD_SNAN;
}

/* Set to negative */
ALWAYS_INLINE void
mpd_set_negative(mpd_t *result)
{
	result->flags |= MPD_NEG;
}

/* Set to positive */
ALWAYS_INLINE void
mpd_set_positive(mpd_t *result)
{
	result->flags &= ~MPD_NEG;
}

/* Set to dynamic */
ALWAYS_INLINE void
mpd_set_dynamic(mpd_t *result)
{
	result->flags &= ~MPD_STATIC;
}

/* Set to static */
ALWAYS_INLINE void
mpd_set_static(mpd_t *result)
{
	result->flags |= MPD_STATIC;
}

/* Set data to dynamic */
ALWAYS_INLINE void
mpd_set_dynamic_data(mpd_t *result)
{
	result->flags &= ~MPD_DATAFLAGS;
}

/* Set data to static */
ALWAYS_INLINE void
mpd_set_static_data(mpd_t *result)
{
	result->flags &= ~MPD_DATAFLAGS;
	result->flags |= MPD_STATIC_DATA;
}

/* Set data to shared */
ALWAYS_INLINE void
mpd_set_shared_data(mpd_t *result)
{
	result->flags &= ~MPD_DATAFLAGS;
	result->flags |= MPD_SHARED_DATA;
}

/* Set data to const */
ALWAYS_INLINE void
mpd_set_const_data(mpd_t *result)
{
	result->flags &= ~MPD_DATAFLAGS;
	result->flags |= MPD_CONST_DATA;
}

/* Clear flags, preserving memory attributes. */
ALWAYS_INLINE void
mpd_clear_flags(mpd_t *result)
{
	result->flags &= (MPD_STATIC|MPD_DATAFLAGS);
}

/* Set flags, preserving memory attributes. */
ALWAYS_INLINE void
mpd_set_flags(mpd_t *result, uint8_t flags)
{
	result->flags &= (MPD_STATIC|MPD_DATAFLAGS);
	result->flags |= flags;
}

/* Copy flags, preserving memory attributes of result. */
ALWAYS_INLINE void
mpd_copy_flags(mpd_t *result, const mpd_t *a)
{
	uint8_t aflags = a->flags;
	result->flags &= (MPD_STATIC|MPD_DATAFLAGS);
	result->flags |= (aflags & ~(MPD_STATIC|MPD_DATAFLAGS));
}

/* Make a work context */
static inline void
mpd_workcontext(mpd_context_t *workctx, const mpd_context_t *ctx)
{
	workctx->prec = ctx->prec;
	workctx->emax = ctx->emax;
	workctx->emin = ctx->emin;
	workctx->round = ctx->round;
	workctx->traps = 0;
	workctx->status= 0;
	workctx->newtrap= 0;
	workctx->clamp = ctx->clamp;
	workctx->allcr = ctx->allcr;
}


/******************************************************************************/
/*                  Getting and setting parts of decimals                     */
/******************************************************************************/

/* Flip the sign of a decimal */
static inline void
_mpd_negate(mpd_t *dec)
{
	dec->flags ^= MPD_NEG;
}

/* Set coefficient to zero */
void
mpd_zerocoeff(mpd_t *result)
{
	mpd_minalloc(result);
	result->digits = 1;
	result->len = 1;
	result->data[0] = 0;
}

/* Set the coefficient to all nines. */
void
mpd_qmaxcoeff(mpd_t *result, const mpd_context_t *ctx, uint32_t *status)
{
	mpd_ssize_t len, r;

	_mpd_idiv_word(&len, &r, ctx->prec, MPD_RDIGITS);
	len = (r == 0) ? len : len+1;

	if (!mpd_qresize(result, len, status)) {
		return;
	}

	result->len = len;
	result->digits = ctx->prec;

	--len;
	if (r > 0) {
		result->data[len--] = mpd_pow10[r]-1;
	}
	for (; len >= 0; --len) {
		result->data[len] = MPD_RADIX-1;
	}
}

/*
 * Cut off the most significant digits so that the rest fits in ctx->prec.
 * Cannot fail.
 */
static void
_mpd_cap(mpd_t *result, const mpd_context_t *ctx)
{
	uint32_t dummy;
	mpd_ssize_t len, r;

	if (result->len > 0 && result->digits > ctx->prec) {
		_mpd_idiv_word(&len, &r, ctx->prec, MPD_RDIGITS);
		len = (r == 0) ? len : len+1;

		if (r != 0) {
			result->data[len-1] %= mpd_pow10[r];
		}

		len = _mpd_real_size(result->data, len);
		/* resize to fewer words cannot fail */
		mpd_qresize(result, len, &dummy);
		result->len = len;
		mpd_setdigits(result);
	}
	if (mpd_iszero(result)) {
		_settriple(result, mpd_sign(result), 0, result->exp);
	}
}

/*
 * Cut off the most significant digits of a NaN payload so that the rest
 * fits in ctx->prec - ctx->clamp. Cannot fail.
 */
static void
_mpd_fix_nan(mpd_t *result, const mpd_context_t *ctx)
{
	uint32_t dummy;
	mpd_ssize_t prec;
	mpd_ssize_t len, r;

	prec = ctx->prec - ctx->clamp;
	if (result->len > 0 && result->digits > prec) {
		if (prec == 0) {
			mpd_minalloc(result);
			result->len = result->digits = 0;
		}
		else {
			_mpd_idiv_word(&len, &r, prec, MPD_RDIGITS);
			len = (r == 0) ? len : len+1;

			if (r != 0) {
				 result->data[len-1] %= mpd_pow10[r];
			}

			len = _mpd_real_size(result->data, len);
			/* resize to fewer words cannot fail */
			mpd_qresize(result, len, &dummy);
			result->len = len;
			mpd_setdigits(result);
			if (mpd_iszerocoeff(result)) {
				/* NaN0 is not a valid representation */
				result->len = result->digits = 0;
			}
		}
	}
}

/*
 * Get n most significant digits from a decimal, where 0 < n <= MPD_UINT_DIGITS.
 * Assumes MPD_UINT_DIGITS == MPD_RDIGITS+1, which is true for 32 and 64 bit
 * machines.
 *
 * The result of the operation will be in lo. If the operation is impossible,
 * hi will be nonzero. This is used to indicate an error.
 */
static inline void
_mpd_get_msdigits(mpd_uint_t *hi, mpd_uint_t *lo, const mpd_t *dec,
                  unsigned int n)
{
	mpd_uint_t r, tmp;

	assert(0 < n && n <= MPD_RDIGITS+1);

	_mpd_div_word(&tmp, &r, dec->digits, MPD_RDIGITS);
	r = (r == 0) ? MPD_RDIGITS : r; /* digits in the most significant word */

	*hi = 0;
	*lo = dec->data[dec->len-1];
	if (n <= r) {
		*lo /= mpd_pow10[r-n];
	}
	else if (dec->len > 1) {
		/* at this point 1 <= r < n <= MPD_RDIGITS+1 */
		_mpd_mul_words(hi, lo, *lo, mpd_pow10[n-r]);
		tmp = dec->data[dec->len-2] / mpd_pow10[MPD_RDIGITS-(n-r)];
		*lo = *lo + tmp;
		if (*lo < tmp) (*hi)++;
	}
}


/******************************************************************************/
/*                   Gathering information about a decimal                    */
/******************************************************************************/

/* The real size of the coefficient without leading zero words. */
static inline mpd_ssize_t
_mpd_real_size(mpd_uint_t *data, mpd_ssize_t size)
{
	while (size > 1 && data[size-1] == 0) {
		size--;
	}

	return size;
}

/* Return number of trailing zeros. No errors are possible. */
mpd_ssize_t
mpd_trail_zeros(const mpd_t *dec)
{
	mpd_uint_t word;
	mpd_ssize_t i, tz = 0;

	for (i=0; i < dec->len; ++i) {
		if (dec->data[i] != 0) {
			word = dec->data[i];
			tz = i * MPD_RDIGITS;
			while (word % 10 == 0) {
				word /= 10;
				tz++;
			}
			break;
		}
	}

	return tz;
}

/* Integer: Undefined for specials */
static int
_mpd_isint(const mpd_t *dec)
{
	mpd_ssize_t tz;

	if (mpd_iszerocoeff(dec)) {
		return 1;
	}

	tz = mpd_trail_zeros(dec);
	return (dec->exp + tz >= 0);
}

/* Integer */
int
mpd_isinteger(const mpd_t *dec)
{
	if (mpd_isspecial(dec)) {
		return 0;
	}
	return _mpd_isint(dec);
}

/* Word is a power of 10 */
static int
mpd_word_ispow10(mpd_uint_t word)
{
	int n;

	n = mpd_word_digits(word);
	if (word == mpd_pow10[n-1]) {
		return 1;
	}

	return 0;
}

/* Coefficient is a power of 10 */
static int
mpd_coeff_ispow10(const mpd_t *dec)
{
	if (mpd_word_ispow10(mpd_msword(dec))) {
		if (_mpd_isallzero(dec->data, dec->len-1)) {
			return 1;
		}
	}

	return 0;
}

/* All digits of a word are nines */
static int
mpd_word_isallnine(mpd_uint_t word)
{
	int n;

	n = mpd_word_digits(word);
	if (word == mpd_pow10[n]-1) {
		return 1;
	}

	return 0;
}

/* All digits of the coefficient are nines */
static int
mpd_coeff_isallnine(const mpd_t *dec)
{
	if (mpd_word_isallnine(mpd_msword(dec))) {
		if (_mpd_isallnine(dec->data, dec->len-1)) {
			return 1;
		}
	}

	return 0;
}

/* Odd decimal: Undefined for non-integers! */
int
mpd_isodd(const mpd_t *dec)
{
	mpd_uint_t q, r;
	assert(mpd_isinteger(dec));
	if (mpd_iszerocoeff(dec)) return 0;
	if (dec->exp < 0) {
		_mpd_div_word(&q, &r, -dec->exp, MPD_RDIGITS);
		q = dec->data[q] / mpd_pow10[r];
		return mpd_isoddword(q);
	}
	return dec->exp == 0 && mpd_isoddword(dec->data[0]);
}

/* Even: Undefined for non-integers! */
int
mpd_iseven(const mpd_t *dec)
{
	return !mpd_isodd(dec);
}

/******************************************************************************/
/*                      Getting and setting decimals                          */
/******************************************************************************/

/* Internal function: Set a static decimal from a triple, no error checking. */
static void
_ssettriple(mpd_t *result, uint8_t sign, mpd_uint_t a, mpd_ssize_t exp)
{
	mpd_set_flags(result, sign);
	result->exp = exp;
	_mpd_div_word(&result->data[1], &result->data[0], a, MPD_RADIX);
	result->len = (result->data[1] == 0) ? 1 : 2;
	mpd_setdigits(result);
}

/* Internal function: Set a decimal from a triple, no error checking. */
static void
_settriple(mpd_t *result, uint8_t sign, mpd_uint_t a, mpd_ssize_t exp)
{
	mpd_minalloc(result);
	mpd_set_flags(result, sign);
	result->exp = exp;
	_mpd_div_word(&result->data[1], &result->data[0], a, MPD_RADIX);
	result->len = (result->data[1] == 0) ? 1 : 2;
	mpd_setdigits(result);
}

/* Set a special number from a triple */
void
mpd_setspecial(mpd_t *result, uint8_t sign, uint8_t type)
{
	mpd_minalloc(result);
	result->flags &= ~(MPD_NEG|MPD_SPECIAL);
	result->flags |= (sign|type);
	result->exp = result->digits = result->len = 0;
}

/* Set result of NaN with an error status */
void
mpd_seterror(mpd_t *result, uint32_t flags, uint32_t *status)
{
	mpd_minalloc(result);
	mpd_set_qnan(result);
	mpd_set_positive(result);
	result->exp = result->digits = result->len = 0;
	*status |= flags;
}

/* quietly set a static decimal from an mpd_ssize_t */
void
mpd_qsset_ssize(mpd_t *result, mpd_ssize_t a, const mpd_context_t *ctx,
                uint32_t *status)
{
	mpd_uint_t u;
	uint8_t sign = MPD_POS;

	if (a < 0) {
		if (a == MPD_SSIZE_MIN) {
			u = (mpd_uint_t)MPD_SSIZE_MAX +
			    (-(MPD_SSIZE_MIN+MPD_SSIZE_MAX));
		}
		else {
			u = -a;
		}
		sign = MPD_NEG;
	}
	else {
		u = a;
	}
	_ssettriple(result, sign, u, 0);
	mpd_qfinalize(result, ctx, status);
}

/* quietly set a static decimal from an mpd_uint_t */
void
mpd_qsset_uint(mpd_t *result, mpd_uint_t a, const mpd_context_t *ctx,
               uint32_t *status)
{
	_ssettriple(result, MPD_POS, a, 0);
	mpd_qfinalize(result, ctx, status);
}

/* quietly set a static decimal from an int32_t */
void
mpd_qsset_i32(mpd_t *result, int32_t a, const mpd_context_t *ctx,
              uint32_t *status)
{
	mpd_qsset_ssize(result, a, ctx, status);
}

/* quietly set a static decimal from a uint32_t */
void
mpd_qsset_u32(mpd_t *result, uint32_t a, const mpd_context_t *ctx,
              uint32_t *status)
{
	mpd_qsset_uint(result, a, ctx, status);
}

#ifdef CONFIG_64
/* quietly set a static decimal from an int64_t */
void
mpd_qsset_i64(mpd_t *result, int64_t a, const mpd_context_t *ctx,
              uint32_t *status)
{
	mpd_qsset_ssize(result, a, ctx, status);
}

/* quietly set a static decimal from a uint64_t */
void
mpd_qsset_u64(mpd_t *result, uint64_t a, const mpd_context_t *ctx,
              uint32_t *status)
{
	mpd_qsset_uint(result, a, ctx, status);
}
#endif

/* quietly set a decimal from an mpd_ssize_t */
void
mpd_qset_ssize(mpd_t *result, mpd_ssize_t a, const mpd_context_t *ctx,
               uint32_t *status)
{
	mpd_minalloc(result);
	mpd_qsset_ssize(result, a, ctx, status);
}

/* quietly set a decimal from an mpd_uint_t */
void
mpd_qset_uint(mpd_t *result, mpd_uint_t a, const mpd_context_t *ctx,
              uint32_t *status)
{
	_settriple(result, MPD_POS, a, 0);
	mpd_qfinalize(result, ctx, status);
}

/* quietly set a decimal from an int32_t */
void
mpd_qset_i32(mpd_t *result, int32_t a, const mpd_context_t *ctx,
             uint32_t *status)
{
	mpd_qset_ssize(result, a, ctx, status);
}

/* quietly set a decimal from a uint32_t */
void
mpd_qset_u32(mpd_t *result, uint32_t a, const mpd_context_t *ctx,
             uint32_t *status)
{
	mpd_qset_uint(result, a, ctx, status);
}

#ifdef CONFIG_32
/* set a decimal from a uint64_t */
static void
_c32setu64(mpd_t *result, uint64_t u, uint8_t sign, uint32_t *status)
{
	mpd_uint_t w[3];
	uint64_t q;
	int i, len;

	len = 0;
	do {
		q = u / MPD_RADIX;
		w[len] = (mpd_uint_t)(u - q * MPD_RADIX);
		u = q; len++;
	} while (u != 0);

	if (!mpd_qresize(result, len, status)) {
		return;
	}
	for (i = 0; i < len; i++) {
		result->data[i] = w[i];
	}

	mpd_set_sign(result, sign);
	result->exp = 0;
	result->len = len;
	mpd_setdigits(result);
}

static void
_c32_qset_u64(mpd_t *result, uint64_t a, const mpd_context_t *ctx,
              uint32_t *status)
{
	_c32setu64(result, a, MPD_POS, status);
	mpd_qfinalize(result, ctx, status);
}

/* set a decimal from an int64_t */
static void
_c32_qset_i64(mpd_t *result, int64_t a, const mpd_context_t *ctx,
              uint32_t *status)
{
	uint64_t u;
	uint8_t sign = MPD_POS;

	if (a < 0) {
		if (a == INT64_MIN) {
			u = (uint64_t)INT64_MAX + (-(INT64_MIN+INT64_MAX));
		}
		else {
			u = -a;
		}
		sign = MPD_NEG;
	}
	else {
		u = a;
	}
	_c32setu64(result, u, sign, status);
	mpd_qfinalize(result, ctx, status);
}
#endif /* CONFIG_32 */

/* quietly set a decimal from an int64_t */
void
mpd_qset_i64(mpd_t *result, int64_t a, const mpd_context_t *ctx,
             uint32_t *status)
{
#ifdef CONFIG_64
	mpd_qset_ssize(result, a, ctx, status);
#else
	_c32_qset_i64(result, a, ctx, status);
#endif
}

/* quietly set a decimal from a uint64_t */
void
mpd_qset_u64(mpd_t *result, uint64_t a, const mpd_context_t *ctx,
             uint32_t *status)
{
#ifdef CONFIG_64
	mpd_qset_uint(result, a, ctx, status);
#else
	_c32_qset_u64(result, a, ctx, status);
#endif
}


/*
 * Quietly get an mpd_uint_t from a decimal. Assumes
 * MPD_UINT_DIGITS == MPD_RDIGITS+1, which is true for
 * 32 and 64 bit machines.
 *
 * If the operation is impossible, MPD_Invalid_operation is set.
 */
static mpd_uint_t
_mpd_qget_uint(int use_sign, const mpd_t *a, uint32_t *status)
{
	mpd_t tmp;
	mpd_uint_t tmp_data[2];
	mpd_uint_t lo, hi;

	if (mpd_isspecial(a)) {
		*status |= MPD_Invalid_operation;
		return MPD_UINT_MAX;
	}
	if (mpd_iszero(a)) {
		return 0;
	}
	if (use_sign && mpd_isnegative(a)) {
		*status |= MPD_Invalid_operation;
		return MPD_UINT_MAX;
	}

	if (a->digits+a->exp > MPD_RDIGITS+1) {
		*status |= MPD_Invalid_operation;
		return MPD_UINT_MAX;
	}

	if (a->exp < 0) {
		if (!_mpd_isint(a)) {
			*status |= MPD_Invalid_operation;
			return MPD_UINT_MAX;
		}
		/* At this point a->digits+a->exp <= MPD_RDIGITS+1,
		 * so the shift fits. */
		tmp.data = tmp_data;
		tmp.flags = MPD_STATIC|MPD_CONST_DATA;
		mpd_qsshiftr(&tmp, a, -a->exp);
		tmp.exp = 0;
		a = &tmp;
	}

	_mpd_get_msdigits(&hi, &lo, a, MPD_RDIGITS+1);
	if (hi) {
		*status |= MPD_Invalid_operation;
		return MPD_UINT_MAX;
	}

	if (a->exp > 0) {
		_mpd_mul_words(&hi, &lo, lo, mpd_pow10[a->exp]);
		if (hi) {
			*status |= MPD_Invalid_operation;
			return MPD_UINT_MAX;
		}
	}

	return lo;
}

/*
 * Sets Invalid_operation for:
 *   - specials
 *   - negative numbers (except negative zero)
 *   - non-integers
 *   - overflow
 */
mpd_uint_t
mpd_qget_uint(const mpd_t *a, uint32_t *status)
{
	return _mpd_qget_uint(1, a, status);
}

/* Same as above, but gets the absolute value, i.e. the sign is ignored. */
mpd_uint_t
mpd_qabs_uint(const mpd_t *a, uint32_t *status)
{
	return _mpd_qget_uint(0, a, status);
}

/* quietly get an mpd_ssize_t from a decimal */
mpd_ssize_t
mpd_qget_ssize(const mpd_t *a, uint32_t *status)
{
	mpd_uint_t u;
	int isneg;

	u = mpd_qabs_uint(a, status);
	if (*status&MPD_Invalid_operation) {
		return MPD_SSIZE_MAX;
	}

	isneg = mpd_isnegative(a);
	if (u <= MPD_SSIZE_MAX) {
		return isneg ? -((mpd_ssize_t)u) : (mpd_ssize_t)u;
	}
	else if (isneg && u-1 == MPD_SSIZE_MAX) {
		return MPD_SSIZE_MIN;
	}

	*status |= MPD_Invalid_operation;
	return MPD_SSIZE_MAX;
}

#ifdef CONFIG_64
/* quietly get a uint64_t from a decimal */
uint64_t
mpd_qget_u64(const mpd_t *a, uint32_t *status)
{
	return mpd_qget_uint(a, status);
}

/* quietly get an int64_t from a decimal */
int64_t
mpd_qget_i64(const mpd_t *a, uint32_t *status)
{
	return mpd_qget_ssize(a, status);
}
#else
/* quietly get a uint32_t from a decimal */
uint32_t
mpd_qget_u32(const mpd_t *a, uint32_t *status)
{
	return mpd_qget_uint(a, status);
}

/* quietly get an int32_t from a decimal */
int32_t
mpd_qget_i32(const mpd_t *a, uint32_t *status)
{
	return mpd_qget_ssize(a, status);
}
#endif


/******************************************************************************/
/*         Filtering input of functions, finalizing output of functions       */
/******************************************************************************/

/*
 * Check if the operand is NaN, copy to result and return 1 if this is
 * the case. Copying can fail since NaNs are allowed to have a payload that
 * does not fit in MPD_MINALLOC.
 */
int
mpd_qcheck_nan(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
               uint32_t *status)
{
	if (mpd_isnan(a)) {
		*status |= mpd_issnan(a) ? MPD_Invalid_operation : 0;
		mpd_qcopy(result, a, status);
		mpd_set_qnan(result);
		_mpd_fix_nan(result, ctx);
		return 1;
	}
	return 0;
}

/*
 * Check if either operand is NaN, copy to result and return 1 if this
 * is the case. Copying can fail since NaNs are allowed to have a payload
 * that does not fit in MPD_MINALLOC.
 */
int
mpd_qcheck_nans(mpd_t *result, const mpd_t *a, const mpd_t *b,
                const mpd_context_t *ctx, uint32_t *status)
{
	if ((a->flags|b->flags)&(MPD_NAN|MPD_SNAN)) {
		const mpd_t *choice = b;
		if (mpd_issnan(a)) {
			choice = a;
			*status |= MPD_Invalid_operation;
		}
		else if (mpd_issnan(b)) {
			*status |= MPD_Invalid_operation;
		}
		else if (mpd_isqnan(a)) {
			choice = a;
		}
		mpd_qcopy(result, choice, status);
		mpd_set_qnan(result);
		_mpd_fix_nan(result, ctx);
		return 1;
	}
	return 0;
}

/*
 * Check if one of the operands is NaN, copy to result and return 1 if this
 * is the case. Copying can fail since NaNs are allowed to have a payload
 * that does not fit in MPD_MINALLOC.
 */
static int
mpd_qcheck_3nans(mpd_t *result, const mpd_t *a, const mpd_t *b, const mpd_t *c,
                 const mpd_context_t *ctx, uint32_t *status)
{
	if ((a->flags|b->flags|c->flags)&(MPD_NAN|MPD_SNAN)) {
		const mpd_t *choice = c;
		if (mpd_issnan(a)) {
			choice = a;
			*status |= MPD_Invalid_operation;
		}
		else if (mpd_issnan(b)) {
			choice = b;
			*status |= MPD_Invalid_operation;
		}
		else if (mpd_issnan(c)) {
			*status |= MPD_Invalid_operation;
		}
		else if (mpd_isqnan(a)) {
			choice = a;
		}
		else if (mpd_isqnan(b)) {
			choice = b;
		}
		mpd_qcopy(result, choice, status);
		mpd_set_qnan(result);
		_mpd_fix_nan(result, ctx);
		return 1;
	}
	return 0;
}

/* Check if rounding digit 'rnd' leads to an increment. */
static inline int
_mpd_rnd_incr(const mpd_t *dec, mpd_uint_t rnd, const mpd_context_t *ctx)
{
	int ld;

	switch (ctx->round) {
	case MPD_ROUND_DOWN: case MPD_ROUND_TRUNC:
		return 0;
	case MPD_ROUND_HALF_UP:
		return (rnd >= 5);
	case MPD_ROUND_HALF_EVEN:
		return (rnd > 5) || ((rnd == 5) && mpd_isoddcoeff(dec));
	case MPD_ROUND_CEILING:
		return !(rnd == 0 || mpd_isnegative(dec));
	case MPD_ROUND_FLOOR:
		return !(rnd == 0 || mpd_ispositive(dec));
	case MPD_ROUND_HALF_DOWN:
		return (rnd > 5);
	case MPD_ROUND_UP:
		return !(rnd == 0);
	case MPD_ROUND_05UP:
		ld = (int)mpd_lsd(dec->data[0]);
		return (!(rnd == 0) && (ld == 0 || ld == 5));
	default:
		/* Without a valid context, further results will be undefined. */
		return 0; /* GCOV_NOT_REACHED */
	}
}

/*
 * Apply rounding to a decimal that has been right-shifted into a full
 * precision decimal. If an increment leads to an overflow of the precision,
 * adjust the coefficient and the exponent and check the new exponent for
 * overflow.
 */
static inline void
_mpd_apply_round(mpd_t *dec, mpd_uint_t rnd, const mpd_context_t *ctx,
                 uint32_t *status)
{
	if (_mpd_rnd_incr(dec, rnd, ctx)) {
		/* We have a number with exactly ctx->prec digits. The increment
		 * can only lead to an overflow if the decimal is all nines. In
		 * that case, the result is a power of ten with prec+1 digits.
		 *
		 * If the precision is a multiple of MPD_RDIGITS, this situation is
		 * detected by _mpd_baseincr returning a carry.
		 * If the precision is not a multiple of MPD_RDIGITS, we have to
		 * check if the result has one digit too many.
		 */
		mpd_uint_t carry = _mpd_baseincr(dec->data, dec->len);
		if (carry) {
			dec->data[dec->len-1] = mpd_pow10[MPD_RDIGITS-1];
			dec->exp += 1;
			_mpd_check_exp(dec, ctx, status);
			return;
		}
		mpd_setdigits(dec);
		if (dec->digits > ctx->prec) {
			mpd_qshiftr_inplace(dec, 1);
			dec->exp += 1;
			dec->digits = ctx->prec;
			_mpd_check_exp(dec, ctx, status);
		}
	}
}

/*
 * Apply rounding to a decimal. Allow overflow of the precision.
 */
static inline void
_mpd_apply_round_excess(mpd_t *dec, mpd_uint_t rnd, const mpd_context_t *ctx,
                        uint32_t *status)
{
	if (_mpd_rnd_incr(dec, rnd, ctx)) {
		mpd_uint_t carry = _mpd_baseincr(dec->data, dec->len);
		if (carry) {
			if (!mpd_qresize(dec, dec->len+1, status)) {
				return;
			}
			dec->data[dec->len] = 1;
			dec->len += 1;
		}
		mpd_setdigits(dec);
	}
}

/*
 * Apply rounding to a decimal that has been right-shifted into a decimal
 * with full precision or less. Return failure if an increment would
 * overflow the precision.
 */
static inline int
_mpd_apply_round_fit(mpd_t *dec, mpd_uint_t rnd, const mpd_context_t *ctx,
                     uint32_t *status)
{
	if (_mpd_rnd_incr(dec, rnd, ctx)) {
		mpd_uint_t carry = _mpd_baseincr(dec->data, dec->len);
		if (carry) {
			if (!mpd_qresize(dec, dec->len+1, status)) {
				return 0;
			}
			dec->data[dec->len] = 1;
			dec->len += 1;
		}
		mpd_setdigits(dec);
		if (dec->digits > ctx->prec) {
			mpd_seterror(dec, MPD_Invalid_operation, status);
			return 0;
		}
	}
	return 1;
}

/* Check a normal number for overflow, underflow, clamping. */
static inline void
_mpd_check_exp(mpd_t *dec, const mpd_context_t *ctx, uint32_t *status)
{
	mpd_ssize_t adjexp, etiny, shift;
	int rnd;

	adjexp = mpd_adjexp(dec);
	if (adjexp > ctx->emax) {

		if (mpd_iszerocoeff(dec)) {
			dec->exp = ctx->emax;
			if (ctx->clamp) {
				dec->exp -= (ctx->prec-1);
			}
			mpd_zerocoeff(dec);
			*status |= MPD_Clamped;
			return;
		}

		switch (ctx->round) {
		case MPD_ROUND_HALF_UP: case MPD_ROUND_HALF_EVEN:
		case MPD_ROUND_HALF_DOWN: case MPD_ROUND_UP:
		case MPD_ROUND_TRUNC:
			mpd_setspecial(dec, mpd_sign(dec), MPD_INF);
			break;
		case MPD_ROUND_DOWN: case MPD_ROUND_05UP:
			mpd_qmaxcoeff(dec, ctx, status);
			dec->exp = ctx->emax - ctx->prec + 1;
			break;
		case MPD_ROUND_CEILING:
			if (mpd_isnegative(dec)) {
				mpd_qmaxcoeff(dec, ctx, status);
				dec->exp = ctx->emax - ctx->prec + 1;
			}
			else {
				mpd_setspecial(dec, MPD_POS, MPD_INF);
			}
			break;
		case MPD_ROUND_FLOOR:
			if (mpd_ispositive(dec)) {
				mpd_qmaxcoeff(dec, ctx, status);
				dec->exp = ctx->emax - ctx->prec + 1;
			}
			else {
				mpd_setspecial(dec, MPD_NEG, MPD_INF);
			}
			break;
		default: /* debug */
			abort(); /* GCOV_NOT_REACHED */
		}

		*status |= MPD_Overflow|MPD_Inexact|MPD_Rounded;

	} /* fold down */
	else if (ctx->clamp && dec->exp > mpd_etop(ctx)) {
		shift = dec->exp - mpd_etop(ctx);
		if (!mpd_qshiftl(dec, dec, shift, status)) {
			return;
		}
		dec->exp -= shift;
		*status |= MPD_Clamped;
		if (!mpd_iszerocoeff(dec) && adjexp < ctx->emin) {
			*status |= MPD_Subnormal;
		}
	}
	else if (adjexp < ctx->emin) {

		etiny = mpd_etiny(ctx);

		if (mpd_iszerocoeff(dec)) {
			if (dec->exp < etiny) {
				dec->exp = etiny;
				mpd_zerocoeff(dec);
				*status |= MPD_Clamped;
			}
			return;
		}

		*status |= MPD_Subnormal;
		if (dec->exp < etiny) {
			/* At this point adjexp=exp+digits-1 < emin and exp < etiny=emin-prec+1,
			 * so shift=emin-prec+1-exp > digits-prec, so digits-shift < prec.
			 * [ACL2 proof: checkexp-1] */
			shift = etiny - dec->exp;
			rnd = (int)mpd_qshiftr_inplace(dec, shift);
			dec->exp = etiny;
			/* We always have a spare digit in case of an increment. */
			_mpd_apply_round_excess(dec, rnd, ctx, status);
			*status |= MPD_Rounded;
			if (rnd) {
				*status |= (MPD_Inexact|MPD_Underflow);
				if (mpd_iszerocoeff(dec)) {
					mpd_zerocoeff(dec);
					*status |= MPD_Clamped;
				}
			}
		}
	}
}

/* Transcendental functions do not always set Underflow reliably,
 * since they only use as much precision as is necessary for correct
 * rounding. If a result like 1.0000000000e-101 is finalized, there
 * is no rounding digit that would trigger Underflow. But we can
 * assume Inexact, so a short check suffices. */
static inline void
mpd_check_underflow(mpd_t *dec, const mpd_context_t *ctx, uint32_t *status)
{
	if (mpd_adjexp(dec) < ctx->emin && !mpd_iszero(dec) &&
	    dec->exp < mpd_etiny(ctx)) {
		*status |= MPD_Underflow;
	}
}

/* Check if a normal number must be rounded after the exponent has been checked. */
static inline void
_mpd_check_round(mpd_t *dec, const mpd_context_t *ctx, uint32_t *status)
{
	mpd_uint_t rnd;
	mpd_ssize_t shift;

	/* must handle specials: _mpd_check_exp() can produce infinities or NaNs */
	if (mpd_isspecial(dec)) {
		return;
	}

	if (dec->digits > ctx->prec) {
		shift = dec->digits - ctx->prec;
		rnd = mpd_qshiftr_inplace(dec, shift);
		dec->exp += shift;
		_mpd_apply_round(dec, rnd, ctx, status);
		*status |= MPD_Rounded;
		if (rnd) {
			*status |= MPD_Inexact;
		}
	}
}

/* Finalize all operations. */
void
mpd_qfinalize(mpd_t *result, const mpd_context_t *ctx, uint32_t *status)
{
	if (mpd_isspecial(result)) {
		if (mpd_isnan(result)) {
			_mpd_fix_nan(result, ctx);
		}
		return;
	}

	_mpd_check_exp(result, ctx, status);
	_mpd_check_round(result, ctx, status);
}


/******************************************************************************/
/*                                 Copying                                    */
/******************************************************************************/

/* Internal function: Copy a decimal, share data with src: USE WITH CARE! */
static inline void
_mpd_copy_shared(mpd_t *dest, const mpd_t *src)
{
	dest->flags = src->flags;
	dest->exp = src->exp;
	dest->digits = src->digits;
	dest->len = src->len;
	dest->alloc = src->alloc;
	dest->data = src->data;

	mpd_set_shared_data(dest);
}

/*
 * Copy a decimal. In case of an error, status is set to MPD_Malloc_error.
 */
int
mpd_qcopy(mpd_t *result, const mpd_t *a, uint32_t *status)
{
	if (result == a) return 1;

	if (!mpd_qresize(result, a->len, status)) {
		return 0;
	}

	mpd_copy_flags(result, a);
	result->exp = a->exp;
	result->digits = a->digits;
	result->len = a->len;
	memcpy(result->data, a->data, a->len * (sizeof *result->data));

	return 1;
}

/*
 * Copy to a decimal with a static buffer. The caller has to make sure that
 * the buffer is big enough. Cannot fail.
 */
static void
mpd_qcopy_static(mpd_t *result, const mpd_t *a)
{
	if (result == a) return;

	memcpy(result->data, a->data, a->len * (sizeof *result->data));

	mpd_copy_flags(result, a);
	result->exp = a->exp;
	result->digits = a->digits;
	result->len = a->len;
}

/*
 * Return a newly allocated copy of the operand. In case of an error,
 * status is set to MPD_Malloc_error and the return value is NULL.
 */
mpd_t *
mpd_qncopy(const mpd_t *a)
{
	mpd_t *result;

	if ((result = mpd_qnew_size(a->len)) == NULL) {
		return NULL;
	}
	memcpy(result->data, a->data, a->len * (sizeof *result->data));
	mpd_copy_flags(result, a);
	result->exp = a->exp;
	result->digits = a->digits;
	result->len = a->len;

	return result;
}

/*
 * Copy a decimal and set the sign to positive. In case of an error, the
 * status is set to MPD_Malloc_error.
 */
int
mpd_qcopy_abs(mpd_t *result, const mpd_t *a, uint32_t *status)
{
	if (!mpd_qcopy(result, a, status)) {
		return 0;
	}
	mpd_set_positive(result);
	return 1;
}

/*
 * Copy a decimal and negate the sign. In case of an error, the
 * status is set to MPD_Malloc_error.
 */
int
mpd_qcopy_negate(mpd_t *result, const mpd_t *a, uint32_t *status)
{
	if (!mpd_qcopy(result, a, status)) {
		return 0;
	}
	_mpd_negate(result);
	return 1;
}

/*
 * Copy a decimal, setting the sign of the first operand to the sign of the
 * second operand. In case of an error, the status is set to MPD_Malloc_error.
 */
int
mpd_qcopy_sign(mpd_t *result, const mpd_t *a, const mpd_t *b, uint32_t *status)
{
	uint8_t sign_b = mpd_sign(b); /* result may equal b! */

	if (!mpd_qcopy(result, a, status)) {
		return 0;
	}
	mpd_set_sign(result, sign_b);
	return 1;
}


/******************************************************************************/
/*                                Comparisons                                 */
/******************************************************************************/

/*
 * For all functions that compare two operands and return an int the usual
 * convention applies to the return value:
 *
 * -1 if op1 < op2
 *  0 if op1 == op2
 *  1 if op1 > op2
 *
 *  INT_MAX for error
 */


/* Convenience macro. If a and b are not equal, return from the calling
 * function with the correct comparison value. */
#define CMP_EQUAL_OR_RETURN(a, b)  \
        if (a != b) {              \
                if (a < b) {       \
                        return -1; \
                }                  \
                return 1;          \
        }

/*
 * Compare the data of big and small. This function does the equivalent
 * of first shifting small to the left and then comparing the data of
 * big and small, except that no allocation for the left shift is needed.
 */
static int
_mpd_basecmp(mpd_uint_t *big, mpd_uint_t *small, mpd_size_t n, mpd_size_t m,
             mpd_size_t shift)
{
#if defined(__GNUC__) && !defined(__INTEL_COMPILER)
	/* spurious uninitialized warnings */
	mpd_uint_t l=l, lprev=lprev, h=h;
#else
	mpd_uint_t l, lprev, h;
#endif
	mpd_uint_t q, r;
	mpd_uint_t ph, x;

	assert(m > 0 && n >= m && shift > 0);

	_mpd_div_word(&q, &r, (mpd_uint_t)shift, MPD_RDIGITS);

	if (r != 0) {

		ph = mpd_pow10[r];

		--m; --n;
		_mpd_divmod_pow10(&h, &lprev, small[m--], MPD_RDIGITS-r);
		if (h != 0) {
			CMP_EQUAL_OR_RETURN(big[n], h)
			--n;
		}
		for (; m != MPD_SIZE_MAX; m--,n--) {
			_mpd_divmod_pow10(&h, &l, small[m], MPD_RDIGITS-r);
			x = ph * lprev + h;
			CMP_EQUAL_OR_RETURN(big[n], x)
			lprev = l;
		}
		x = ph * lprev;
		CMP_EQUAL_OR_RETURN(big[q], x)
	}
	else {
		while (--m != MPD_SIZE_MAX) {
			CMP_EQUAL_OR_RETURN(big[m+q], small[m])
		}
	}

	return !_mpd_isallzero(big, q);
}

/* Compare two decimals with the same adjusted exponent. */
static int
_mpd_cmp_same_adjexp(const mpd_t *a, const mpd_t *b)
{
	mpd_ssize_t shift, i;

	if (a->exp != b->exp) {
		/* Cannot wrap: a->exp + a->digits = b->exp + b->digits, so
		 * a->exp - b->exp = b->digits - a->digits. */
		shift = a->exp - b->exp;
		if (shift > 0) {
			return -1 * _mpd_basecmp(b->data, a->data, b->len, a->len, shift);
		}
		else {
			return _mpd_basecmp(a->data, b->data, a->len, b->len, -shift);
		}
	}

	/*
	 * At this point adjexp(a) == adjexp(b) and a->exp == b->exp,
	 * so a->digits == b->digits, therefore a->len == b->len.
	 */
	for (i = a->len-1; i >= 0; --i) {
		CMP_EQUAL_OR_RETURN(a->data[i], b->data[i])
	}

	return 0;
}

/* Compare two numerical values. */
static int
_mpd_cmp(const mpd_t *a, const mpd_t *b)
{
	mpd_ssize_t adjexp_a, adjexp_b;

	/* equal pointers */
	if (a == b) {
		return 0;
	}

	/* infinities */
	if (mpd_isinfinite(a)) {
		if (mpd_isinfinite(b)) {
			return mpd_isnegative(b) - mpd_isnegative(a);
		}
		return mpd_arith_sign(a);
	}
	if (mpd_isinfinite(b)) {
		return -mpd_arith_sign(b);
	}

	/* zeros */
	if (mpd_iszerocoeff(a)) {
		if (mpd_iszerocoeff(b)) {
			return 0;
		}
		return -mpd_arith_sign(b);
	}
	if (mpd_iszerocoeff(b)) {
		return mpd_arith_sign(a);
	}

	/* different signs */
	if (mpd_sign(a) != mpd_sign(b)) {
		return mpd_sign(b) - mpd_sign(a);
	}

	/* different adjusted exponents */
	adjexp_a = mpd_adjexp(a);
	adjexp_b = mpd_adjexp(b);
	if (adjexp_a != adjexp_b) {
		if (adjexp_a < adjexp_b) {
			return -1 * mpd_arith_sign(a);
		}
		return mpd_arith_sign(a);
	}

	/* same adjusted exponents */
	return _mpd_cmp_same_adjexp(a, b) * mpd_arith_sign(a);
}

/* Compare the absolutes of two numerical values. */
static int
_mpd_cmp_abs(const mpd_t *a, const mpd_t *b)
{
	mpd_ssize_t adjexp_a, adjexp_b;

	/* equal pointers */
	if (a == b) {
		return 0;
	}

	/* infinities */
	if (mpd_isinfinite(a)) {
		if (mpd_isinfinite(b)) {
			return 0;
		}
		return 1;
	}
	if (mpd_isinfinite(b)) {
		return -1;
	}

	/* zeros */
	if (mpd_iszerocoeff(a)) {
		if (mpd_iszerocoeff(b)) {
			return 0;
		}
		return -1;
	}
	if (mpd_iszerocoeff(b)) {
		return 1;
	}

	/* different adjusted exponents */
	adjexp_a = mpd_adjexp(a);
	adjexp_b = mpd_adjexp(b);
	if (adjexp_a != adjexp_b) {
		if (adjexp_a < adjexp_b) {
			return -1;
		}
		return 1;
	}

	/* same adjusted exponents */
	return _mpd_cmp_same_adjexp(a, b);
}

/* Compare two values and return an integer result. */
int
mpd_qcmp(const mpd_t *a, const mpd_t *b, uint32_t *status)
{
	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_isnan(a) || mpd_isnan(b)) {
			*status |= MPD_Invalid_operation;
			return INT_MAX;
		}
	}

	return _mpd_cmp(a, b);
}

/*
 * Compare a and b, convert the the usual integer result to a decimal and
 * store it in 'result'. For convenience, the integer result of the comparison
 * is returned. Comparisons involving NaNs return NaN/INT_MAX.
 */
int
mpd_qcompare(mpd_t *result, const mpd_t *a, const mpd_t *b,
             const mpd_context_t *ctx, uint32_t *status)
{
	int c;

	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(result, a, b, ctx, status)) {
			return INT_MAX;
		}
	}

	c = _mpd_cmp(a, b);
	_settriple(result, (c < 0), (c != 0), 0);
	return c;
}

/* Same as mpd_compare(), but signal for all NaNs, i.e. also for quiet NaNs. */
int
mpd_qcompare_signal(mpd_t *result, const mpd_t *a, const mpd_t *b,
                    const mpd_context_t *ctx, uint32_t *status)
{
	int c;

	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(result, a, b, ctx, status)) {
			*status |= MPD_Invalid_operation;
			return INT_MAX;
		}
	}

	c = _mpd_cmp(a, b);
	_settriple(result, (c < 0), (c != 0), 0);
	return c;
}

/* Compare the operands using a total order. */
int
mpd_cmp_total(const mpd_t *a, const mpd_t *b)
{
	mpd_t aa, bb;
	int nan_a, nan_b;
	int c;

	if (mpd_sign(a) != mpd_sign(b)) {
		return mpd_sign(b) - mpd_sign(a);
	}


	if (mpd_isnan(a)) {
		c = 1;
		if (mpd_isnan(b)) {
			nan_a = (mpd_isqnan(a)) ? 1 : 0;
			nan_b = (mpd_isqnan(b)) ? 1 : 0;
			if (nan_b == nan_a) {
				if (a->len > 0 && b->len > 0) {
					_mpd_copy_shared(&aa, a);
					_mpd_copy_shared(&bb, b);
					aa.exp = bb.exp = 0;
					/* compare payload */
					c = _mpd_cmp_abs(&aa, &bb);
				}
				else {
					c = (a->len > 0) - (b->len > 0);
				}
			}
			else {
				c = nan_a - nan_b;
			}
		}
	}
	else if (mpd_isnan(b)) {
		c = -1;
	}
	else {
		c = _mpd_cmp_abs(a, b);
		if (c == 0 && a->exp != b->exp) {
			c = (a->exp < b->exp) ? -1 : 1;
		}
	}

	return c * mpd_arith_sign(a);
}

/*
 * Compare a and b according to a total order, convert the usual integer result
 * to a decimal and store it in 'result'. For convenience, the integer result
 * of the comparison is returned.
 */
int
mpd_compare_total(mpd_t *result, const mpd_t *a, const mpd_t *b)
{
	int c;

	c = mpd_cmp_total(a, b);
	_settriple(result, (c < 0), (c != 0), 0);
	return c;
}

/* Compare the magnitude of the operands using a total order. */
int
mpd_cmp_total_mag(const mpd_t *a, const mpd_t *b)
{
	mpd_t aa, bb;

	_mpd_copy_shared(&aa, a);
	_mpd_copy_shared(&bb, b);

	mpd_set_positive(&aa);
	mpd_set_positive(&bb);

	return mpd_cmp_total(&aa, &bb);
}

/*
 * Compare the magnitude of a and b according to a total order, convert the
 * the usual integer result to a decimal and store it in 'result'.
 * For convenience, the integer result of the comparison is returned.
 */
int
mpd_compare_total_mag(mpd_t *result, const mpd_t *a, const mpd_t *b)
{
	int c;

	c = mpd_cmp_total_mag(a, b);
	_settriple(result, (c < 0), (c != 0), 0);
	return c;
}

/* Determine an ordering for operands that are numerically equal. */
static inline int
_mpd_cmp_numequal(const mpd_t *a, const mpd_t *b)
{
	int sign_a, sign_b;
	int c;

	sign_a = mpd_sign(a);
	sign_b = mpd_sign(b);
	if (sign_a != sign_b) {
		c = sign_b - sign_a;
	}
	else {
		c = (a->exp < b->exp) ? -1 : 1;
		c *= mpd_arith_sign(a);
	}

	return c;
}


/******************************************************************************/
/*                         Shifting the coefficient                           */
/******************************************************************************/

/*
 * Shift the coefficient of the operand to the left, no check for specials.
 * Both operands may be the same pointer. If the result length has to be
 * increased, mpd_qresize() might fail with MPD_Malloc_error.
 */
int
mpd_qshiftl(mpd_t *result, const mpd_t *a, mpd_ssize_t n, uint32_t *status)
{
	mpd_ssize_t size;

	assert(n >= 0);

	if (mpd_iszerocoeff(a) || n == 0) {
		return mpd_qcopy(result, a, status);
	}

	size = mpd_digits_to_size(a->digits+n);
	if (!mpd_qresize(result, size, status)) {
		return 0; /* result is NaN */
	}

	_mpd_baseshiftl(result->data, a->data, size, a->len, n);

	mpd_copy_flags(result, a);
	result->len = size;
	result->exp = a->exp;
	result->digits = a->digits+n;

	return 1;
}

/* Determine the rounding indicator if all digits of the coefficient are shifted
 * out of the picture. */
static mpd_uint_t
_mpd_get_rnd(const mpd_uint_t *data, mpd_ssize_t len, int use_msd)
{
	mpd_uint_t rnd = 0, rest = 0, word;

	word = data[len-1];
	/* special treatment for the most significant digit if shift == digits */
	if (use_msd) {
		_mpd_divmod_pow10(&rnd, &rest, word, mpd_word_digits(word)-1);
		if (len > 1 && rest == 0) {
			 rest = !_mpd_isallzero(data, len-1);
		}
	}
	else {
		rest = !_mpd_isallzero(data, len);
	}

	return (rnd == 0 || rnd == 5) ? rnd + !!rest : rnd;
}

/*
 * Same as mpd_qshiftr(), but 'result' is a static array. It is the
 * caller's responsibility to make sure that the array is big enough.
 * The function cannot fail.
 */
mpd_uint_t
mpd_qsshiftr(mpd_t *result, const mpd_t *a, mpd_ssize_t n)
{
	mpd_uint_t rnd;
	mpd_ssize_t size;

	assert(n >= 0);

	if (mpd_iszerocoeff(a) || n == 0) {
		mpd_qcopy_static(result, a);
		return 0;
	}

	if (n >= a->digits) {
		rnd = _mpd_get_rnd(a->data, a->len, (n==a->digits));
		mpd_zerocoeff(result);
		result->digits = 1;
		size = 1;
	}
	else {
		result->digits = a->digits-n;
		size = mpd_digits_to_size(result->digits);
		rnd = _mpd_baseshiftr(result->data, a->data, a->len, n);
	}

	mpd_copy_flags(result, a);
	result->exp = a->exp;
	result->len = size;

	return rnd;
}

/*
 * Inplace shift of the coefficient to the right, no check for specials.
 * Returns the rounding indicator for mpd_rnd_incr().
 * The function cannot fail.
 */
mpd_uint_t
mpd_qshiftr_inplace(mpd_t *result, mpd_ssize_t n)
{
	uint32_t dummy;
	mpd_uint_t rnd;
	mpd_ssize_t size;

	assert(n >= 0);

	if (mpd_iszerocoeff(result) || n == 0) {
		return 0;
	}

	if (n >= result->digits) {
		rnd = _mpd_get_rnd(result->data, result->len, (n==result->digits));
		mpd_zerocoeff(result);
		result->digits = 1;
		size = 1;
	}
	else {
		rnd = _mpd_baseshiftr(result->data, result->data, result->len, n);
		result->digits -= n;
		size = mpd_digits_to_size(result->digits);
		/* reducing the size cannot fail */
		mpd_qresize(result, size, &dummy);
	}

	result->len = size;

	return rnd;
}

/*
 * Shift the coefficient of the operand to the right, no check for specials.
 * Both operands may be the same pointer. Returns the rounding indicator to
 * be used by mpd_rnd_incr(). If the result length has to be increased,
 * mpd_qcopy() or mpd_qresize() might fail with MPD_Malloc_error. In those
 * cases, MPD_UINT_MAX is returned.
 */
mpd_uint_t
mpd_qshiftr(mpd_t *result, const mpd_t *a, mpd_ssize_t n, uint32_t *status)
{
	mpd_uint_t rnd;
	mpd_ssize_t size;

	assert(n >= 0);

	if (mpd_iszerocoeff(a) || n == 0) {
		if (!mpd_qcopy(result, a, status)) {
			return MPD_UINT_MAX;
		}
		return 0;
	}

	if (n >= a->digits) {
		rnd = _mpd_get_rnd(a->data, a->len, (n==a->digits));
		mpd_zerocoeff(result);
		result->digits = 1;
		size = 1;
	}
	else {
		result->digits = a->digits-n;
		size = mpd_digits_to_size(result->digits);
		if (result == a) {
			rnd = _mpd_baseshiftr(result->data, a->data, a->len, n);
			/* reducing the size cannot fail */
			mpd_qresize(result, size, status);
		}
		else {
			if (!mpd_qresize(result, size, status)) {
				return MPD_UINT_MAX;
			}
			rnd = _mpd_baseshiftr(result->data, a->data, a->len, n);
		}
	}

	mpd_copy_flags(result, a);
	result->exp = a->exp;
	result->len = size;

	return rnd;
}


/******************************************************************************/
/*                         Miscellaneous operations                           */
/******************************************************************************/

/* Logical And */
void
mpd_qand(mpd_t *result, const mpd_t *a, const mpd_t *b,
         const mpd_context_t *ctx, uint32_t *status)
{
	const mpd_t *big = a, *small = b;
	mpd_uint_t x, y, z, xbit, ybit;
	int k, mswdigits;
	mpd_ssize_t i;

	if (mpd_isspecial(a) || mpd_isspecial(b) ||
	    mpd_isnegative(a) || mpd_isnegative(b) ||
	    a->exp != 0 || b->exp != 0) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	if (b->digits > a->digits) {
		big = b;
		small = a;
	}
	if (!mpd_qresize(result, big->len, status)) {
		return;
	}


	/* full words */
	for (i = 0; i < small->len-1; i++) {
		x = small->data[i];
		y = big->data[i];
		z = 0;
		for (k = 0; k < MPD_RDIGITS; k++) {
			xbit = x % 10;
			x /= 10;
			ybit = y % 10;
			y /= 10;
			if (xbit > 1 || ybit > 1) {
				goto invalid_operation;
			}
			z += (xbit&ybit) ? mpd_pow10[k] : 0;
		}
		result->data[i] = z;
	}
	/* most significant word of small */
	x = small->data[i];
	y = big->data[i];
	z = 0;
	mswdigits = mpd_word_digits(x);
	for (k = 0; k < mswdigits; k++) {
		xbit = x % 10;
		x /= 10;
		ybit = y % 10;
		y /= 10;
		if (xbit > 1 || ybit > 1) {
			goto invalid_operation;
		}
		z += (xbit&ybit) ? mpd_pow10[k] : 0;
	}
	result->data[i++] = z;

	/* scan the rest of y for digit > 1 */
	for (; k < MPD_RDIGITS; k++) {
		ybit = y % 10;
		y /= 10;
		if (ybit > 1) {
			goto invalid_operation;
		}
	}
	/* scan the rest of big for digit > 1 */
	for (; i < big->len; i++) {
		y = big->data[i];
		for (k = 0; k < MPD_RDIGITS; k++) {
			ybit = y % 10;
			y /= 10;
			if (ybit > 1) {
				goto invalid_operation;
			}
		}
	}

	mpd_clear_flags(result);
	result->exp = 0;
	result->len = _mpd_real_size(result->data, small->len);
	mpd_qresize(result, result->len, status);
	mpd_setdigits(result);
	_mpd_cap(result, ctx);
	return;

invalid_operation:
	mpd_seterror(result, MPD_Invalid_operation, status);
}

/* Class of an operand. Returns a pointer to the constant name. */
const char *
mpd_class(const mpd_t *a, const mpd_context_t *ctx)
{
	if (mpd_isnan(a)) {
		if (mpd_isqnan(a))
			return "NaN";
		else
			return "sNaN";
	}
	else if (mpd_ispositive(a)) {
		if (mpd_isinfinite(a))
			return "+Infinity";
		else if (mpd_iszero(a))
			return "+Zero";
		else if (mpd_isnormal(a, ctx))
			return "+Normal";
		else
			return "+Subnormal";
	}
	else {
		if (mpd_isinfinite(a))
			return "-Infinity";
		else if (mpd_iszero(a))
			return "-Zero";
		else if (mpd_isnormal(a, ctx))
			return "-Normal";
		else
			return "-Subnormal";
	}
}

/* Logical Xor */
void
mpd_qinvert(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
            uint32_t *status)
{
	mpd_uint_t x, z, xbit;
	mpd_ssize_t i, digits, len;
	mpd_ssize_t q, r;
	int k;

	if (mpd_isspecial(a) || mpd_isnegative(a) || a->exp != 0) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}

	digits = (a->digits < ctx->prec) ? ctx->prec : a->digits;
	_mpd_idiv_word(&q, &r, digits, MPD_RDIGITS);
	len = (r == 0) ? q : q+1;
	if (!mpd_qresize(result, len, status)) {
		return;
	}

	for (i = 0; i < len; i++) {
		x = (i < a->len) ?  a->data[i] : 0;
		z = 0;
		for (k = 0; k < MPD_RDIGITS; k++) {
			xbit = x % 10;
			x /= 10;
			if (xbit > 1) {
				goto invalid_operation;
			}
			z += !xbit ? mpd_pow10[k] : 0;
		}
		result->data[i] = z;
	}

	mpd_clear_flags(result);
	result->exp = 0;
	result->len = _mpd_real_size(result->data, len);
	mpd_qresize(result, result->len, status);
	mpd_setdigits(result);
	_mpd_cap(result, ctx);
	return;

invalid_operation:
	mpd_seterror(result, MPD_Invalid_operation, status);
}

/* Exponent of the magnitude of the most significant digit of the operand. */
void
mpd_qlogb(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
          uint32_t *status)
{
	if (mpd_isspecial(a)) {
		if (mpd_qcheck_nan(result, a, ctx, status)) {
			return;
		}
		mpd_setspecial(result, MPD_POS, MPD_INF);
	}
	else if (mpd_iszerocoeff(a)) {
		mpd_setspecial(result, MPD_NEG, MPD_INF);
		*status |= MPD_Division_by_zero;
	}
	else {
		mpd_qset_ssize(result, mpd_adjexp(a), ctx, status);
	}
}

/* Logical Or */
void
mpd_qor(mpd_t *result, const mpd_t *a, const mpd_t *b,
        const mpd_context_t *ctx, uint32_t *status)
{
	const mpd_t *big = a, *small = b;
	mpd_uint_t x, y, z, xbit, ybit;
	int k, mswdigits;
	mpd_ssize_t i;

	if (mpd_isspecial(a) || mpd_isspecial(b) ||
	    mpd_isnegative(a) || mpd_isnegative(b) ||
	    a->exp != 0 || b->exp != 0) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	if (b->digits > a->digits) {
		big = b;
		small = a;
	}
	if (!mpd_qresize(result, big->len, status)) {
		return;
	}


	/* full words */
	for (i = 0; i < small->len-1; i++) {
		x = small->data[i];
		y = big->data[i];
		z = 0;
		for (k = 0; k < MPD_RDIGITS; k++) {
			xbit = x % 10;
			x /= 10;
			ybit = y % 10;
			y /= 10;
			if (xbit > 1 || ybit > 1) {
				goto invalid_operation;
			}
			z += (xbit|ybit) ? mpd_pow10[k] : 0;
		}
		result->data[i] = z;
	}
	/* most significant word of small */
	x = small->data[i];
	y = big->data[i];
	z = 0;
	mswdigits = mpd_word_digits(x);
	for (k = 0; k < mswdigits; k++) {
		xbit = x % 10;
		x /= 10;
		ybit = y % 10;
		y /= 10;
		if (xbit > 1 || ybit > 1) {
			goto invalid_operation;
		}
		z += (xbit|ybit) ? mpd_pow10[k] : 0;
	}

	/* scan and copy the rest of y for digit > 1 */
	for (; k < MPD_RDIGITS; k++) {
		ybit = y % 10;
		y /= 10;
		if (ybit > 1) {
			goto invalid_operation;
		}
		z += ybit*mpd_pow10[k];
	}
	result->data[i++] = z;
	/* scan and copy the rest of big for digit > 1 */
	for (; i < big->len; i++) {
		y = big->data[i];
		for (k = 0; k < MPD_RDIGITS; k++) {
			ybit = y % 10;
			y /= 10;
			if (ybit > 1) {
				goto invalid_operation;
			}
		}
		result->data[i] = big->data[i];
	}

	mpd_clear_flags(result);
	result->exp = 0;
	result->len = _mpd_real_size(result->data, big->len);
	mpd_qresize(result, result->len, status);
	mpd_setdigits(result);
	_mpd_cap(result, ctx);
	return;

invalid_operation:
	mpd_seterror(result, MPD_Invalid_operation, status);
}

/*
 * Rotate the coefficient of a by b->data digits. b must be an integer with
 * exponent 0.
 */
void
mpd_qrotate(mpd_t *result, const mpd_t *a, const mpd_t *b,
            const mpd_context_t *ctx, uint32_t *status)
{
	uint32_t workstatus = 0;
	MPD_NEW_STATIC(tmp,0,0,0,0);
	MPD_NEW_STATIC(big,0,0,0,0);
	MPD_NEW_STATIC(small,0,0,0,0);
	mpd_ssize_t n, lshift, rshift;

	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(result, a, b, ctx, status)) {
			return;
		}
	}
	if (b->exp != 0 || mpd_isinfinite(b)) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}

	n = mpd_qget_ssize(b, &workstatus);
	if (workstatus&MPD_Invalid_operation) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	if (n > ctx->prec || n < -ctx->prec) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	if (mpd_isinfinite(a)) {
		mpd_qcopy(result, a, status);
		return;
	}

	if (n >= 0) {
		lshift = n;
		rshift = ctx->prec-n;
	}
	else {
		lshift = ctx->prec+n;
		rshift = -n;
	}

	if (a->digits > ctx->prec) {
		if (!mpd_qcopy(&tmp, a, status)) {
			mpd_seterror(result, MPD_Malloc_error, status);
			goto finish;
		}
		_mpd_cap(&tmp, ctx);
		a = &tmp;
	}

	if (!mpd_qshiftl(&big, a, lshift, status)) {
		mpd_seterror(result, MPD_Malloc_error, status);
		goto finish;
	}
	_mpd_cap(&big, ctx);

	if (mpd_qshiftr(&small, a, rshift, status) == MPD_UINT_MAX) {
		mpd_seterror(result, MPD_Malloc_error, status);
		goto finish;
	}
	_mpd_qadd(result, &big, &small, ctx, status);


finish:
	mpd_del(&tmp);
	mpd_del(&big);
	mpd_del(&small);
}

/*
 * b must be an integer with exponent 0 and in the range +-2*(emax + prec).
 * XXX: In my opinion +-(2*emax + prec) would be more sensible.
 * The result is a with the value of b added to its exponent.
 */
void
mpd_qscaleb(mpd_t *result, const mpd_t *a, const mpd_t *b,
            const mpd_context_t *ctx, uint32_t *status)
{
	uint32_t workstatus = 0;
	mpd_uint_t n, maxjump;
	int64_t exp;

	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(result, a, b, ctx, status)) {
			return;
		}
	}
	if (b->exp != 0 || mpd_isinfinite(b)) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}

	n = mpd_qabs_uint(b, &workstatus);
	/* the spec demands this */
	maxjump = 2 * (ctx->emax + ctx->prec);

	if (n > maxjump || workstatus&MPD_Invalid_operation) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	if (mpd_isinfinite(a)) {
		mpd_qcopy(result, a, status);
		return;
	}

	exp = a->exp + (int64_t)n * mpd_arith_sign(b);
	exp = (exp > MPD_EXP_INF) ? MPD_EXP_INF : exp;
	exp = (exp < MPD_EXP_CLAMP) ? MPD_EXP_CLAMP : exp;
	mpd_qcopy(result, a, status);
	result->exp = (mpd_ssize_t)exp;

	mpd_qfinalize(result, ctx, status);
}

/*
 * Shift the coefficient by n digits, positive n is a left shift. In the case
 * of a left shift, the result is decapitated to fit the context precision. If
 * you don't want that, use mpd_shiftl().
 */
void
mpd_qshiftn(mpd_t *result, const mpd_t *a, mpd_ssize_t n, const mpd_context_t *ctx,
            uint32_t *status)
{
	if (mpd_isspecial(a)) {
		if (mpd_qcheck_nan(result, a, ctx, status)) {
			return;
		}
		mpd_qcopy(result, a, status);
		return;
	}

	if (n >= 0 && n <= ctx->prec) {
		mpd_qshiftl(result, a, n, status);
		_mpd_cap(result, ctx);
	}
	else if (n < 0 && n >= -ctx->prec) {
		if (!mpd_qcopy(result, a, status)) {
			return;
		}
		_mpd_cap(result, ctx);
		mpd_qshiftr_inplace(result, -n);
	}
	else {
		mpd_seterror(result, MPD_Invalid_operation, status);
	}
}

/*
 * Same as mpd_shiftn(), but the shift is specified by the decimal b, which
 * must be an integer with a zero exponent. Infinities remain infinities.
 */
void
mpd_qshift(mpd_t *result, const mpd_t *a, const mpd_t *b, const mpd_context_t *ctx,
           uint32_t *status)
{
	uint32_t workstatus = 0;
	mpd_ssize_t n;

	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(result, a, b, ctx, status)) {
			return;
		}
	}
	if (b->exp != 0 || mpd_isinfinite(b)) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}

	n = mpd_qget_ssize(b, &workstatus);
	if (workstatus&MPD_Invalid_operation) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	if (n > ctx->prec || n < -ctx->prec) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	if (mpd_isinfinite(a)) {
		mpd_qcopy(result, a, status);
		return;
	}

	if (n >= 0) {
		mpd_qshiftl(result, a, n, status);
		_mpd_cap(result, ctx);
	}
	else {
		if (!mpd_qcopy(result, a, status)) {
			return;
		}
		_mpd_cap(result, ctx);
		mpd_qshiftr_inplace(result, -n);
	}
}

/* Logical Xor */
void
mpd_qxor(mpd_t *result, const mpd_t *a, const mpd_t *b,
        const mpd_context_t *ctx, uint32_t *status)
{
	const mpd_t *big = a, *small = b;
	mpd_uint_t x, y, z, xbit, ybit;
	int k, mswdigits;
	mpd_ssize_t i;

	if (mpd_isspecial(a) || mpd_isspecial(b) ||
	    mpd_isnegative(a) || mpd_isnegative(b) ||
	    a->exp != 0 || b->exp != 0) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	if (b->digits > a->digits) {
		big = b;
		small = a;
	}
	if (!mpd_qresize(result, big->len, status)) {
		return;
	}


	/* full words */
	for (i = 0; i < small->len-1; i++) {
		x = small->data[i];
		y = big->data[i];
		z = 0;
		for (k = 0; k < MPD_RDIGITS; k++) {
			xbit = x % 10;
			x /= 10;
			ybit = y % 10;
			y /= 10;
			if (xbit > 1 || ybit > 1) {
				goto invalid_operation;
			}
			z += (xbit^ybit) ? mpd_pow10[k] : 0;
		}
		result->data[i] = z;
	}
	/* most significant word of small */
	x = small->data[i];
	y = big->data[i];
	z = 0;
	mswdigits = mpd_word_digits(x);
	for (k = 0; k < mswdigits; k++) {
		xbit = x % 10;
		x /= 10;
		ybit = y % 10;
		y /= 10;
		if (xbit > 1 || ybit > 1) {
			goto invalid_operation;
		}
		z += (xbit^ybit) ? mpd_pow10[k] : 0;
	}

	/* scan and copy the rest of y for digit > 1 */
	for (; k < MPD_RDIGITS; k++) {
		ybit = y % 10;
		y /= 10;
		if (ybit > 1) {
			goto invalid_operation;
		}
		z += ybit*mpd_pow10[k];
	}
	result->data[i++] = z;
	/* scan and copy the rest of big for digit > 1 */
	for (; i < big->len; i++) {
		y = big->data[i];
		for (k = 0; k < MPD_RDIGITS; k++) {
			ybit = y % 10;
			y /= 10;
			if (ybit > 1) {
				goto invalid_operation;
			}
		}
		result->data[i] = big->data[i];
	}

	mpd_clear_flags(result);
	result->exp = 0;
	result->len = _mpd_real_size(result->data, big->len);
	mpd_qresize(result, result->len, status);
	mpd_setdigits(result);
	_mpd_cap(result, ctx);
	return;

invalid_operation:
	mpd_seterror(result, MPD_Invalid_operation, status);
}


/******************************************************************************/
/*                         Arithmetic operations                              */
/******************************************************************************/

/*
 * The absolute value of a. If a is negative, the result is the same
 * as the result of the minus operation. Otherwise, the result is the
 * result of the plus operation.
 */
void
mpd_qabs(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
         uint32_t *status)
{
	if (mpd_isspecial(a)) {
		if (mpd_qcheck_nan(result, a, ctx, status)) {
			return;
		}
	}

	if (mpd_isnegative(a)) {
		mpd_qminus(result, a, ctx, status);
	}
	else {
		mpd_qplus(result, a, ctx, status);
	}

	mpd_qfinalize(result, ctx, status);
}

static inline void
_mpd_ptrswap(mpd_t **a, mpd_t **b)
{
	mpd_t *t = *a;
	*a = *b;
	*b = t;
}

/* Add or subtract infinities. */
static void
_mpd_qaddsub_inf(mpd_t *result, const mpd_t *a, const mpd_t *b, uint8_t sign_b,
                 uint32_t *status)
{
	if (mpd_isinfinite(a)) {
		if (mpd_sign(a) != sign_b && mpd_isinfinite(b)) {
			mpd_seterror(result, MPD_Invalid_operation, status);
		}
		else {
			mpd_setspecial(result, mpd_sign(a), MPD_INF);
		}
		return;
	}
	assert(mpd_isinfinite(b));
	mpd_setspecial(result, sign_b, MPD_INF);
}

/* Add or subtract non-special numbers. */
static void
_mpd_qaddsub(mpd_t *result, const mpd_t *a, const mpd_t *b, uint8_t sign_b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_t *big, *small;
	MPD_NEW_STATIC(big_aligned,0,0,0,0);
	MPD_NEW_CONST(tiny,0,0,0,1,1,1);
	mpd_uint_t carry;
	mpd_ssize_t newsize, shift;
	mpd_ssize_t exp, i;
	int swap = 0;


	/* compare exponents */
	big = (mpd_t *)a; small = (mpd_t *)b;
	if (big->exp != small->exp) {
		if (small->exp > big->exp) {
			_mpd_ptrswap(&big, &small);
			swap++;
		}
		if (!mpd_iszerocoeff(big)) {
			/* Test for adjexp(small) + big->digits < adjexp(big), if big-digits > prec
			 * Test for adjexp(small) + prec + 1    < adjexp(big), if big-digits <= prec
			 * If true, the magnitudes of the numbers are so far apart that one can as
			 * well add or subtract 1*10**big->exp. */
			exp = big->exp - 1;
			exp += (big->digits > ctx->prec) ? 0 : big->digits-ctx->prec-1;
			if (mpd_adjexp(small) < exp) {
				mpd_copy_flags(&tiny, small);
				tiny.exp = exp;
				tiny.digits = 1;
				tiny.len = 1;
				tiny.data[0] = mpd_iszerocoeff(small) ? 0 : 1;
				small = &tiny;
			}
			/* this cannot wrap: the difference is positive and <= maxprec+1 */
			shift = big->exp - small->exp;
			if (!mpd_qshiftl(&big_aligned, big, shift, status)) {
				mpd_seterror(result, MPD_Malloc_error, status);
				goto finish;
			}
			big = &big_aligned;
		}
	}
	result->exp = small->exp;


	/* compare length of coefficients */
	if (big->len < small->len) {
		_mpd_ptrswap(&big, &small);
		swap++;
	}

	newsize = big->len;
	if (!mpd_qresize(result, newsize, status)) {
		goto finish;
	}

	if (mpd_sign(a) == sign_b) {

		carry = _mpd_baseadd(result->data, big->data, small->data,
		                     big->len, small->len);

		if (carry) {
			newsize = big->len + 1;
			if (!mpd_qresize(result, newsize, status)) {
				goto finish;
			}
			result->data[newsize-1] = carry;
		}

		result->len = newsize;
		mpd_set_flags(result, sign_b);
	}
	else {
		if (big->len == small->len) {
			for (i=big->len-1; i >= 0; --i) {
				if (big->data[i] != small->data[i]) {
					if (big->data[i] < small->data[i]) {
						_mpd_ptrswap(&big, &small);
						swap++;
					}
					break;
				}
			}
		}

		_mpd_basesub(result->data, big->data, small->data,
		             big->len, small->len);
		newsize = _mpd_real_size(result->data, big->len);
		/* resize to smaller cannot fail */
		(void)mpd_qresize(result, newsize, status);

		result->len = newsize;
		sign_b = (swap & 1) ? sign_b : mpd_sign(a);
		mpd_set_flags(result, sign_b);

		if (mpd_iszerocoeff(result)) {
			mpd_set_positive(result);
			if (ctx->round == MPD_ROUND_FLOOR) {
				mpd_set_negative(result);
			}
		}
	}

	mpd_setdigits(result);

finish:
	mpd_del(&big_aligned);
}

/* Add a and b. No specials, no finalizing. */
static void
_mpd_qadd(mpd_t *result, const mpd_t *a, const mpd_t *b,
          const mpd_context_t *ctx, uint32_t *status)
{
	_mpd_qaddsub(result, a, b, mpd_sign(b), ctx, status);
}

/* Subtract b from a. No specials, no finalizing. */
static void
_mpd_qsub(mpd_t *result, const mpd_t *a, const mpd_t *b,
         const mpd_context_t *ctx, uint32_t *status)
{
	 _mpd_qaddsub(result, a, b, !mpd_sign(b), ctx, status);
}

/* Add a and b. */
void
mpd_qadd(mpd_t *result, const mpd_t *a, const mpd_t *b,
         const mpd_context_t *ctx, uint32_t *status)
{
	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(result, a, b, ctx, status)) {
			return;
		}
		_mpd_qaddsub_inf(result, a, b, mpd_sign(b), status);
		return;
	}

	_mpd_qaddsub(result, a, b, mpd_sign(b), ctx, status);
	mpd_qfinalize(result, ctx, status);
}

/* Subtract b from a. */
void
mpd_qsub(mpd_t *result, const mpd_t *a, const mpd_t *b,
         const mpd_context_t *ctx, uint32_t *status)
{
	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(result, a, b, ctx, status)) {
			return;
		}
		_mpd_qaddsub_inf(result, a, b, !mpd_sign(b), status);
		return;
	}

	_mpd_qaddsub(result, a, b, !mpd_sign(b), ctx, status);
	mpd_qfinalize(result, ctx, status);
}

/* Add decimal and mpd_ssize_t. */
void
mpd_qadd_ssize(mpd_t *result, const mpd_t *a, mpd_ssize_t b,
               const mpd_context_t *ctx, uint32_t *status)
{
	mpd_t bb;
	mpd_uint_t bdata[2];

	bb.data = bdata;
	bb.flags = MPD_STATIC|MPD_CONST_DATA;

	mpd_qsset_ssize(&bb, b, ctx, status);
	mpd_qadd(result, a, &bb, ctx, status);
}

/* Add decimal and mpd_uint_t. */
void
mpd_qadd_uint(mpd_t *result, const mpd_t *a, mpd_uint_t b,
              const mpd_context_t *ctx, uint32_t *status)
{
	mpd_t bb;
	mpd_uint_t bdata[2];

	bb.data = bdata;
	bb.flags = MPD_STATIC|MPD_CONST_DATA;

	mpd_qsset_uint(&bb, b, ctx, status);
	mpd_qadd(result, a, &bb, ctx, status);
}

/* Subtract mpd_ssize_t from decimal. */
void
mpd_qsub_ssize(mpd_t *result, const mpd_t *a, mpd_ssize_t b,
               const mpd_context_t *ctx, uint32_t *status)
{
	mpd_t bb;
	mpd_uint_t bdata[2];

	bb.data = bdata;
	bb.flags = MPD_STATIC|MPD_CONST_DATA;

	mpd_qsset_ssize(&bb, b, ctx, status);
	mpd_qsub(result, a, &bb, ctx, status);
}

/* Subtract mpd_uint_t from decimal. */
void
mpd_qsub_uint(mpd_t *result, const mpd_t *a, mpd_uint_t b,
              const mpd_context_t *ctx, uint32_t *status)
{
	mpd_t bb;
	mpd_uint_t bdata[2];

	bb.data = bdata;
	bb.flags = MPD_STATIC|MPD_CONST_DATA;

	mpd_qsset_uint(&bb, b, ctx, status);
	mpd_qsub(result, a, &bb, ctx, status);
}

/* Add decimal and int32_t. */
void
mpd_qadd_i32(mpd_t *result, const mpd_t *a, int32_t b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_qadd_ssize(result, a, b, ctx, status);
}

/* Add decimal and uint32_t. */
void
mpd_qadd_u32(mpd_t *result, const mpd_t *a, uint32_t b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_qadd_uint(result, a, b, ctx, status);
}

#ifdef CONFIG_64
/* Add decimal and int64_t. */
void
mpd_qadd_i64(mpd_t *result, const mpd_t *a, int64_t b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_qadd_ssize(result, a, b, ctx, status);
}

/* Add decimal and uint64_t. */
void
mpd_qadd_u64(mpd_t *result, const mpd_t *a, uint64_t b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_qadd_uint(result, a, b, ctx, status);
}
#endif

/* Subtract int32_t from decimal. */
void
mpd_qsub_i32(mpd_t *result, const mpd_t *a, int32_t b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_qsub_ssize(result, a, b, ctx, status);
}

/* Subtract uint32_t from decimal. */
void
mpd_qsub_u32(mpd_t *result, const mpd_t *a, uint32_t b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_qsub_uint(result, a, b, ctx, status);
}

#ifdef CONFIG_64
/* Subtract int64_t from decimal. */
void
mpd_qsub_i64(mpd_t *result, const mpd_t *a, int64_t b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_qsub_ssize(result, a, b, ctx, status);
}

/* Subtract uint64_t from decimal. */
void
mpd_qsub_u64(mpd_t *result, const mpd_t *a, uint64_t b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_qsub_uint(result, a, b, ctx, status);
}
#endif


/* Divide infinities. */
static void
_mpd_qdiv_inf(mpd_t *result, const mpd_t *a, const mpd_t *b,
              const mpd_context_t *ctx, uint32_t *status)
{
	if (mpd_isinfinite(a)) {
		if (mpd_isinfinite(b)) {
			mpd_seterror(result, MPD_Invalid_operation, status);
			return;
		}
		mpd_setspecial(result, mpd_sign(a)^mpd_sign(b), MPD_INF);
		return;
	}
	assert(mpd_isinfinite(b));
	_settriple(result, mpd_sign(a)^mpd_sign(b), 0, mpd_etiny(ctx));
	*status |= MPD_Clamped;
}

enum {NO_IDEAL_EXP, SET_IDEAL_EXP};
/* Divide a by b. */
static void
_mpd_qdiv(int action, mpd_t *q, const mpd_t *a, const mpd_t *b,
          const mpd_context_t *ctx, uint32_t *status)
{
	MPD_NEW_STATIC(aligned,0,0,0,0);
	mpd_uint_t ld;
	mpd_ssize_t shift, exp, tz;
	mpd_ssize_t newsize;
	mpd_ssize_t ideal_exp;
	mpd_uint_t rem;
	uint8_t sign_a = mpd_sign(a);
	uint8_t sign_b = mpd_sign(b);


	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(q, a, b, ctx, status)) {
			return;
		}
		_mpd_qdiv_inf(q, a, b, ctx, status);
		return;
	}
	if (mpd_iszerocoeff(b)) {
		if (mpd_iszerocoeff(a)) {
			mpd_seterror(q, MPD_Division_undefined, status);
		}
		else {
			mpd_setspecial(q, sign_a^sign_b, MPD_INF);
			*status |= MPD_Division_by_zero;
		}
		return;
	}
	if (mpd_iszerocoeff(a)) {
		exp = a->exp - b->exp;
		_settriple(q, sign_a^sign_b, 0, exp);
		mpd_qfinalize(q, ctx, status);
		return;
	}

	shift = (b->digits - a->digits) + ctx->prec + 1;
	ideal_exp = a->exp - b->exp;
	exp = ideal_exp - shift;
	if (shift > 0) {
		if (!mpd_qshiftl(&aligned, a, shift, status)) {
			mpd_seterror(q, MPD_Malloc_error, status);
			goto finish;
		}
		a = &aligned;
	}
	else if (shift < 0) {
		shift = -shift;
		if (!mpd_qshiftl(&aligned, b, shift, status)) {
			mpd_seterror(q, MPD_Malloc_error, status);
			goto finish;
		}
		b = &aligned;
	}


	newsize = a->len - b->len + 1;
	if ((q != b && q != a) || (q == b && newsize > b->len)) {
		if (!mpd_qresize(q, newsize, status)) {
			mpd_seterror(q, MPD_Malloc_error, status);
			goto finish;
		}
	}


	if (b->len == 1) {
		rem = _mpd_shortdiv(q->data, a->data, a->len, b->data[0]);
	}
	else if (a->len < 2*MPD_NEWTONDIV_CUTOFF &&
	         b->len < MPD_NEWTONDIV_CUTOFF) {
		int ret = _mpd_basedivmod(q->data, NULL, a->data, b->data,
		                          a->len, b->len);
		if (ret < 0) {
			mpd_seterror(q, MPD_Malloc_error, status);
			goto finish;
		}
		rem = ret;
	}
	else {
		MPD_NEW_STATIC(r,0,0,0,0);
		_mpd_qbarrett_divmod(q, &r, a, b, status);
		if (mpd_isspecial(q) || mpd_isspecial(&r)) {
			mpd_del(&r);
			goto finish;
		}
		rem = !mpd_iszerocoeff(&r);
		mpd_del(&r);
		newsize = q->len;
	}

	newsize = _mpd_real_size(q->data, newsize);
	/* resize to smaller cannot fail */
	mpd_qresize(q, newsize, status);
	q->len = newsize;
	mpd_setdigits(q);

	shift = ideal_exp - exp;
	if (rem) {
		ld = mpd_lsd(q->data[0]);
		if (ld == 0 || ld == 5) {
			q->data[0] += 1;
		}
	}
	else if (action == SET_IDEAL_EXP && shift > 0) {
		tz = mpd_trail_zeros(q);
		shift = (tz > shift) ? shift : tz;
		mpd_qshiftr_inplace(q, shift);
		exp += shift;
	}

	mpd_set_flags(q, sign_a^sign_b);
	q->exp = exp;


finish:
	mpd_del(&aligned);
	mpd_qfinalize(q, ctx, status);
}

/* Divide a by b. */
void
mpd_qdiv(mpd_t *q, const mpd_t *a, const mpd_t *b,
         const mpd_context_t *ctx, uint32_t *status)
{
	_mpd_qdiv(SET_IDEAL_EXP, q, a, b, ctx, status);
}

/* Internal function, used with the actions specified in the enum below. */
static void
_mpd_qdivmod(mpd_t *q, mpd_t *r, const mpd_t *a, const mpd_t *b,
	     const mpd_context_t *ctx, uint32_t *status)
{
	MPD_NEW_STATIC(aligned,0,0,0,0);
	mpd_ssize_t qsize, rsize;
	mpd_ssize_t ideal_exp, expdiff, shift;
	uint8_t sign_a = mpd_sign(a);
	uint8_t sign_ab = mpd_sign(a)^mpd_sign(b);


	ideal_exp = (a->exp > b->exp) ?  b->exp : a->exp;
	if (mpd_iszerocoeff(a)) {
		if (!mpd_qcopy(r, a, status)) {
			goto nanresult; /* GCOV_NOT_REACHED */
		}
		r->exp = ideal_exp;
		_settriple(q, sign_ab, 0, 0);
		return;
	}

	expdiff = mpd_adjexp(a) - mpd_adjexp(b);
	if (expdiff < 0) {
		if (a->exp > b->exp) {
			/* positive and less than b->digits - a->digits */
			shift = a->exp - b->exp;
			if (!mpd_qshiftl(r, a, shift, status)) {
				goto nanresult;
			}
			r->exp = ideal_exp;
		}
		else {
			if (!mpd_qcopy(r, a, status)) {
				goto nanresult;
			}
		}
		_settriple(q, sign_ab, 0, 0);
		return;
	}
	if (expdiff > ctx->prec) {
		*status |= MPD_Division_impossible;
		goto nanresult;
	}


	/*
	 * At this point we have:
	 *   (1) 0 <= a->exp + a->digits - b->exp - b->digits <= prec
	 *   (2) a->exp - b->exp >= b->digits - a->digits
	 *   (3) a->exp - b->exp <= prec + b->digits - a->digits
	 */
	if (a->exp != b->exp) {
		shift = a->exp - b->exp;
		if (shift > 0) {
			/* by (3), after the shift a->digits <= prec + b->digits */
			if (!mpd_qshiftl(&aligned, a, shift, status)) {
				goto nanresult;
			}
			a = &aligned;
		}
		else  {
			shift = -shift;
			/* by (2), after the shift b->digits <= a->digits */
			if (!mpd_qshiftl(&aligned, b, shift, status)) {
				goto nanresult;
			}
			b = &aligned;
		}
	}


	qsize = a->len - b->len + 1;
	if (!(q == a && qsize < a->len) && !(q == b && qsize < b->len)) {
		if (!mpd_qresize(q, qsize, status)) {
			goto nanresult;
		}
	}

	rsize = b->len;
	if (!(r == a && rsize < a->len)) {
		if (!mpd_qresize(r, rsize, status)) {
			goto nanresult;
		}
	}

	if (b->len == 1) {
		if (a->len == 1) {
			_mpd_div_word(&q->data[0], &r->data[0], a->data[0], b->data[0]);
		}
		else {
			r->data[0] = _mpd_shortdiv(q->data, a->data, a->len, b->data[0]);
		}
	}
	else if (a->len < 2*MPD_NEWTONDIV_CUTOFF &&
	         b->len < MPD_NEWTONDIV_CUTOFF) {
		int ret;
		ret = _mpd_basedivmod(q->data, r->data, a->data, b->data,
		                      a->len, b->len);
		if (ret == -1) {
			*status |= MPD_Malloc_error;
			goto nanresult;
		}
	}
	else {
		_mpd_qbarrett_divmod(q, r, a, b, status);
		if (mpd_isspecial(q) || mpd_isspecial(r)) {
			goto nanresult;
		}
		if (mpd_isinfinite(q) || q->digits > ctx->prec) {
			*status |= MPD_Division_impossible;
			goto nanresult;
		}
		qsize = q->len;
		rsize = r->len;
	}

	qsize = _mpd_real_size(q->data, qsize);
	/* resize to smaller cannot fail */
	mpd_qresize(q, qsize, status);
	q->len = qsize;
	mpd_setdigits(q);
	mpd_set_flags(q, sign_ab);
	q->exp = 0;
	if (q->digits > ctx->prec) {
		*status |= MPD_Division_impossible;
		goto nanresult;
	}

	rsize = _mpd_real_size(r->data, rsize);
	/* resize to smaller cannot fail */
	mpd_qresize(r, rsize, status);
	r->len = rsize;
	mpd_setdigits(r);
	mpd_set_flags(r, sign_a);
	r->exp = ideal_exp;

out:
	mpd_del(&aligned);
	return;

nanresult:
	mpd_setspecial(q, MPD_POS, MPD_NAN);
	mpd_setspecial(r, MPD_POS, MPD_NAN);
	goto out;
}

/* Integer division with remainder, special cases have been dealt with. */
void
mpd_qdivmod(mpd_t *q, mpd_t *r, const mpd_t *a, const mpd_t *b,
            const mpd_context_t *ctx, uint32_t *status)
{
	uint8_t sign = mpd_sign(a)^mpd_sign(b);

	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(q, a, b, ctx, status)) {
			mpd_qcopy(r, q, status);
			return;
		}
		if (mpd_isinfinite(a)) {
			if (mpd_isinfinite(b)) {
				mpd_setspecial(q, MPD_POS, MPD_NAN);
			}
			else {
				mpd_setspecial(q, sign, MPD_INF);
			}
			mpd_setspecial(r, MPD_POS, MPD_NAN);
			*status |= MPD_Invalid_operation;
			return;
		}
		if (mpd_isinfinite(b)) {
			if (!mpd_qcopy(r, a, status)) {
				mpd_seterror(q, MPD_Malloc_error, status);
				return;
			}
			mpd_qfinalize(r, ctx, status);
			_settriple(q, sign, 0, 0);
			return;
		}
		/* debug */
		abort(); /* GCOV_NOT_REACHED */
	}
	if (mpd_iszerocoeff(b)) {
		if (mpd_iszerocoeff(a)) {
			mpd_setspecial(q, MPD_POS, MPD_NAN);
			mpd_setspecial(r, MPD_POS, MPD_NAN);
			*status |= MPD_Division_undefined;
		}
		else {
			mpd_setspecial(q, sign, MPD_INF);
			mpd_setspecial(r, MPD_POS, MPD_NAN);
			*status |= (MPD_Division_by_zero|MPD_Invalid_operation);
		}
		return;
	}

	_mpd_qdivmod(q, r, a, b, ctx, status);
	mpd_qfinalize(q, ctx, status);
	mpd_qfinalize(r, ctx, status);
}

void
mpd_qdivint(mpd_t *q, const mpd_t *a, const mpd_t *b,
            const mpd_context_t *ctx, uint32_t *status)
{
	MPD_NEW_STATIC(r,0,0,0,0);
	uint8_t sign = mpd_sign(a)^mpd_sign(b);

	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(q, a, b, ctx, status)) {
			return;
		}
		if (mpd_isinfinite(a) && mpd_isinfinite(b)) {
			mpd_seterror(q, MPD_Invalid_operation, status);
			return;
		}
		if (mpd_isinfinite(a)) {
			mpd_setspecial(q, sign, MPD_INF);
			return;
		}
		if (mpd_isinfinite(b)) {
			_settriple(q, sign, 0, 0);
			return;
		}
		/* debug */
		abort(); /* GCOV_NOT_REACHED */
	}
	if (mpd_iszerocoeff(b)) {
		if (mpd_iszerocoeff(a)) {
			mpd_seterror(q, MPD_Division_undefined, status);
		}
		else {
			mpd_setspecial(q, sign, MPD_INF);
			*status |= MPD_Division_by_zero;
		}
		return;
	}


	_mpd_qdivmod(q, &r, a, b, ctx, status);
	mpd_del(&r);
	mpd_qfinalize(q, ctx, status);
}

/* Divide decimal by mpd_ssize_t. */
void
mpd_qdiv_ssize(mpd_t *result, const mpd_t *a, mpd_ssize_t b,
               const mpd_context_t *ctx, uint32_t *status)
{
	mpd_t bb;
	mpd_uint_t bdata[2];

	bb.data = bdata;
	bb.flags = MPD_STATIC|MPD_CONST_DATA;

	mpd_qsset_ssize(&bb, b, ctx, status);
	mpd_qdiv(result, a, &bb, ctx, status);
}

/* Divide decimal by mpd_uint_t. */
void
mpd_qdiv_uint(mpd_t *result, const mpd_t *a, mpd_uint_t b,
              const mpd_context_t *ctx, uint32_t *status)
{
	mpd_t bb;
	mpd_uint_t bdata[2];

	bb.data = bdata;
	bb.flags = MPD_STATIC|MPD_CONST_DATA;

	mpd_qsset_uint(&bb, b, ctx, status);
	mpd_qdiv(result, a, &bb, ctx, status);
}

/* Divide decimal by int32_t. */
void
mpd_qdiv_i32(mpd_t *result, const mpd_t *a, int32_t b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_qdiv_ssize(result, a, b, ctx, status);
}

/* Divide decimal by uint32_t. */
void
mpd_qdiv_u32(mpd_t *result, const mpd_t *a, uint32_t b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_qdiv_uint(result, a, b, ctx, status);
}

#ifdef CONFIG_64
/* Divide decimal by int64_t. */
void
mpd_qdiv_i64(mpd_t *result, const mpd_t *a, int64_t b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_qdiv_ssize(result, a, b, ctx, status);
}

/* Divide decimal by uint64_t. */
void
mpd_qdiv_u64(mpd_t *result, const mpd_t *a, uint64_t b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_qdiv_uint(result, a, b, ctx, status);
}
#endif

#if defined(_MSC_VER)
  /* conversion from 'double' to 'mpd_ssize_t', possible loss of data */
  #pragma warning(disable:4244)
#endif
/*
 * Get the number of iterations for the Horner scheme in _mpd_qexp().
 */
static inline mpd_ssize_t
_mpd_get_exp_iterations(const mpd_t *a, mpd_ssize_t prec)
{
	mpd_uint_t dummy;
	mpd_uint_t msdigits;
	double f;

	/* 9 is MPD_RDIGITS for 32 bit platforms */
	_mpd_get_msdigits(&dummy, &msdigits, a, 9);
	f = ((double)msdigits + 1) / mpd_pow10[mpd_word_digits(msdigits)];

#ifdef CONFIG_64
  #ifdef USE_80BIT_LONG_DOUBLE
	return ceill((1.435*(long double)prec - 1.182)
	             / log10l((long double)prec/f));
  #else
	/* prec > floor((1ULL<<53) / 1.435) */
	if (prec > 6276793905742851LL) {
		return MPD_SSIZE_MAX;
	}
	return ceil((1.435*(double)prec - 1.182) / log10((double)prec/f));
  #endif
#else /* CONFIG_32 */
	return ceil((1.435*(double)prec - 1.182) / log10((double)prec/f));
	#if defined(_MSC_VER)
	  #pragma warning(default:4244)
	#endif
#endif
}

/*
 * Internal function, specials have been dealt with.
 *
 * The algorithm is from Hull&Abrham, Variable Precision Exponential Function,
 * ACM Transactions on Mathematical Software, Vol. 12, No. 2, June 1986.
 *
 * Main differences:
 *
 *  - The number of iterations for the Horner scheme is calculated using the
 *    C log10() function.
 *
 *  - The analysis for early abortion has been adapted for the mpd_t
 *    ranges.
 */
static void
_mpd_qexp(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
          uint32_t *status)
{
	mpd_context_t workctx;
	MPD_NEW_STATIC(tmp,0,0,0,0);
	MPD_NEW_STATIC(sum,0,0,0,0);
	MPD_NEW_CONST(word,0,0,0,1,1,1);
	mpd_ssize_t j, n, t;

	assert(!mpd_isspecial(a));

	/*
	 * We are calculating e^x = e^(r*10^t) = (e^r)^(10^t), where r < 1 and t >= 0.
	 *
	 * If t > 0, we have:
	 *
	 *   (1) 0.1 <= r < 1, so e^r >= e^0.1. Overflow in the final power operation
	 *       will occur when (e^0.1)^(10^t) > 10^(emax+1). If we consider MAX_EMAX,
	 *       this will happen for t > 10 (32 bit) or (t > 19) (64 bit).
	 *
	 *   (2) -1 < r <= -0.1, so e^r > e^-1. Underflow in the final power operation
	 *       will occur when (e^-1)^(10^t) < 10^(etiny-1). If we consider MIN_ETINY,
	 *       this will also happen for t > 10 (32 bit) or (t > 19) (64 bit).
	 */
#if defined(CONFIG_64)
	#define MPD_EXP_MAX_T 19
#elif defined(CONFIG_32)
	#define MPD_EXP_MAX_T 10
#endif
	t = a->digits + a->exp;
	t = (t > 0) ? t : 0;
	if (t > MPD_EXP_MAX_T) {
		if (mpd_ispositive(a)) {
			mpd_setspecial(result, MPD_POS, MPD_INF);
			*status |= MPD_Overflow|MPD_Inexact|MPD_Rounded;
		}
		else {
			_settriple(result, MPD_POS, 0, mpd_etiny(ctx));
			*status |= (MPD_Inexact|MPD_Rounded|MPD_Subnormal|
			            MPD_Underflow|MPD_Clamped);
		}
		return;
	}

	mpd_maxcontext(&workctx);
	workctx.prec = ctx->prec + t + 2;
	workctx.prec = (workctx.prec < 9) ? 9 : workctx.prec;
	workctx.round = MPD_ROUND_HALF_EVEN;

	if ((n = _mpd_get_exp_iterations(a, workctx.prec)) == MPD_SSIZE_MAX) {
		mpd_seterror(result, MPD_Invalid_operation, status); /* GCOV_UNLIKELY */
		goto finish; /* GCOV_UNLIKELY */
	}

	if (!mpd_qcopy(result, a, status)) {
		goto finish;
	}
	result->exp -= t;

	_settriple(&sum, MPD_POS, 1, 0);

	for (j = n-1; j >= 1; j--) {
		word.data[0] = j;
		mpd_setdigits(&word);
		mpd_qdiv(&tmp, result, &word, &workctx, &workctx.status);
		mpd_qmul(&sum, &sum, &tmp, &workctx, &workctx.status);
		mpd_qadd(&sum, &sum, &one, &workctx, &workctx.status);
	}

#ifdef CONFIG_64
	_mpd_qpow_uint(result, &sum, mpd_pow10[t], MPD_POS, &workctx, status);
#else
	if (t <= MPD_MAX_POW10) {
		_mpd_qpow_uint(result, &sum, mpd_pow10[t], MPD_POS, &workctx, status);
	}
	else {
		t -= MPD_MAX_POW10;
		_mpd_qpow_uint(&tmp, &sum, mpd_pow10[MPD_MAX_POW10], MPD_POS,
		               &workctx, status);
		_mpd_qpow_uint(result, &tmp, mpd_pow10[t], MPD_POS, &workctx, status);
	}
#endif


finish:
	mpd_del(&tmp);
	mpd_del(&sum);
	*status |= (workctx.status&MPD_Errors);
	*status |= (MPD_Inexact|MPD_Rounded);
}

/* exp(a) */
void
mpd_qexp(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
         uint32_t *status)
{
	mpd_context_t workctx;

	if (mpd_isspecial(a)) {
		if (mpd_qcheck_nan(result, a, ctx, status)) {
			return;
		}
		if (mpd_isnegative(a)) {
			_settriple(result, MPD_POS, 0, 0);
		}
		else {
			mpd_setspecial(result, MPD_POS, MPD_INF);
		}
		return;
	}
	if (mpd_iszerocoeff(a)) {
		_settriple(result, MPD_POS, 1, 0);
		return;
	}

	workctx = *ctx;
	workctx.round = MPD_ROUND_HALF_EVEN;

	if (ctx->allcr) {
		MPD_NEW_STATIC(t1, 0,0,0,0);
		MPD_NEW_STATIC(t2, 0,0,0,0);
		MPD_NEW_STATIC(ulp, 0,0,0,0);
		MPD_NEW_STATIC(aa, 0,0,0,0);
		mpd_ssize_t prec;

		if (result == a) {
			if (!mpd_qcopy(&aa, a, status)) {
				mpd_seterror(result, MPD_Malloc_error, status);
				return;
			}
			a = &aa;
		}

		workctx.clamp = 0;
		prec = ctx->prec + 3;
		while (1) {
			workctx.prec = prec;
			_mpd_qexp(result, a, &workctx, status);
			_ssettriple(&ulp, MPD_POS, 1,
			            result->exp + result->digits-workctx.prec-1);

			workctx.prec = ctx->prec;
			mpd_qadd(&t1, result, &ulp, &workctx, &workctx.status);
			mpd_qsub(&t2, result, &ulp, &workctx, &workctx.status);
			if (mpd_isspecial(result) || mpd_iszerocoeff(result) ||
			    mpd_qcmp(&t1, &t2, status) == 0) {
				workctx.clamp = ctx->clamp;
				mpd_check_underflow(result, &workctx, status);
				mpd_qfinalize(result, &workctx, status);
				break;
			}
			prec += MPD_RDIGITS;
		}
		mpd_del(&t1);
		mpd_del(&t2);
		mpd_del(&ulp);
		mpd_del(&aa);
	}
	else {
		_mpd_qexp(result, a, &workctx, status);
		mpd_check_underflow(result, &workctx, status);
		mpd_qfinalize(result, &workctx, status);
	}
}

/* Fused multiply-add: (a * b) + c, with a single final rounding. */
void
mpd_qfma(mpd_t *result, const mpd_t *a, const mpd_t *b, const mpd_t *c,
         const mpd_context_t *ctx, uint32_t *status)
{
	uint32_t workstatus = 0;
	mpd_t *cc = (mpd_t *)c;

	if (result == c) {
		if ((cc = mpd_qncopy(c)) == NULL) {
			mpd_seterror(result, MPD_Malloc_error, status);
			return;
		}
	}

	_mpd_qmul(result, a, b, ctx, &workstatus);
	if (!(workstatus&MPD_Invalid_operation)) {
		mpd_qadd(result, result, cc, ctx, &workstatus);
	}

	if (cc != c) mpd_del(cc);
	*status |= workstatus;
}

static inline int
ln_schedule_prec(mpd_ssize_t klist[MPD_MAX_PREC_LOG2], mpd_ssize_t maxprec,
                 mpd_ssize_t initprec)
{
	mpd_ssize_t k;
	int i;

	assert(maxprec >= 2 && initprec >= 2);
	if (maxprec <= initprec) return -1;

	i = 0; k = maxprec;
	do {
		k = (k+2) / 2;
		klist[i++] = k;
	} while (k > initprec);

	return i-1;
}

/* Two word initial approximations for ln(10) */
#ifdef CONFIG_64
#if MPD_RDIGITS != 19
  #error "mpdecimal.c: MPD_RDIGITS must be 19."
#endif
static mpd_uint_t mpd_ln10_data[MPD_MINALLOC_MAX] = {
  179914546843642076, 2302585092994045684
};
static mpd_uint_t mpd_ln10_init[2] = {
  179914546843642076, 2302585092994045684
};
#else
#if MPD_RDIGITS != 9
  #error "mpdecimal.c: MPD_RDIGITS must be 9."
#endif
static mpd_uint_t mpd_ln10_data[MPD_MINALLOC_MAX] = {299404568, 230258509};
static mpd_uint_t mpd_ln10_init[2] = {299404568, 230258509};
#endif
/* mpd_ln10 is cached in order to speed up computations */
mpd_t mpd_ln10 = {MPD_STATIC|MPD_STATIC_DATA, -(2*MPD_RDIGITS-1),
                  2*MPD_RDIGITS, 2, MPD_MINALLOC_MAX, mpd_ln10_data};

static void
mpd_reset_ln10(void)
{
	if (mpd_isdynamic_data(&mpd_ln10)) {
		mpd_free(mpd_ln10.data);
	}
	mpd_ln10.data = mpd_ln10_data;
	mpd_ln10_data[0] = mpd_ln10_init[0];
	mpd_ln10_data[1] = mpd_ln10_init[1];
	mpd_ln10.flags = MPD_STATIC|MPD_STATIC_DATA;
	mpd_ln10.exp = -(2*MPD_RDIGITS-1);
	mpd_ln10.digits = 2*MPD_RDIGITS;
	mpd_ln10.len = 2;
	mpd_ln10.alloc = MPD_MINALLOC_MAX;
}

/*
 * Initializes or updates mpd_ln10. If mpd_ln10 is cached and has exactly the
 * requested precision, the function returns. If the cached precision is greater
 * than the requested precision, mpd_ln10 is shifted to the requested precision.
 *
 * The function can fail with MPD_Malloc_error.
 */
void
mpd_update_ln10(mpd_ssize_t maxprec, uint32_t *status)
{
	mpd_context_t varcontext, maxcontext;
	MPD_NEW_STATIC(tmp, 0,0,0,0);
	MPD_NEW_CONST(static10, 0,0,2,1,1,10);
	mpd_ssize_t klist[MPD_MAX_PREC_LOG2];
	int i;

	if (mpd_isspecial(&mpd_ln10)) {
		mpd_reset_ln10();
	}

	if (mpd_ln10.digits > maxprec) {
		/* shift to smaller cannot fail */
		mpd_qshiftr_inplace(&mpd_ln10, mpd_ln10.digits-maxprec);
		mpd_ln10.exp = -(mpd_ln10.digits-1);
		return;
	}
	else if (mpd_ln10.digits == maxprec) {
		return;
	}

	mpd_maxcontext(&maxcontext);
	mpd_maxcontext(&varcontext);
	varcontext.round = MPD_ROUND_TRUNC;

	i = ln_schedule_prec(klist, maxprec+2, mpd_ln10.digits);
	for (; i >= 0; i--) {
		varcontext.prec = 2*klist[i]+3;
		mpd_ln10.flags ^= MPD_NEG;
		_mpd_qexp(&tmp, &mpd_ln10, &varcontext, status);
		mpd_ln10.flags ^= MPD_NEG;
		mpd_qmul(&tmp, &static10, &tmp, &varcontext, status);
		mpd_qsub(&tmp, &tmp, &one, &maxcontext, status);
		mpd_qadd(&mpd_ln10, &mpd_ln10, &tmp, &maxcontext, status);
		if (mpd_isspecial(&mpd_ln10)) {
			break;
		}
	}

	mpd_del(&tmp);
	varcontext.prec = maxprec;
	varcontext.round = MPD_ROUND_HALF_EVEN;
	mpd_qfinalize(&mpd_ln10, &varcontext, status);
}

/* Initial approximations for the ln() iteration */
static const uint16_t lnapprox[900] = {
  /* index 0 - 400: log((i+100)/100) * 1000 */
  0, 10, 20, 30, 39, 49, 58, 68, 77, 86, 95, 104, 113, 122, 131, 140, 148, 157,
  166, 174, 182, 191, 199, 207, 215, 223, 231, 239, 247, 255, 262, 270, 278,
  285, 293, 300, 308, 315, 322, 329, 336, 344, 351, 358, 365, 372, 378, 385,
  392, 399, 406, 412, 419, 425, 432, 438, 445, 451, 457, 464, 470, 476, 482,
  489, 495, 501, 507, 513, 519, 525, 531, 536, 542, 548, 554, 560, 565, 571,
  577, 582, 588, 593, 599, 604, 610, 615, 621, 626, 631, 637, 642, 647, 652,
  658, 663, 668, 673, 678, 683, 688, 693, 698, 703, 708, 713, 718, 723, 728,
  732, 737, 742, 747, 751, 756, 761, 766, 770, 775, 779, 784, 788, 793, 798,
  802, 806, 811, 815, 820, 824, 829, 833, 837, 842, 846, 850, 854, 859, 863,
  867, 871, 876, 880, 884, 888, 892, 896, 900, 904, 908, 912, 916, 920, 924,
  928, 932, 936, 940, 944, 948, 952, 956, 959, 963, 967, 971, 975, 978, 982,
  986, 990, 993, 997, 1001, 1004, 1008, 1012, 1015, 1019, 1022, 1026, 1030,
  1033, 1037, 1040, 1044, 1047, 1051, 1054, 1058, 1061, 1065, 1068, 1072, 1075,
  1078, 1082, 1085, 1089, 1092, 1095, 1099, 1102, 1105, 1109, 1112, 1115, 1118,
  1122, 1125, 1128, 1131, 1135, 1138, 1141, 1144, 1147, 1151, 1154, 1157, 1160,
  1163, 1166, 1169, 1172, 1176, 1179, 1182, 1185, 1188, 1191, 1194, 1197, 1200,
  1203, 1206, 1209, 1212, 1215, 1218, 1221, 1224, 1227, 1230, 1233, 1235, 1238,
  1241, 1244, 1247, 1250, 1253, 1256, 1258, 1261, 1264, 1267, 1270, 1273, 1275,
  1278, 1281, 1284, 1286, 1289, 1292, 1295, 1297, 1300, 1303, 1306, 1308, 1311,
  1314, 1316, 1319, 1322, 1324, 1327, 1330, 1332, 1335, 1338, 1340, 1343, 1345,
  1348, 1351, 1353, 1356, 1358, 1361, 1364, 1366, 1369, 1371, 1374, 1376, 1379,
  1381, 1384, 1386, 1389, 1391, 1394, 1396, 1399, 1401, 1404, 1406, 1409, 1411,
  1413, 1416, 1418, 1421, 1423, 1426, 1428, 1430, 1433, 1435, 1437, 1440, 1442,
  1445, 1447, 1449, 1452, 1454, 1456, 1459, 1461, 1463, 1466, 1468, 1470, 1472,
  1475, 1477, 1479, 1482, 1484, 1486, 1488, 1491, 1493, 1495, 1497, 1500, 1502,
  1504, 1506, 1509, 1511, 1513, 1515, 1517, 1520, 1522, 1524, 1526, 1528, 1530,
  1533, 1535, 1537, 1539, 1541, 1543, 1545, 1548, 1550, 1552, 1554, 1556, 1558,
  1560, 1562, 1564, 1567, 1569, 1571, 1573, 1575, 1577, 1579, 1581, 1583, 1585,
  1587, 1589, 1591, 1593, 1595, 1597, 1599, 1601, 1603, 1605, 1607, 1609,
  /* index 401 - 899: -log((i+100)/1000) * 1000 */
  691, 689, 687, 685, 683, 681, 679, 677, 675, 673, 671, 669, 668, 666, 664,
  662, 660, 658, 656, 654, 652, 650, 648, 646, 644, 642, 641, 639, 637, 635,
  633, 631, 629, 627, 626, 624, 622, 620, 618, 616, 614, 612, 611, 609, 607,
  605, 603, 602, 600, 598, 596, 594, 592, 591, 589, 587, 585, 583, 582, 580,
  578, 576, 574, 573, 571, 569, 567, 566, 564, 562, 560, 559, 557, 555, 553,
  552, 550, 548, 546, 545, 543, 541, 540, 538, 536, 534, 533, 531, 529, 528,
  526, 524, 523, 521, 519, 518, 516, 514, 512, 511, 509, 508, 506, 504, 502,
  501, 499, 498, 496, 494, 493, 491, 489, 488, 486, 484, 483, 481, 480, 478,
  476, 475, 473, 472, 470, 468, 467, 465, 464, 462, 460, 459, 457, 456, 454,
  453, 451, 449, 448, 446, 445, 443, 442, 440, 438, 437, 435, 434, 432, 431,
  429, 428, 426, 425, 423, 422, 420, 419, 417, 416, 414, 412, 411, 410, 408,
  406, 405, 404, 402, 400, 399, 398, 396, 394, 393, 392, 390, 389, 387, 386,
  384, 383, 381, 380, 378, 377, 375, 374, 372, 371, 370, 368, 367, 365, 364,
  362, 361, 360, 358, 357, 355, 354, 352, 351, 350, 348, 347, 345, 344, 342,
  341, 340, 338, 337, 336, 334, 333, 331, 330, 328, 327, 326, 324, 323, 322,
  320, 319, 318, 316, 315, 313, 312, 311, 309, 308, 306, 305, 304, 302, 301,
  300, 298, 297, 296, 294, 293, 292, 290, 289, 288, 286, 285, 284, 282, 281,
  280, 278, 277, 276, 274, 273, 272, 270, 269, 268, 267, 265, 264, 263, 261,
  260, 259, 258, 256, 255, 254, 252, 251, 250, 248, 247, 246, 245, 243, 242,
  241, 240, 238, 237, 236, 234, 233, 232, 231, 229, 228, 227, 226, 224, 223,
  222, 221, 219, 218, 217, 216, 214, 213, 212, 211, 210, 208, 207, 206, 205,
  203, 202, 201, 200, 198, 197, 196, 195, 194, 192, 191, 190, 189, 188, 186,
  185, 184, 183, 182, 180, 179, 178, 177, 176, 174, 173, 172, 171, 170, 168,
  167, 166, 165, 164, 162, 161, 160, 159, 158, 157, 156, 154, 153, 152, 151,
  150, 148, 147, 146, 145, 144, 143, 142, 140, 139, 138, 137, 136, 135, 134,
  132, 131, 130, 129, 128, 127, 126, 124, 123, 122, 121, 120, 119, 118, 116,
  115, 114, 113, 112, 111, 110, 109, 108, 106, 105, 104, 103, 102, 101, 100,
  99, 98, 97, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 84, 83, 82, 81, 80, 79,
  78, 77, 76, 75, 74, 73, 72, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59,
  58, 57, 56, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39,
  38, 37, 36, 35, 34, 33, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19,
  18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1
};

/* Internal ln() function that does not check for specials, zero or one. */
static void
_mpd_qln(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
         uint32_t *status)
{
	mpd_context_t varcontext, maxcontext;
	mpd_t *z = (mpd_t *) result;
	MPD_NEW_STATIC(v,0,0,0,0);
	MPD_NEW_STATIC(vtmp,0,0,0,0);
	MPD_NEW_STATIC(tmp,0,0,0,0);
	mpd_ssize_t klist[MPD_MAX_PREC_LOG2];
	mpd_ssize_t maxprec, shift, t;
	mpd_ssize_t a_digits, a_exp;
	mpd_uint_t dummy, x;
	int i;

	assert(!mpd_isspecial(a) && !mpd_iszerocoeff(a));

	/*
	 * We are calculating ln(a) = ln(v * 10^t) = ln(v) + t*ln(10),
	 * where 0.5 < v <= 5.
	 */
	if (!mpd_qcopy(&v, a, status)) {
		mpd_seterror(result, MPD_Malloc_error, status);
		goto finish;
	}

	/* Initial approximation: we have at least one non-zero digit */
	_mpd_get_msdigits(&dummy, &x, &v, 3);
	if (x < 10) x *= 10;
	if (x < 100) x *= 10;
	x -= 100;

	/* a may equal z */
	a_digits = a->digits;
	a_exp = a->exp;

	mpd_minalloc(z);
	mpd_clear_flags(z);
	z->data[0] = lnapprox[x];
	z->len = 1;
	z->exp = -3;
	mpd_setdigits(z);

	if (x <= 400) {
		v.exp = -(a_digits - 1);
		t = a_exp + a_digits - 1;
	}
	else {
		v.exp = -a_digits;
		t = a_exp + a_digits;
		mpd_set_negative(z);
	}

	mpd_maxcontext(&maxcontext);
	mpd_maxcontext(&varcontext);
	varcontext.round = MPD_ROUND_TRUNC;

	maxprec = ctx->prec + 2;
	if (x <= 10 || x >= 805) {
		/* v is close to 1: Estimate the magnitude of the logarithm.
		 * If v = 1 or ln(v) will underflow, skip the loop. Otherwise,
		 * adjust the precision upwards in order to obtain a sufficient
		 * number of significant digits.
		 *
		 *   1) x/(1+x) < ln(1+x) < x, for x > -1, x != 0
		 *
		 *   2) (v-1)/v < ln(v) < v-1
		 */
		mpd_t *lower = &tmp;
		mpd_t *upper = &vtmp;
		int cmp = _mpd_cmp(&v, &one);

		varcontext.round = MPD_ROUND_CEILING;
		varcontext.prec = maxprec;
		mpd_qsub(upper, &v, &one, &varcontext, &varcontext.status);
		varcontext.round = MPD_ROUND_FLOOR;
		mpd_qdiv(lower, upper, &v, &varcontext, &varcontext.status);
		varcontext.round = MPD_ROUND_TRUNC;

		if (cmp < 0) {
			_mpd_ptrswap(&upper, &lower);
		}
		if (mpd_adjexp(upper) < mpd_etiny(ctx)) {
			_settriple(z, (cmp<0), 1, mpd_etiny(ctx)-1);
			goto postloop;
		}
		if (mpd_adjexp(lower) < 0) {
			maxprec = maxprec - mpd_adjexp(lower);
		}
	}

	i = ln_schedule_prec(klist, maxprec, 2);
	for (; i >= 0; i--) {
		varcontext.prec = 2*klist[i]+3;
		z->flags ^= MPD_NEG;
		_mpd_qexp(&tmp, z, &varcontext, status);
		z->flags ^= MPD_NEG;

		if (v.digits > varcontext.prec) {
			shift = v.digits - varcontext.prec;
			mpd_qshiftr(&vtmp, &v, shift, status);
			vtmp.exp += shift;
			mpd_qmul(&tmp, &vtmp, &tmp, &varcontext, status);
		}
		else {
			mpd_qmul(&tmp, &v, &tmp, &varcontext, status);
		}

		mpd_qsub(&tmp, &tmp, &one, &maxcontext, status);
		mpd_qadd(z, z, &tmp, &maxcontext, status);
		if (mpd_isspecial(z)) {
			break;
		}
	}

postloop:
	mpd_update_ln10(maxprec+2, status);
	mpd_qmul_ssize(&tmp, &mpd_ln10, t, &maxcontext, status);
	varcontext.prec = maxprec+2;
	mpd_qadd(result, &tmp, z, &varcontext, status);


finish:
	mpd_del(&v);
	mpd_del(&vtmp);
	mpd_del(&tmp);
}

/* ln(a) */
void
mpd_qln(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
        uint32_t *status)
{
	mpd_context_t workctx;
	mpd_ssize_t adjexp, t;

	if (mpd_isspecial(a)) {
		if (mpd_qcheck_nan(result, a, ctx, status)) {
			return;
		}
		if (mpd_isnegative(a)) {
			mpd_seterror(result, MPD_Invalid_operation, status);
			return;
		}
		mpd_setspecial(result, MPD_POS, MPD_INF);
		return;
	}
	if (mpd_iszerocoeff(a)) {
		mpd_setspecial(result, MPD_NEG, MPD_INF);
		return;
	}
	if (mpd_isnegative(a)) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	if (_mpd_cmp(a, &one) == 0) {
		_settriple(result, MPD_POS, 0, 0);
		return;
	}
	/* Check if the result will overflow.
	 *
	 * 1) adjexp(a) + 1 > log10(a) >= adjexp(a)
	 *
	 * 2) |log10(a)| >= adjexp(a), if adjexp(a) >= 0
	 *    |log10(a)| > -adjexp(a)-1, if adjexp(a) < 0
	 *
	 * 3) |log(a)| > 2*|log10(a)|
	 */
	adjexp = mpd_adjexp(a);
	t = (adjexp < 0) ? -adjexp-1 : adjexp;
	t *= 2;
	if (mpd_exp_digits(t)-1 > ctx->emax) {
		*status |= MPD_Overflow|MPD_Inexact|MPD_Rounded;
		mpd_setspecial(result, (adjexp<0), MPD_INF);
		return;
	}

	workctx = *ctx;
	workctx.round = MPD_ROUND_HALF_EVEN;

	if (ctx->allcr) {
		MPD_NEW_STATIC(t1, 0,0,0,0);
		MPD_NEW_STATIC(t2, 0,0,0,0);
		MPD_NEW_STATIC(ulp, 0,0,0,0);
		MPD_NEW_STATIC(aa, 0,0,0,0);
		mpd_ssize_t prec;

		if (result == a) {
			if (!mpd_qcopy(&aa, a, status)) {
				mpd_seterror(result, MPD_Malloc_error, status);
				return;
			}
			a = &aa;
		}

		workctx.clamp = 0;
		prec = ctx->prec + 3;
		while (1) {
			workctx.prec = prec;
			_mpd_qln(result, a, &workctx, status);
			_ssettriple(&ulp, MPD_POS, 1,
			            result->exp + result->digits-workctx.prec-1);

			workctx.prec = ctx->prec;
			mpd_qadd(&t1, result, &ulp, &workctx, &workctx.status);
			mpd_qsub(&t2, result, &ulp, &workctx, &workctx.status);
			if (mpd_isspecial(result) || mpd_iszerocoeff(result) ||
			    mpd_qcmp(&t1, &t2, status) == 0) {
				workctx.clamp = ctx->clamp;
				mpd_check_underflow(result, &workctx, status);
				mpd_qfinalize(result, &workctx, status);
				break;
			}
			prec += MPD_RDIGITS;
		}
		mpd_del(&t1);
		mpd_del(&t2);
		mpd_del(&ulp);
		mpd_del(&aa);
	}
	else {
		_mpd_qln(result, a, &workctx, status);
		mpd_check_underflow(result, &workctx, status);
		mpd_qfinalize(result, &workctx, status);
	}
}

/* Internal log10() function that does not check for specials, zero, ... */
static void
_mpd_qlog10(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
            uint32_t *status)
{
	mpd_context_t workctx;

	mpd_maxcontext(&workctx);
	workctx.prec = ctx->prec + 3;
	_mpd_qln(result, a, &workctx, status);
	mpd_update_ln10(workctx.prec, status);

	workctx = *ctx;
	workctx.round = MPD_ROUND_HALF_EVEN;
	mpd_qdiv(result, result, &mpd_ln10, &workctx, status);
}

/* log10(a) */
void
mpd_qlog10(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
           uint32_t *status)
{
	mpd_context_t workctx;
	mpd_ssize_t adjexp, t;

	workctx = *ctx;
	workctx.round = MPD_ROUND_HALF_EVEN;

	if (mpd_isspecial(a)) {
		if (mpd_qcheck_nan(result, a, ctx, status)) {
			return;
		}
		if (mpd_isnegative(a)) {
			mpd_seterror(result, MPD_Invalid_operation, status);
			return;
		}
		mpd_setspecial(result, MPD_POS, MPD_INF);
		return;
	}
	if (mpd_iszerocoeff(a)) {
		mpd_setspecial(result, MPD_NEG, MPD_INF);
		return;
	}
	if (mpd_isnegative(a)) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	if (mpd_coeff_ispow10(a)) {
		uint8_t sign = 0;
		adjexp = mpd_adjexp(a);
		if (adjexp < 0) {
			sign = 1;
			adjexp = -adjexp;
		}
		_settriple(result, sign, adjexp, 0);
		mpd_qfinalize(result, &workctx, status);
		return;
	}
	/* Check if the result will overflow.
	 *
	 * 1) adjexp(a) + 1 > log10(a) >= adjexp(a)
	 *
	 * 2) |log10(a)| >= adjexp(a), if adjexp(a) >= 0
	 *    |log10(a)| > -adjexp(a)-1, if adjexp(a) < 0
	 */
	adjexp = mpd_adjexp(a);
	t = (adjexp < 0) ? -adjexp-1 : adjexp;
	if (mpd_exp_digits(t)-1 > ctx->emax) {
		*status |= MPD_Overflow|MPD_Inexact|MPD_Rounded;
		mpd_setspecial(result, (adjexp<0), MPD_INF);
		return;
	}

	if (ctx->allcr) {
		MPD_NEW_STATIC(t1, 0,0,0,0);
		MPD_NEW_STATIC(t2, 0,0,0,0);
		MPD_NEW_STATIC(ulp, 0,0,0,0);
		MPD_NEW_STATIC(aa, 0,0,0,0);
		mpd_ssize_t prec;

		if (result == a) {
			if (!mpd_qcopy(&aa, a, status)) {
				mpd_seterror(result, MPD_Malloc_error, status);
				return;
			}
			a = &aa;
		}

		workctx.clamp = 0;
		prec = ctx->prec + 3;
		while (1) {
			workctx.prec = prec;
			_mpd_qlog10(result, a, &workctx, status);
			_ssettriple(&ulp, MPD_POS, 1,
			            result->exp + result->digits-workctx.prec-1);

			workctx.prec = ctx->prec;
			mpd_qadd(&t1, result, &ulp, &workctx, &workctx.status);
			mpd_qsub(&t2, result, &ulp, &workctx, &workctx.status);
			if (mpd_isspecial(result) || mpd_iszerocoeff(result) ||
			    mpd_qcmp(&t1, &t2, status) == 0) {
				workctx.clamp = ctx->clamp;
				mpd_check_underflow(result, &workctx, status);
				mpd_qfinalize(result, &workctx, status);
				break;
			}
			prec += MPD_RDIGITS;
		}
		mpd_del(&t1);
		mpd_del(&t2);
		mpd_del(&ulp);
		mpd_del(&aa);
	}
	else {
		_mpd_qlog10(result, a, &workctx, status);
		mpd_check_underflow(result, &workctx, status);
	}
}

/*
 * Maximum of the two operands. Attention: If one operand is a quiet NaN and the
 * other is numeric, the numeric operand is returned. This may not be what one
 * expects.
 */
void
mpd_qmax(mpd_t *result, const mpd_t *a, const mpd_t *b,
         const mpd_context_t *ctx, uint32_t *status)
{
	int c;

	if (mpd_isqnan(a) && !mpd_isnan(b)) {
		mpd_qcopy(result, b, status);
	}
	else if (mpd_isqnan(b) && !mpd_isnan(a)) {
		mpd_qcopy(result, a, status);
	}
	else if (mpd_qcheck_nans(result, a, b, ctx, status)) {
		return;
	}
	else {
		c = _mpd_cmp(a, b);
		if (c == 0) {
			c = _mpd_cmp_numequal(a, b);
		}

		if (c < 0) {
			mpd_qcopy(result, b, status);
		}
		else {
			mpd_qcopy(result, a, status);
		}
	}

	mpd_qfinalize(result, ctx, status);
}

/*
 * Maximum magnitude: Same as mpd_max(), but compares the operands with their
 * sign ignored.
 */
void
mpd_qmax_mag(mpd_t *result, const mpd_t *a, const mpd_t *b,
             const mpd_context_t *ctx, uint32_t *status)
{
	int c;

	if (mpd_isqnan(a) && !mpd_isnan(b)) {
		mpd_qcopy(result, b, status);
	}
	else if (mpd_isqnan(b) && !mpd_isnan(a)) {
		mpd_qcopy(result, a, status);
	}
	else if (mpd_qcheck_nans(result, a, b, ctx, status)) {
		return;
	}
	else {
		c = _mpd_cmp_abs(a, b);
		if (c == 0) {
			c = _mpd_cmp_numequal(a, b);
		}

		if (c < 0) {
			mpd_qcopy(result, b, status);
		}
		else {
			mpd_qcopy(result, a, status);
		}
	}

	mpd_qfinalize(result, ctx, status);
}

/*
 * Minimum of the two operands. Attention: If one operand is a quiet NaN and the
 * other is numeric, the numeric operand is returned. This may not be what one
 * expects.
 */
void
mpd_qmin(mpd_t *result, const mpd_t *a, const mpd_t *b,
         const mpd_context_t *ctx, uint32_t *status)
{
	int c;

	if (mpd_isqnan(a) && !mpd_isnan(b)) {
		mpd_qcopy(result, b, status);
	}
	else if (mpd_isqnan(b) && !mpd_isnan(a)) {
		mpd_qcopy(result, a, status);
	}
	else if (mpd_qcheck_nans(result, a, b, ctx, status)) {
		return;
	}
	else {
		c = _mpd_cmp(a, b);
		if (c == 0) {
			c = _mpd_cmp_numequal(a, b);
		}

		if (c < 0) {
			mpd_qcopy(result, a, status);
		}
		else {
			mpd_qcopy(result, b, status);
		}
	}

	mpd_qfinalize(result, ctx, status);
}

/*
 * Minimum magnitude: Same as mpd_min(), but compares the operands with their
 * sign ignored.
 */
void
mpd_qmin_mag(mpd_t *result, const mpd_t *a, const mpd_t *b,
             const mpd_context_t *ctx, uint32_t *status)
{
	int c;

	if (mpd_isqnan(a) && !mpd_isnan(b)) {
		mpd_qcopy(result, b, status);
	}
	else if (mpd_isqnan(b) && !mpd_isnan(a)) {
		mpd_qcopy(result, a, status);
	}
	else if (mpd_qcheck_nans(result, a, b, ctx, status)) {
		return;
	}
	else {
		c = _mpd_cmp_abs(a, b);
		if (c == 0) {
			c = _mpd_cmp_numequal(a, b);
		}

		if (c < 0) {
			mpd_qcopy(result, a, status);
		}
		else {
			mpd_qcopy(result, b, status);
		}
	}

	mpd_qfinalize(result, ctx, status);
}

/* Minimum space needed for the result array in _karatsuba_rec(). */
static inline mpd_size_t
_kmul_resultsize(mpd_size_t la, mpd_size_t lb)
{
	mpd_size_t n, m;

	n = add_size_t(la, lb);
	n = add_size_t(n, 1);

	m = (la+1)/2 + 1;
	m = mul_size_t(m, 3);

	return (m > n) ? m : n;
}

/* Work space needed in _karatsuba_rec(). lim >= 4 */
static inline mpd_size_t
_kmul_worksize(mpd_size_t n, mpd_size_t lim)
{
	mpd_size_t m;

	if (n <= lim) {
		return 0;
	}

	m = (n+1)/2 + 1;

	return add_size_t(mul_size_t(m, 2), _kmul_worksize(m, lim));
}


#define MPD_KARATSUBA_BASECASE 16  /* must be >= 4 */

/*
 * Add the product of a and b to c.
 * c must be _kmul_resultsize(la, lb) in size.
 * w is used as a work array and must be _kmul_worksize(a, lim) in size.
 * Roman E. Maeder, Storage Allocation for the Karatsuba Integer Multiplication
 * Algorithm. In "Design and implementation of symbolic computation systems",
 * Springer, 1993, ISBN 354057235X, 9783540572350.
 */
static void
_karatsuba_rec(mpd_uint_t *c, const mpd_uint_t *a, const mpd_uint_t *b,
               mpd_uint_t *w, mpd_size_t la, mpd_size_t lb)
{
	mpd_size_t m, lt;

	assert (la >= lb && lb > 0);

	if (la <= MPD_KARATSUBA_BASECASE) {
		_mpd_basemul(c, a, b, la, lb);
		return;
	}

	m = (la+1)/2;  // ceil(la/2)

	/* lb <= m < la */
	if (lb <= m) {

		/* lb can now be larger than la-m */
		if (lb > la-m) {
			lt = lb + lb + 1;       // space needed for result array
			mpd_uint_zero(w, lt);   // clear result array
			_karatsuba_rec(w, b, a+m, w+lt, lb, la-m); // b*ah
		}
		else {
			lt = (la-m) + (la-m) + 1;  // space needed for result array
			mpd_uint_zero(w, lt);      // clear result array
			_karatsuba_rec(w, a+m, b, w+lt, la-m, lb); // ah*b
		}
		_mpd_baseaddto(c+m, w, (la-m)+lb);      // add ah*b*B**m

		lt = m + m + 1;         // space needed for the result array
		mpd_uint_zero(w, lt);   // clear result array
		_karatsuba_rec(w, a, b, w+lt, m, lb);  // al*b
		_mpd_baseaddto(c, w, m+lb);    // add al*b

		return;
	}

	/* la >= lb > m */
	memcpy(w, a, m * sizeof *w);
	w[m] = 0;
	_mpd_baseaddto(w, a+m, la-m);

	memcpy(w+(m+1), b, m * sizeof *w);
	w[m+1+m] = 0;
	_mpd_baseaddto(w+(m+1), b+m, lb-m);

	_karatsuba_rec(c+m, w, w+(m+1), w+2*(m+1), m+1, m+1);

	lt = (la-m) + (la-m) + 1;
	mpd_uint_zero(w, lt);

	_karatsuba_rec(w, a+m, b+m, w+lt, la-m, lb-m);

	_mpd_baseaddto(c+2*m, w, (la-m) + (lb-m));
	_mpd_basesubfrom(c+m, w, (la-m) + (lb-m));

	lt = m + m + 1;
	mpd_uint_zero(w, lt);

	_karatsuba_rec(w, a, b, w+lt, m, m);
	_mpd_baseaddto(c, w, m+m);
	_mpd_basesubfrom(c+m, w, m+m);

	return;
}

/*
 * Multiply u and v, using Karatsuba multiplication. Returns a pointer
 * to the result or NULL in case of failure (malloc error).
 * Conditions: ulen >= vlen, ulen >= 4
 */
mpd_uint_t *
_mpd_kmul(const mpd_uint_t *u, const mpd_uint_t *v,
          mpd_size_t ulen, mpd_size_t vlen,
          mpd_size_t *rsize)
{
	mpd_uint_t *result = NULL, *w = NULL;
	mpd_size_t m;

	assert(ulen >= 4);
	assert(ulen >= vlen);

	*rsize = _kmul_resultsize(ulen, vlen);
	if ((result = mpd_calloc(*rsize, sizeof *result)) == NULL) {
		return NULL;
	}

	m = _kmul_worksize(ulen, MPD_KARATSUBA_BASECASE);
	if (m && ((w = mpd_calloc(m, sizeof *w)) == NULL)) {
		mpd_free(result);
		return NULL;
	}

	_karatsuba_rec(result, u, v, w, ulen, vlen);


	if (w) mpd_free(w);
	return result;
}


/* Determine the minimum length for the number theoretic transform. */
static inline mpd_size_t
_mpd_get_transform_len(mpd_size_t rsize)
{
	mpd_size_t log2rsize;
	mpd_size_t x, step;

	assert(rsize >= 4);
	log2rsize = mpd_bsr(rsize);

	if (rsize <= 1024) {
		x = ONE_UM<<log2rsize;
		return (rsize == x) ? x : x<<1;
	}
	else if (rsize <= MPD_MAXTRANSFORM_2N) {
		x = ONE_UM<<log2rsize;
		if (rsize == x) return x;
		step = x>>1;
		x += step;
		return (rsize <= x) ? x : x + step;
	}
	else if (rsize <= MPD_MAXTRANSFORM_2N+MPD_MAXTRANSFORM_2N/2) {
		return MPD_MAXTRANSFORM_2N+MPD_MAXTRANSFORM_2N/2;
	}
	else if (rsize <= 3*MPD_MAXTRANSFORM_2N) {
		return 3*MPD_MAXTRANSFORM_2N;
	}
	else {
		return MPD_SIZE_MAX;
	}
}

#ifdef PPRO
#ifndef _MSC_VER
static inline unsigned short
_mpd_get_control87(void)
{
	unsigned short cw;

	__asm__ __volatile__ ("fnstcw %0" : "=m" (cw));
	return cw;
}

static inline void
_mpd_set_control87(unsigned short cw)
{
	__asm__ __volatile__ ("fldcw %0" : : "m" (cw));
}
#endif

unsigned int
mpd_set_fenv(void)
{
	unsigned int cw;
#ifdef _MSC_VER
	cw = _control87(0, 0);
	_control87((_RC_CHOP|_PC_64), (_MCW_RC|_MCW_PC));
#else
	cw = _mpd_get_control87();
	_mpd_set_control87(cw|0x780);
#endif
	return cw;
}

void
mpd_restore_fenv(unsigned int cw)
{
#ifdef _MSC_VER
	_control87(cw, (_MCW_RC|_MCW_PC));
#else
	_mpd_set_control87((unsigned short)cw);
#endif
}
#endif /* PPRO */

/*
 * Multiply u and v, using the fast number theoretic transform. Returns
 * a pointer to the result or NULL in case of failure (malloc error).
 */
mpd_uint_t *
_mpd_fntmul(const mpd_uint_t *u, const mpd_uint_t *v,
            mpd_size_t ulen, mpd_size_t vlen,
            mpd_size_t *rsize)
{
	mpd_uint_t *c1 = NULL, *c2 = NULL, *c3 = NULL, *vtmp = NULL;
	mpd_size_t n;

#ifdef PPRO
	unsigned int cw;
	cw = mpd_set_fenv();
#endif

	*rsize = add_size_t(ulen, vlen);
	if ((n = _mpd_get_transform_len(*rsize)) == MPD_SIZE_MAX) {
		goto malloc_error;
	}

	if ((c1 = mpd_calloc(sizeof *c1, n)) == NULL) {
		goto malloc_error;
	}
	if ((c2 = mpd_calloc(sizeof *c2, n)) == NULL) {
		goto malloc_error;
	}
	if ((c3 = mpd_calloc(sizeof *c3, n)) == NULL) {
		goto malloc_error;
	}

	memcpy(c1, u, ulen * (sizeof *c1));
	memcpy(c2, u, ulen * (sizeof *c2));
	memcpy(c3, u, ulen * (sizeof *c3));

	if (u == v) {
		if (!fnt_autoconvolute(c1, n, P1) ||
		    !fnt_autoconvolute(c2, n, P2) ||
		    !fnt_autoconvolute(c3, n, P3)) {
			goto malloc_error;
		}
	}
	else {
		if ((vtmp = mpd_calloc(sizeof *vtmp, n)) == NULL) {
			goto malloc_error;
		}

		memcpy(vtmp, v, vlen * (sizeof *vtmp));
		if (!fnt_convolute(c1, vtmp, n, P1)) {
			mpd_free(vtmp);
			goto malloc_error;
		}

		memcpy(vtmp, v, vlen * (sizeof *vtmp));
		mpd_uint_zero(vtmp+vlen, n-vlen);
		if (!fnt_convolute(c2, vtmp, n, P2)) {
			mpd_free(vtmp);
			goto malloc_error;
		}

		memcpy(vtmp, v, vlen * (sizeof *vtmp));
		mpd_uint_zero(vtmp+vlen, n-vlen);
		if (!fnt_convolute(c3, vtmp, n, P3)) {
			mpd_free(vtmp);
			goto malloc_error;
		}

		mpd_free(vtmp);
	}

	crt3(c1, c2, c3, *rsize);

out:
#ifdef PPRO
	mpd_restore_fenv(cw);
#endif
	if (c2) mpd_free(c2);
	if (c3) mpd_free(c3);
	return c1;

malloc_error:
	if (c1) mpd_free(c1);
	c1 = NULL;
	goto out;
}


/*
 * Karatsuba multiplication with FNT/basemul as the base case.
 */
static int
_karatsuba_rec_fnt(mpd_uint_t *c, const mpd_uint_t *a, const mpd_uint_t *b,
                   mpd_uint_t *w, mpd_size_t la, mpd_size_t lb)
{
	mpd_size_t m, lt;

	assert (la >= lb && lb > 0);

	if (la <= 3*(MPD_MAXTRANSFORM_2N/2)) {

		if (lb <= 192) {
			_mpd_basemul(c, b, a, lb, la);
		}
		else {
			mpd_uint_t *result;
			mpd_size_t dummy;

			if ((result = _mpd_fntmul(a, b, la, lb, &dummy)) == NULL) {
				return 0;
			}
			memcpy(c, result, (la+lb) * (sizeof *result));
			mpd_free(result);
		}
		return 1;
	}

	m = (la+1)/2;  // ceil(la/2)

	/* lb <= m < la */
	if (lb <= m) {

		/* lb can now be larger than la-m */
		if (lb > la-m) {
			lt = lb + lb + 1;       // space needed for result array
			mpd_uint_zero(w, lt);   // clear result array
			if (!_karatsuba_rec_fnt(w, b, a+m, w+lt, lb, la-m)) { // b*ah
				return 0; /* GCOV_UNLIKELY */
			}
		}
		else {
			lt = (la-m) + (la-m) + 1;  // space needed for result array
			mpd_uint_zero(w, lt);      // clear result array
			if (!_karatsuba_rec_fnt(w, a+m, b, w+lt, la-m, lb)) { // ah*b
				return 0; /* GCOV_UNLIKELY */
			}
		}
		_mpd_baseaddto(c+m, w, (la-m)+lb); // add ah*b*B**m

		lt = m + m + 1;         // space needed for the result array
		mpd_uint_zero(w, lt);   // clear result array
		if (!_karatsuba_rec_fnt(w, a, b, w+lt, m, lb)) {  // al*b
			return 0; /* GCOV_UNLIKELY */
		}
		_mpd_baseaddto(c, w, m+lb);       // add al*b

		return 1;
	}

	/* la >= lb > m */
	memcpy(w, a, m * sizeof *w);
	w[m] = 0;
	_mpd_baseaddto(w, a+m, la-m);

	memcpy(w+(m+1), b, m * sizeof *w);
	w[m+1+m] = 0;
	_mpd_baseaddto(w+(m+1), b+m, lb-m);

	if (!_karatsuba_rec_fnt(c+m, w, w+(m+1), w+2*(m+1), m+1, m+1)) {
		return 0; /* GCOV_UNLIKELY */
	}

	lt = (la-m) + (la-m) + 1;
	mpd_uint_zero(w, lt);

	if (!_karatsuba_rec_fnt(w, a+m, b+m, w+lt, la-m, lb-m)) {
		return 0; /* GCOV_UNLIKELY */
	}

	_mpd_baseaddto(c+2*m, w, (la-m) + (lb-m));
	_mpd_basesubfrom(c+m, w, (la-m) + (lb-m));

	lt = m + m + 1;
	mpd_uint_zero(w, lt);

	if (!_karatsuba_rec_fnt(w, a, b, w+lt, m, m)) {
		return 0; /* GCOV_UNLIKELY */
	}
	_mpd_baseaddto(c, w, m+m);
	_mpd_basesubfrom(c+m, w, m+m);

	return 1;
}

/*
 * Multiply u and v, using Karatsuba multiplication with the FNT as the
 * base case. Returns a pointer to the result or NULL in case of failure
 * (malloc error). Conditions: ulen >= vlen, ulen >= 4.
 */
mpd_uint_t *
_mpd_kmul_fnt(const mpd_uint_t *u, const mpd_uint_t *v,
              mpd_size_t ulen, mpd_size_t vlen,
              mpd_size_t *rsize)
{
	mpd_uint_t *result = NULL, *w = NULL;
	mpd_size_t m;

	assert(ulen >= 4);
	assert(ulen >= vlen);

	*rsize = _kmul_resultsize(ulen, vlen);
	if ((result = mpd_calloc(*rsize, sizeof *result)) == NULL) {
		return NULL;
	}

	m = _kmul_worksize(ulen, 3*(MPD_MAXTRANSFORM_2N/2));
	if (m && ((w = mpd_calloc(m, sizeof *w)) == NULL)) {
		mpd_free(result); /* GCOV_UNLIKELY */
		return NULL; /* GCOV_UNLIKELY */
	}

	if (!_karatsuba_rec_fnt(result, u, v, w, ulen, vlen)) {
		mpd_free(result);
		result = NULL;
	}


	if (w) mpd_free(w);
	return result;
}


/* Deal with the special cases of multiplying infinities. */
static void
_mpd_qmul_inf(mpd_t *result, const mpd_t *a, const mpd_t *b, uint32_t *status)
{
	if (mpd_isinfinite(a)) {
		if (mpd_iszero(b)) {
			mpd_seterror(result, MPD_Invalid_operation, status);
		}
		else {
			mpd_setspecial(result, mpd_sign(a)^mpd_sign(b), MPD_INF);
		}
		return;
	}
	assert(mpd_isinfinite(b));
	if (mpd_iszero(a)) {
		mpd_seterror(result, MPD_Invalid_operation, status);
	}
	else {
		mpd_setspecial(result, mpd_sign(a)^mpd_sign(b), MPD_INF);
	}
}

/*
 * Internal function: Multiply a and b. _mpd_qmul deals with specials but
 * does NOT finalize the result. This is for use in mpd_fma().
 */
static inline void
_mpd_qmul(mpd_t *result, const mpd_t *a, const mpd_t *b,
          const mpd_context_t *ctx, uint32_t *status)
{
	mpd_t *big = (mpd_t *)a, *small = (mpd_t *)b;
	mpd_uint_t *rdata = NULL;
	mpd_uint_t rbuf[MPD_MINALLOC_MAX];
	mpd_size_t rsize, i;


	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(result, a, b, ctx, status)) {
			return;
		}
		_mpd_qmul_inf(result, a, b, status);
		return;
	}

	if (small->len > big->len) {
		_mpd_ptrswap(&big, &small);
	}

	rsize = big->len + small->len;

	if (big->len == 1) {
		_mpd_singlemul(result->data, big->data[0], small->data[0]);
		goto finish;
	}
	if (rsize <= (mpd_size_t)MPD_MINALLOC_MAX) {
		if (big->len == 2) {
			_mpd_mul_2_le2(rbuf, big->data, small->data, small->len);
		}
		else {
			mpd_uint_zero(rbuf, rsize);
			if (small->len == 1) {
				_mpd_shortmul(rbuf, big->data, big->len, small->data[0]);
			}
			else {
				_mpd_basemul(rbuf, small->data, big->data, small->len, big->len);
			}
		}
		if (!mpd_qresize(result, rsize, status)) {
			return;
		}
		for(i = 0; i < rsize; i++) {
			result->data[i] = rbuf[i];
		}
		goto finish;
	}


	if (small->len == 1) {
		if ((rdata = mpd_calloc(rsize, sizeof *rdata)) == NULL) {
			mpd_seterror(result, MPD_Malloc_error, status);
			return;
		}
		_mpd_shortmul(rdata, big->data, big->len, small->data[0]);
	}
	else if (rsize <= 1024) {
		rdata = _mpd_kmul(big->data, small->data, big->len, small->len, &rsize);
		if (rdata == NULL) {
			mpd_seterror(result, MPD_Malloc_error, status);
			return;
		}
	}
	else if (rsize <= 3*MPD_MAXTRANSFORM_2N) {
		rdata = _mpd_fntmul(big->data, small->data, big->len, small->len, &rsize);
		if (rdata == NULL) {
			mpd_seterror(result, MPD_Malloc_error, status);
			return;
		}
	}
	else {
		rdata = _mpd_kmul_fnt(big->data, small->data, big->len, small->len, &rsize);
		if (rdata == NULL) {
			mpd_seterror(result, MPD_Malloc_error, status); /* GCOV_UNLIKELY */
			return; /* GCOV_UNLIKELY */
		}
	}

	if (mpd_isdynamic_data(result)) {
		mpd_free(result->data);
	}
	result->data = rdata;
	result->alloc = rsize;
	mpd_set_dynamic_data(result);


finish:
	mpd_set_flags(result, mpd_sign(a)^mpd_sign(b));
	result->exp = big->exp + small->exp;
	result->len = _mpd_real_size(result->data, rsize);
	/* resize to smaller cannot fail */
	mpd_qresize(result, result->len, status);
	mpd_setdigits(result);
}

/* Multiply a and b. */
void
mpd_qmul(mpd_t *result, const mpd_t *a, const mpd_t *b,
         const mpd_context_t *ctx, uint32_t *status)
{
	_mpd_qmul(result, a, b, ctx, status);
	mpd_qfinalize(result, ctx, status);
}

/* Multiply decimal and mpd_ssize_t. */
void
mpd_qmul_ssize(mpd_t *result, const mpd_t *a, mpd_ssize_t b,
               const mpd_context_t *ctx, uint32_t *status)
{
	mpd_t bb;
	mpd_uint_t bdata[2];

	bb.data = bdata;
	bb.flags = MPD_STATIC|MPD_CONST_DATA;

	mpd_qsset_ssize(&bb, b, ctx, status);
	mpd_qmul(result, a, &bb, ctx, status);
}

/* Multiply decimal and mpd_uint_t. */
void
mpd_qmul_uint(mpd_t *result, const mpd_t *a, mpd_uint_t b,
              const mpd_context_t *ctx, uint32_t *status)
{
	mpd_t bb;
	mpd_uint_t bdata[2];

	bb.data = bdata;
	bb.flags = MPD_STATIC|MPD_CONST_DATA;

	mpd_qsset_uint(&bb, b, ctx, status);
	mpd_qmul(result, a, &bb, ctx, status);
}

void
mpd_qmul_i32(mpd_t *result, const mpd_t *a, int32_t b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_qmul_ssize(result, a, b, ctx, status);
}

void
mpd_qmul_u32(mpd_t *result, const mpd_t *a, uint32_t b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_qmul_uint(result, a, b, ctx, status);
}

#ifdef CONFIG_64
void
mpd_qmul_i64(mpd_t *result, const mpd_t *a, int64_t b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_qmul_ssize(result, a, b, ctx, status);
}

void
mpd_qmul_u64(mpd_t *result, const mpd_t *a, uint64_t b,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_qmul_uint(result, a, b, ctx, status);
}
#endif

/* Like the minus operator. */
void
mpd_qminus(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
           uint32_t *status)
{
	if (mpd_isspecial(a)) {
		if (mpd_qcheck_nan(result, a, ctx, status)) {
			return;
		}
	}

	if (mpd_iszero(a)) {
		mpd_qcopy_abs(result, a, status);
	}
	else {
		mpd_qcopy_negate(result, a, status);
	}

	mpd_qfinalize(result, ctx, status);
}

/* Like the plus operator. */
void
mpd_qplus(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
          uint32_t *status)
{
	if (mpd_isspecial(a)) {
		if (mpd_qcheck_nan(result, a, ctx, status)) {
			return;
		}
	}

	if (mpd_iszero(a)) {
		mpd_qcopy_abs(result, a, status);
	}
	else {
		mpd_qcopy(result, a, status);
	}

	mpd_qfinalize(result, ctx, status);
}

/* The largest representable number that is smaller than the operand. */
void
mpd_qnext_minus(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
                uint32_t *status)
{
	mpd_context_t workctx; /* function context */
	MPD_NEW_CONST(tiny,MPD_POS,mpd_etiny(ctx)-1,1,1,1,1);

	if (mpd_isspecial(a)) {
		if (mpd_qcheck_nan(result, a, ctx, status)) {
			return;
		}
		if (mpd_isinfinite(a)) {
			if (mpd_isnegative(a)) {
				mpd_qcopy(result, a, status);
				return;
			}
			else {
				mpd_clear_flags(result);
				mpd_qmaxcoeff(result, ctx, status);
				if (mpd_isnan(result)) {
					return;
				}
				result->exp = ctx->emax - ctx->prec + 1;
				return;
			}
		}
		/* debug */
		abort(); /* GCOV_NOT_REACHED */
	}

	mpd_workcontext(&workctx, ctx);
	workctx.round = MPD_ROUND_FLOOR;

	if (!mpd_qcopy(result, a, status)) {
		return;
	}

	mpd_qfinalize(result, &workctx, &workctx.status);
	if (workctx.status&(MPD_Inexact|MPD_Errors)) {
		*status |= (workctx.status&MPD_Errors);
		return;
	}

	workctx.status = 0;
	mpd_qsub(result, a, &tiny, &workctx, &workctx.status);
	*status |= (workctx.status&MPD_Errors);
}

/* The smallest representable number that is larger than the operand. */
void
mpd_qnext_plus(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
               uint32_t *status)
{
	mpd_context_t workctx;
	MPD_NEW_CONST(tiny,MPD_POS,mpd_etiny(ctx)-1,1,1,1,1);

	if (mpd_isspecial(a)) {
		if (mpd_qcheck_nan(result, a, ctx, status)) {
			return;
		}
		if (mpd_isinfinite(a)) {
			if (mpd_ispositive(a)) {
				mpd_qcopy(result, a, status);
			}
			else {
				mpd_clear_flags(result);
				mpd_qmaxcoeff(result, ctx, status);
				if (mpd_isnan(result)) {
					return;
				}
				mpd_set_flags(result, MPD_NEG);
				result->exp = mpd_etop(ctx);
			}
			return;
		}
	}

	mpd_workcontext(&workctx, ctx);
	workctx.round = MPD_ROUND_CEILING;

	if (!mpd_qcopy(result, a, status)) {
		return;
	}

	mpd_qfinalize(result, &workctx, &workctx.status);
	if (workctx.status & (MPD_Inexact|MPD_Errors)) {
		*status |= (workctx.status&MPD_Errors);
		return;
	}

	workctx.status = 0;
	mpd_qadd(result, a, &tiny, &workctx, &workctx.status);
	*status |= (workctx.status&MPD_Errors);
}

/*
 * The number closest to the first operand that is in the direction towards
 * the second operand.
 */
void
mpd_qnext_toward(mpd_t *result, const mpd_t *a, const mpd_t *b,
                 const mpd_context_t *ctx, uint32_t *status)
{
	int c;

	if (mpd_isnan(a) || mpd_isnan(b)) {
		if (mpd_qcheck_nans(result, a, b, ctx, status))
			return;
	}

	c = _mpd_cmp(a, b);
	if (c == 0) {
		mpd_qcopy_sign(result, a, b, status);
		return;
	}

	if (c < 0) {
		mpd_qnext_plus(result, a, ctx, status);
	}
	else {
		mpd_qnext_minus(result, a, ctx, status);
	}

	if (mpd_isinfinite(result)) {
		*status |= (MPD_Overflow|MPD_Rounded|MPD_Inexact);
	}
	else if (mpd_adjexp(result) < ctx->emin) {
		*status |= (MPD_Underflow|MPD_Subnormal|MPD_Rounded|MPD_Inexact);
		if (mpd_iszero(result)) {
			*status |= MPD_Clamped;
		}
	}
}

/*
 * Internal function: Integer power with mpd_uint_t exponent, base is modified!
 * Function can fail with MPD_Malloc_error.
 */
static inline void
_mpd_qpow_uint(mpd_t *result, mpd_t *base, mpd_uint_t exp, uint8_t resultsign,
               const mpd_context_t *ctx, uint32_t *status)
{
	uint32_t workstatus = 0;
	mpd_uint_t n;

	if (exp == 0) {
		_settriple(result, resultsign, 1, 0); /* GCOV_NOT_REACHED */
		return; /* GCOV_NOT_REACHED */
	}

	if (!mpd_qcopy(result, base, status)) {
		return;
	}

	n = mpd_bits[mpd_bsr(exp)];
	while (n >>= 1) {
		mpd_qmul(result, result, result, ctx, &workstatus);
		if (exp & n) {
			mpd_qmul(result, result, base, ctx, &workstatus);
		}
		if (workstatus & (MPD_Overflow|MPD_Clamped)) {
			break;
		}
	}

	*status |= workstatus;
	mpd_set_sign(result, resultsign);
}

/*
 * Internal function: Integer power with mpd_t exponent, tbase and texp
 * are modified!! Function can fail with MPD_Malloc_error.
 */
static inline void
_mpd_qpow_mpd(mpd_t *result, mpd_t *tbase, mpd_t *texp, uint8_t resultsign,
              const mpd_context_t *ctx, uint32_t *status)
{
	uint32_t workstatus = 0;
	mpd_context_t maxctx;
	MPD_NEW_CONST(two,0,0,1,1,1,2);


	mpd_maxcontext(&maxctx);

	/* resize to smaller cannot fail */
	mpd_qcopy(result, &one, status);

	while (!mpd_iszero(texp)) {
		if (mpd_isodd(texp)) {
			mpd_qmul(result, result, tbase, ctx, &workstatus);
			*status |= workstatus;
			if (workstatus & (MPD_Overflow|MPD_Clamped)) {
				break;
			}
		}
		mpd_qmul(tbase, tbase, tbase, ctx, &workstatus);
		mpd_qdivint(texp, texp, &two, &maxctx, &workstatus);
		if (mpd_isnan(tbase) || mpd_isnan(texp)) {
			mpd_seterror(result, workstatus&MPD_Errors, status);
			return;
		}
	}
	mpd_set_sign(result, resultsign);
}

/*
 * The power function for integer exponents.
 */
static void
_mpd_qpow_int(mpd_t *result, const mpd_t *base, const mpd_t *exp,
              uint8_t resultsign,
              const mpd_context_t *ctx, uint32_t *status)
{
	mpd_context_t workctx;
	MPD_NEW_STATIC(tbase,0,0,0,0);
	MPD_NEW_STATIC(texp,0,0,0,0);
	mpd_ssize_t n;


	mpd_workcontext(&workctx, ctx);
	workctx.prec += (exp->digits + exp->exp + 2);
	workctx.round = MPD_ROUND_HALF_EVEN;
	workctx.clamp = 0;
	if (mpd_isnegative(exp)) {
		mpd_qdiv(&tbase, &one, base, &workctx, status);
		if (*status&MPD_Errors) {
			mpd_setspecial(result, MPD_POS, MPD_NAN);
			goto finish;
		}
	}
	else {
		if (!mpd_qcopy(&tbase, base, status)) {
			mpd_setspecial(result, MPD_POS, MPD_NAN);
			goto finish;
		}
	}

	n = mpd_qabs_uint(exp, &workctx.status);
	if (workctx.status&MPD_Invalid_operation) {
		if (!mpd_qcopy(&texp, exp, status)) {
			mpd_setspecial(result, MPD_POS, MPD_NAN); /* GCOV_UNLIKELY */
			goto finish; /* GCOV_UNLIKELY */
		}
		_mpd_qpow_mpd(result, &tbase, &texp, resultsign, &workctx, status);
	}
	else {
		_mpd_qpow_uint(result, &tbase, n, resultsign, &workctx, status);
	}

	if (mpd_isinfinite(result)) {
		/* for ROUND_DOWN, ROUND_FLOOR, etc. */
		_settriple(result, resultsign, 1, MPD_EXP_INF);
	}

finish:
	mpd_del(&tbase);
	mpd_del(&texp);
	mpd_qfinalize(result, ctx, status);
}

/*
 * This is an internal function that does not check for NaNs.
 */
static int
_qcheck_pow_one_inf(mpd_t *result, const mpd_t *base, uint8_t resultsign,
                    const mpd_context_t *ctx, uint32_t *status)
{
	mpd_ssize_t shift;
	int cmp;

	if ((cmp = _mpd_cmp(base, &one)) == 0) {
		shift = ctx->prec-1;
		mpd_qshiftl(result, &one, shift, status);
		result->exp = -shift;
		mpd_set_flags(result, resultsign);
		*status |= (MPD_Inexact|MPD_Rounded);
	}

	return cmp;
}

/*
 * If base equals one, calculate the correct power of one result.
 * Otherwise, result is undefined. Return the value of the comparison
 * against 1.
 *
 * This is an internal function that does not check for specials.
 */
static int
_qcheck_pow_one(mpd_t *result, const mpd_t *base, const mpd_t *exp,
                uint8_t resultsign,
                const mpd_context_t *ctx, uint32_t *status)
{
	uint32_t workstatus = 0;
	mpd_ssize_t shift;
	int cmp;

	if ((cmp = _mpd_cmp_abs(base, &one)) == 0) {
		if (_mpd_isint(exp)) {
			if (mpd_isnegative(exp)) {
				_settriple(result, resultsign, 1, 0);
				return 0;
			}
			/* 1.000**3 = 1.000000000 */
			mpd_qmul_ssize(result, exp, -base->exp, ctx, &workstatus);
			if (workstatus&MPD_Errors) {
				*status |= (workstatus&MPD_Errors);
				return 0;
			}
			/* digits-1 after exponentiation */
			shift = mpd_qget_ssize(result, &workstatus);
			/* shift is MPD_SSIZE_MAX if result is too large */
			if (shift > ctx->prec-1) {
				shift = ctx->prec-1;
				*status |= MPD_Rounded;
			}
		}
		else if (mpd_ispositive(base)) {
			shift = ctx->prec-1;
			*status |= (MPD_Inexact|MPD_Rounded);
		}
		else {
			return -2; /* GCOV_NOT_REACHED */
		}
		if (!mpd_qshiftl(result, &one, shift, status)) {
			return 0;
		}
		result->exp = -shift;
		mpd_set_flags(result, resultsign);
	}

	return cmp;
}

/*
 * Detect certain over/underflow of x**y.
 * ACL2 proof: pow_bounds.lisp.
 *
 *   Symbols:
 *
 *     e: EXP_INF or EXP_CLAMP
 *     x: base
 *     y: exponent
 *
 *     omega(e) = log10(abs(e))
 *     zeta(x)  = log10(abs(log10(x)))
 *     theta(y) = log10(abs(y))
 *
 *   Upper and lower bounds:
 *
 *     ub_omega(e) = ceil(log10(abs(e)))
 *     lb_theta(y) = floor(log10(abs(y)))
 *
 *                  | floor(log10(floor(abs(log10(x))))) if x < 1/10 or x >= 10
 *     lb_zeta(x) = | floor(log10(abs(x-1)/10)) if 1/10 <= x < 1
 *                  | floor(log10(abs((x-1)/100))) if 1 < x < 10
 *
 *   ub_omega(e) and lb_theta(y) are obviously upper and lower bounds
 *   for omega(e) and theta(y).
 *
 *   lb_zeta is a lower bound for zeta(x):
 *
 *     x < 1/10 or x >= 10:
 *
 *       abs(log10(x)) >= 1, so the outer log10 is well defined. Since log10
 *       is strictly increasing, the end result is a lower bound.
 *
 *     1/10 <= x < 1:
 *
 *       We use: log10(x) <= (x-1)/log(10)
 *               abs(log10(x)) >= abs(x-1)/log(10)
 *               abs(log10(x)) >= abs(x-1)/10
 *
 *     1 < x < 10:
 *
 *       We use: (x-1)/(x*log(10)) < log10(x)
 *               abs((x-1)/100) < abs(log10(x))
 *
 *       XXX: abs((x-1)/10) would work, need ACL2 proof.
 *
 *
 *   Let (0 < x < 1 and y < 0) or (x > 1 and y > 0).                  (H1)
 *   Let ub_omega(exp_inf) < lb_zeta(x) + lb_theta(y)                 (H2)
 *
 *   Then:
 *       log10(abs(exp_inf)) < log10(abs(log10(x))) + log10(abs(y)).   (1)
 *                   exp_inf < log10(x) * y                            (2)
 *               10**exp_inf < x**y                                    (3)
 *
 *   Let (0 < x < 1 and y > 0) or (x > 1 and y < 0).                  (H3)
 *   Let ub_omega(exp_clamp) < lb_zeta(x) + lb_theta(y)               (H4)
 *
 *   Then:
 *     log10(abs(exp_clamp)) < log10(abs(log10(x))) + log10(abs(y)).   (4)
 *              log10(x) * y < exp_clamp                               (5)
 *                      x**y < 10**exp_clamp                           (6)
 *
 */
static mpd_ssize_t
_lower_bound_zeta(const mpd_t *x, uint32_t *status)
{
	mpd_context_t maxctx;
	MPD_NEW_STATIC(scratch,0,0,0,0);
	mpd_ssize_t t, u;

	t = mpd_adjexp(x);
	if (t > 0) {
                /* x >= 10 -> floor(log10(floor(abs(log10(x))))) */
		return mpd_exp_digits(t) - 1;
	}
	else if (t < -1) {
		/* x < 1/10 -> floor(log10(floor(abs(log10(x))))) */
		return mpd_exp_digits(t+1) - 1;
	}
	else {
		mpd_maxcontext(&maxctx);
		mpd_qsub(&scratch, x, &one, &maxctx, status);
		if (mpd_isspecial(&scratch)) {
			mpd_del(&scratch);
			return MPD_SSIZE_MAX;
		}
		u = mpd_adjexp(&scratch);
		mpd_del(&scratch);

		/* t == -1, 1/10 <= x < 1 -> floor(log10(abs(x-1)/10))
		 * t == 0,  1 < x < 10    -> floor(log10(abs(x-1)/100)) */
		return (t == 0) ? u-2 : u-1;
	}
}

/*
 * Detect cases of certain overflow/underflow in the power function.
 * Assumptions: x != 1, y != 0. The proof above is for positive x.
 * If x is negative and y is an odd integer, x**y == -(abs(x)**y),
 * so the analysis does not change.
 */
static int
_qcheck_pow_bounds(mpd_t *result, const mpd_t *x, const mpd_t *y,
                   uint8_t resultsign,
                   const mpd_context_t *ctx, uint32_t *status)
{
	MPD_NEW_SHARED(abs_x, x);
	mpd_ssize_t ub_omega, lb_zeta, lb_theta;
	uint8_t sign;

	mpd_set_positive(&abs_x);

	lb_theta = mpd_adjexp(y);
	lb_zeta = _lower_bound_zeta(&abs_x, status);
	if (lb_zeta == MPD_SSIZE_MAX) {
		mpd_seterror(result, MPD_Malloc_error, status);
		return 1;
	}

	sign = (mpd_adjexp(&abs_x) < 0) ^ mpd_sign(y);
	if (sign == 0) {
		/* (0 < |x| < 1 and y < 0) or (|x| > 1 and y > 0) */
		ub_omega = mpd_exp_digits(ctx->emax);
		if (ub_omega < lb_zeta + lb_theta) {
			_settriple(result, resultsign, 1, MPD_EXP_INF);
			mpd_qfinalize(result, ctx, status);
			return 1;
		}
	}
	else {
                /* (0 < |x| < 1 and y > 0) or (|x| > 1 and y < 0). */
		ub_omega = mpd_exp_digits(mpd_etiny(ctx));
		if (ub_omega < lb_zeta + lb_theta) {
			_settriple(result, resultsign, 1, mpd_etiny(ctx)-1);
			mpd_qfinalize(result, ctx, status);
			return 1;
		}
	}

	return 0;
}

/*
 * TODO: Implement algorithm for computing exact powers from decimal.py.
 * In order to prevent infinite loops, this has to be called before
 * using Ziv's strategy for correct rounding.
 */
/*
static int
_mpd_qpow_exact(mpd_t *result, const mpd_t *base, const mpd_t *exp,
                const mpd_context_t *ctx, uint32_t *status)
{
	return 0;
}
*/

/* The power function for real exponents */
static void
_mpd_qpow_real(mpd_t *result, const mpd_t *base, const mpd_t *exp,
               const mpd_context_t *ctx, uint32_t *status)
{
	mpd_context_t workctx;
	MPD_NEW_STATIC(texp,0,0,0,0);

	if (!mpd_qcopy(&texp, exp, status)) {
		mpd_seterror(result, MPD_Malloc_error, status);
		return;
	}

	mpd_maxcontext(&workctx);
	workctx.prec = (base->digits > ctx->prec) ? base->digits : ctx->prec;
	workctx.prec += (4 + MPD_EXPDIGITS);
	workctx.round = MPD_ROUND_HALF_EVEN;
	workctx.allcr = ctx->allcr;

	mpd_qln(result, base, &workctx, &workctx.status);
	mpd_qmul(result, result, &texp, &workctx, &workctx.status);
	mpd_qexp(result, result, &workctx, status);

	mpd_del(&texp);
	*status |= (workctx.status&MPD_Errors);
	*status |= (MPD_Inexact|MPD_Rounded);
}

/* The power function: base**exp */
void
mpd_qpow(mpd_t *result, const mpd_t *base, const mpd_t *exp,
         const mpd_context_t *ctx, uint32_t *status)
{
	uint8_t resultsign = 0;
	int intexp = 0;
	int cmp;

	if (mpd_isspecial(base) || mpd_isspecial(exp)) {
		if (mpd_qcheck_nans(result, base, exp, ctx, status)) {
			return;
		}
	}
	if (mpd_isinteger(exp)) {
		intexp = 1;
		resultsign = mpd_isnegative(base) && mpd_isodd(exp);
	}

	if (mpd_iszero(base)) {
		if (mpd_iszero(exp)) {
			mpd_seterror(result, MPD_Invalid_operation, status);
		}
		else if (mpd_isnegative(exp)) {
			mpd_setspecial(result, resultsign, MPD_INF);
		}
		else {
			_settriple(result, resultsign, 0, 0);
		}
		return;
	}
	if (mpd_isnegative(base)) {
		if (!intexp || mpd_isinfinite(exp)) {
			mpd_seterror(result, MPD_Invalid_operation, status);
			return;
		}
	}
	if (mpd_isinfinite(exp)) {
		/* power of one */
		cmp = _qcheck_pow_one_inf(result, base, resultsign, ctx, status);
		if (cmp == 0) {
			return;
		}
		else {
			cmp *= mpd_arith_sign(exp);
			if (cmp < 0) {
				_settriple(result, resultsign, 0, 0);
			}
			else {
				mpd_setspecial(result, resultsign, MPD_INF);
			}
		}
		return;
	}
	if (mpd_isinfinite(base)) {
		if (mpd_iszero(exp)) {
			_settriple(result, resultsign, 1, 0);
		}
		else if (mpd_isnegative(exp)) {
			_settriple(result, resultsign, 0, 0);
		}
		else {
			mpd_setspecial(result, resultsign, MPD_INF);
		}
		return;
	}
	if (mpd_iszero(exp)) {
		_settriple(result, resultsign, 1, 0);
		return;
	}
	if (_qcheck_pow_one(result, base, exp, resultsign, ctx, status) == 0) {
		return;
	}
	if (_qcheck_pow_bounds(result, base, exp, resultsign, ctx, status)) {
		return;
	}

	if (intexp) {
		_mpd_qpow_int(result, base, exp, resultsign, ctx, status);
	}
	else {
		_mpd_qpow_real(result, base, exp, ctx, status);
		if (!mpd_isspecial(result) && _mpd_cmp(result, &one) == 0) {
			mpd_ssize_t shift = ctx->prec-1;
			mpd_qshiftl(result, &one, shift, status);
			result->exp = -shift;
		}
		if (mpd_isinfinite(result)) {
			/* for ROUND_DOWN, ROUND_FLOOR, etc. */
			_settriple(result, MPD_POS, 1, MPD_EXP_INF);
		}
		mpd_qfinalize(result, ctx, status);
	}
}

/*
 * Internal function: Integer powmod with mpd_uint_t exponent, base is modified!
 * Function can fail with MPD_Malloc_error.
 */
static inline void
_mpd_qpowmod_uint(mpd_t *result, mpd_t *base, mpd_uint_t exp,
                  mpd_t *mod, uint32_t *status)
{
	mpd_context_t maxcontext;

	mpd_maxcontext(&maxcontext);

	/* resize to smaller cannot fail */
	mpd_qcopy(result, &one, status);

	while (exp > 0) {
		if (exp & 1) {
			mpd_qmul(result, result, base, &maxcontext, status);
			mpd_qrem(result, result, mod, &maxcontext, status);
		}
		mpd_qmul(base, base, base, &maxcontext, status);
		mpd_qrem(base, base, mod, &maxcontext, status);
		exp >>= 1;
	}
}

/* The powmod function: (base**exp) % mod */
void
mpd_qpowmod(mpd_t *result, const mpd_t *base, const mpd_t *exp,
            const mpd_t *mod,
            const mpd_context_t *ctx, uint32_t *status)
{
	mpd_context_t maxcontext;
	MPD_NEW_STATIC(tbase,0,0,0,0);
	MPD_NEW_STATIC(texp,0,0,0,0);
	MPD_NEW_STATIC(tmod,0,0,0,0);
	MPD_NEW_STATIC(tmp,0,0,0,0);
	MPD_NEW_CONST(two,0,0,1,1,1,2);
	mpd_ssize_t tbase_exp, texp_exp;
	mpd_ssize_t i;
	mpd_t t;
	mpd_uint_t r;
	uint8_t sign;


	if (mpd_isspecial(base) || mpd_isspecial(exp) || mpd_isspecial(mod)) {
		if (mpd_qcheck_3nans(result, base, exp, mod, ctx, status)) {
			return;
		}
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}


	if (!_mpd_isint(base) || !_mpd_isint(exp) || !_mpd_isint(mod)) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	if (mpd_iszerocoeff(mod)) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	sign = (mpd_isnegative(base)) && (mpd_isodd(exp));
	if (mpd_iszerocoeff(exp)) {
		if (mpd_iszerocoeff(base)) {
			mpd_seterror(result, MPD_Invalid_operation, status);
			return;
		}
		r = (_mpd_cmp_abs(mod, &one)==0) ? 0 : 1;
		_settriple(result, sign, r, 0);
		return;
	}
	if (mpd_isnegative(exp)) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	if (mpd_iszerocoeff(base)) {
		_settriple(result, sign, 0, 0);
		return;
	}
	if (mod->digits+mod->exp > ctx->prec) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}

	if (!mpd_qcopy(&tmod, mod, status)) {
		goto mpd_errors;
	}
	mpd_set_positive(&tmod);

	mpd_maxcontext(&maxcontext);

	mpd_qround_to_int(&tbase, base, &maxcontext, status);
	mpd_qround_to_int(&texp, exp, &maxcontext, status);
	mpd_qround_to_int(&tmod, &tmod, &maxcontext, status);

	tbase_exp = tbase.exp;
	tbase.exp = 0;
	texp_exp = texp.exp;
	texp.exp = 0;

	/* base = (base.int % modulo * pow(10, base.exp, modulo)) % modulo */
	mpd_qrem(&tbase, &tbase, &tmod, &maxcontext, status);
	_settriple(result, MPD_POS, 1, tbase_exp);
	mpd_qrem(result, result, &tmod, &maxcontext, status);
	mpd_qmul(&tbase, &tbase, result, &maxcontext, status);
	mpd_qrem(&tbase, &tbase, &tmod, &maxcontext, status);
	if (mpd_isspecial(&tbase) ||
	    mpd_isspecial(&texp) ||
	    mpd_isspecial(&tmod)) {
		goto mpd_errors;
	}

	for (i = 0; i < texp_exp; i++) {
		_mpd_qpowmod_uint(&tmp, &tbase, 10, &tmod, status);
		t = tmp;
		tmp = tbase;
		tbase = t;
	}
	if (mpd_isspecial(&tbase)) {
		goto mpd_errors; /* GCOV_UNLIKELY */
	}

	/* resize to smaller cannot fail */
	mpd_qcopy(result, &one, status);
	while (mpd_isfinite(&texp) && !mpd_iszero(&texp)) {
		if (mpd_isodd(&texp)) {
			mpd_qmul(result, result, &tbase, &maxcontext, status);
			mpd_qrem(result, result, &tmod, &maxcontext, status);
		}
		mpd_qmul(&tbase, &tbase, &tbase, &maxcontext, status);
		mpd_qrem(&tbase, &tbase, &tmod, &maxcontext, status);
		mpd_qdivint(&texp, &texp, &two, &maxcontext, status);
	}
	if (mpd_isspecial(&texp) || mpd_isspecial(&tbase) ||
            mpd_isspecial(&tmod) || mpd_isspecial(result)) {
		/* MPD_Malloc_error */
		goto mpd_errors;
	}
	else {
		mpd_set_sign(result, sign);
	}

out:
	mpd_del(&tbase);
	mpd_del(&texp);
	mpd_del(&tmod);
	mpd_del(&tmp);
	mpd_qfinalize(result, ctx, status);
	return;

mpd_errors:
	mpd_setspecial(result, MPD_POS, MPD_NAN);
	goto out;
}

void
mpd_qquantize(mpd_t *result, const mpd_t *a, const mpd_t *b,
              const mpd_context_t *ctx, uint32_t *status)
{
	uint32_t workstatus = 0;
	mpd_ssize_t b_exp = b->exp;
	mpd_ssize_t expdiff, shift;
	mpd_uint_t rnd;

	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(result, a, b, ctx, status)) {
			return;
		}
		if (mpd_isinfinite(a) && mpd_isinfinite(b)) {
			mpd_qcopy(result, a, status);
			return;
		}
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}

	if (b->exp > ctx->emax || b->exp < mpd_etiny(ctx)) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}

	if (mpd_iszero(a)) {
		_settriple(result, mpd_sign(a), 0, b->exp);
		mpd_qfinalize(result, ctx, status);
		return;
	}


	expdiff = a->exp - b->exp;
	if (a->digits + expdiff > ctx->prec) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}

	if (expdiff >= 0) {
		shift = expdiff;
		if (!mpd_qshiftl(result, a, shift, status)) {
			return;
		}
		result->exp = b_exp;
	}
	else {
		/* At this point expdiff < 0 and a->digits+expdiff <= prec,
		 * so the shift before an increment will fit in prec. */
		shift = -expdiff;
		rnd = mpd_qshiftr(result, a, shift, status);
		if (rnd == MPD_UINT_MAX) {
			return;
		}
		result->exp = b_exp;
		if (!_mpd_apply_round_fit(result, rnd, ctx, status)) {
			return;
		}
		workstatus |= MPD_Rounded;
		if (rnd) {
			workstatus |= MPD_Inexact;
		}
	}

	if (mpd_adjexp(result) > ctx->emax ||
	    mpd_adjexp(result) < mpd_etiny(ctx)) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}

	*status |= workstatus;
	mpd_qfinalize(result, ctx, status);
}

void
mpd_qreduce(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
            uint32_t *status)
{
	mpd_ssize_t shift, maxexp, maxshift;
	uint8_t sign_a = mpd_sign(a);

	if (mpd_isspecial(a)) {
		if (mpd_qcheck_nan(result, a, ctx, status)) {
			return;
		}
		mpd_qcopy(result, a, status);
		return;
	}

	if (!mpd_qcopy(result, a, status)) {
		return;
	}
	mpd_qfinalize(result, ctx, status);
	if (mpd_isspecial(result)) {
		return;
	}
	if (mpd_iszero(result)) {
		_settriple(result, sign_a, 0, 0);
		return;
	}

	shift = mpd_trail_zeros(result);
	maxexp = (ctx->clamp) ? mpd_etop(ctx) : ctx->emax;
	/* After the finalizing above result->exp <= maxexp. */
	maxshift = maxexp - result->exp;
	shift = (shift > maxshift) ? maxshift : shift;

	mpd_qshiftr_inplace(result, shift);
	result->exp += shift;
}

void
mpd_qrem(mpd_t *r, const mpd_t *a, const mpd_t *b, const mpd_context_t *ctx,
         uint32_t *status)
{
	MPD_NEW_STATIC(q,0,0,0,0);

	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(r, a, b, ctx, status)) {
			return;
		}
		if (mpd_isinfinite(a)) {
			mpd_seterror(r, MPD_Invalid_operation, status);
			return;
		}
		if (mpd_isinfinite(b)) {
			mpd_qcopy(r, a, status);
			mpd_qfinalize(r, ctx, status);
			return;
		}
		/* debug */
		abort(); /* GCOV_NOT_REACHED */
	}
	if (mpd_iszerocoeff(b)) {
		if (mpd_iszerocoeff(a)) {
			mpd_seterror(r, MPD_Division_undefined, status);
		}
		else {
			mpd_seterror(r, MPD_Invalid_operation, status);
		}
		return;
	}

	_mpd_qdivmod(&q, r, a, b, ctx, status);
	mpd_del(&q);
	mpd_qfinalize(r, ctx, status);
}

void
mpd_qrem_near(mpd_t *r, const mpd_t *a, const mpd_t *b,
              const mpd_context_t *ctx, uint32_t *status)
{
	mpd_context_t workctx;
	MPD_NEW_STATIC(btmp,0,0,0,0);
	MPD_NEW_STATIC(q,0,0,0,0);
	mpd_ssize_t expdiff, floordigits;
	int cmp, isodd, allnine;

	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(r, a, b, ctx, status)) {
			return;
		}
		if (mpd_isinfinite(a)) {
			mpd_seterror(r, MPD_Invalid_operation, status);
			return;
		}
		if (mpd_isinfinite(b)) {
			mpd_qcopy(r, a, status);
			mpd_qfinalize(r, ctx, status);
			return;
		}
		/* debug */
		abort(); /* GCOV_NOT_REACHED */
	}
	if (mpd_iszerocoeff(b)) {
		if (mpd_iszerocoeff(a)) {
			mpd_seterror(r,  MPD_Division_undefined, status);
		}
		else {
			mpd_seterror(r,  MPD_Invalid_operation, status);
		}
		return;
	}

	if (r == b) {
		if (!mpd_qcopy(&btmp, b, status)) {
			mpd_seterror(r, MPD_Malloc_error, status);
			return;
		}
		b = &btmp;
	}

	workctx = *ctx;
	workctx.prec = a->digits;
	workctx.prec = (workctx.prec > ctx->prec) ? workctx.prec : ctx->prec;

	_mpd_qdivmod(&q, r, a, b, &workctx, status);
	if (mpd_isnan(&q) || mpd_isnan(r) || q.digits > ctx->prec) {
		mpd_seterror(r, MPD_Division_impossible, status);
		goto finish;
	}
	if (mpd_iszerocoeff(r)) {
		goto finish;
	}

	/* Deal with cases like rmnx078:
	 * remaindernear 999999999.5 1 -> NaN Division_impossible */
	expdiff = mpd_adjexp(b) - mpd_adjexp(r);
	if (-1 <= expdiff && expdiff <= 1) {

		mpd_qtrunc(&q, &q, &workctx, &workctx.status);
		allnine = mpd_coeff_isallnine(&q);
		floordigits = q.digits;
		isodd = mpd_isodd(&q);

		mpd_maxcontext(&workctx);
		if (mpd_sign(a) == mpd_sign(b)) {
			_mpd_qsub(&q, r, b, &workctx, &workctx.status);
			if (workctx.status&MPD_Errors) {
				mpd_seterror(r, workctx.status&MPD_Errors, status);
				goto finish;
			}
		}
		else {
			_mpd_qadd(&q, r, b, &workctx, &workctx.status);
			if (workctx.status&MPD_Errors) {
				mpd_seterror(r, workctx.status&MPD_Errors, status);
				goto finish;
			}
		}

		cmp = mpd_cmp_total_mag(&q, r);
		if (cmp < 0 || (cmp == 0 && isodd)) {
			if (allnine && floordigits == ctx->prec) {
				mpd_seterror(r, MPD_Division_impossible, status);
				goto finish;
			}
			mpd_qcopy(r, &q, status);
			*status &= ~MPD_Rounded;
		}
	}


finish:
	mpd_del(&btmp);
	mpd_del(&q);
	mpd_qfinalize(r, ctx, status);
}

/*
 * Rescale a number so that it has exponent 'exp'. Does not regard
 * context precision, emax, emin, but uses the rounding mode.
 * Special numbers are quietly copied.
 */
void
mpd_qrescale(mpd_t *result, const mpd_t *a, mpd_ssize_t exp,
             const mpd_context_t *ctx, uint32_t *status)
{
	mpd_ssize_t expdiff, shift;
	mpd_uint_t rnd;

	if (mpd_isspecial(a)) {
		mpd_qcopy(result, a, status);
		return;
	}

	if (exp > MPD_MAX_EMAX || exp < MPD_MIN_ETINY) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}

	if (mpd_iszero(a)) {
		_settriple(result, mpd_sign(a), 0, exp);
		return;
	}

	expdiff = a->exp - exp;
	if (expdiff >= 0) {
		shift = expdiff;
		if (!mpd_qshiftl(result, a, shift, status)) {
			return;
		}
		result->exp = exp;
	}
	else {
		shift = -expdiff;
		rnd = mpd_qshiftr(result, a, shift, status);
		if (rnd == MPD_UINT_MAX) {
			return;
		}
		result->exp = exp;
		_mpd_apply_round_excess(result, rnd, ctx, status);
		*status |= MPD_Rounded;
		if (rnd) {
			*status |= MPD_Inexact;
		}
	}

	if (mpd_issubnormal(result, ctx)) {
		*status |= MPD_Subnormal;
	}
}

/* Round to an integer according to 'action' and ctx->round. */
enum {TO_INT_EXACT, TO_INT_SILENT, TO_INT_TRUNC, TO_INT_FLOOR, TO_INT_CEIL};
static void
_mpd_qround_to_integral(int action, mpd_t *result, const mpd_t *a,
                        const mpd_context_t *ctx, uint32_t *status)
{
	mpd_uint_t rnd;

	if (mpd_isspecial(a)) {
		if (mpd_qcheck_nan(result, a, ctx, status)) {
			return;
		}
		mpd_qcopy(result, a, status);
		return;
	}
	if (a->exp >= 0) {
		mpd_qcopy(result, a, status);
		return;
	}
	if (mpd_iszerocoeff(a)) {
		_settriple(result, mpd_sign(a), 0, 0);
		return;
	}

	rnd = mpd_qshiftr(result, a, -a->exp, status);
	if (rnd == MPD_UINT_MAX) {
		return;
	}
	result->exp = 0;

	if (action == TO_INT_EXACT || action == TO_INT_SILENT) {
		_mpd_apply_round_excess(result, rnd, ctx, status);
		if (action == TO_INT_EXACT) {
			*status |= MPD_Rounded;
			if (rnd) {
				*status |= MPD_Inexact;
			}
		}
	}
	else if (action == TO_INT_FLOOR) {
		if (rnd && mpd_isnegative(result)) {
			_mpd_qsub(result, result, &one, ctx, status);
		}
	}
	else if (action == TO_INT_CEIL) {
		if (rnd && mpd_ispositive(result)) {
			_mpd_qadd(result, result, &one, ctx, status);
		}
	}
}

void
mpd_qround_to_intx(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
                   uint32_t *status)
{
	(void)_mpd_qround_to_integral(TO_INT_EXACT, result, a, ctx, status);
}

void
mpd_qround_to_int(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
                  uint32_t *status)
{
	(void)_mpd_qround_to_integral(TO_INT_SILENT, result, a, ctx, status);
}

void
mpd_qtrunc(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
              uint32_t *status)
{
	(void)_mpd_qround_to_integral(TO_INT_TRUNC, result, a, ctx, status);
}

void
mpd_qfloor(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
           uint32_t *status)
{
	(void)_mpd_qround_to_integral(TO_INT_FLOOR, result, a, ctx, status);
}

void
mpd_qceil(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
          uint32_t *status)
{
	(void)_mpd_qround_to_integral(TO_INT_CEIL, result, a, ctx, status);
}

int
mpd_same_quantum(const mpd_t *a, const mpd_t *b)
{
	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		return ((mpd_isnan(a) && mpd_isnan(b)) ||
			(mpd_isinfinite(a) && mpd_isinfinite(b)));
	}

	return a->exp == b->exp;
}

/* Schedule the increase in precision for the Newton iteration. */
static inline int
recpr_schedule_prec(mpd_ssize_t klist[MPD_MAX_PREC_LOG2],
                    mpd_ssize_t maxprec, mpd_ssize_t initprec)
{
	mpd_ssize_t k;
	int i;

	assert(maxprec > 0 && initprec > 0);
	if (maxprec <= initprec) return -1;

	i = 0; k = maxprec;
	do {
		k = (k+1) / 2;
		klist[i++] = k;
	} while (k > initprec);

	return i-1;
}

/*
 * Initial approximation for the reciprocal. Result has MPD_RDIGITS-2
 * significant digits.
 */
static void
_mpd_qreciprocal_approx(mpd_t *z, const mpd_t *v, uint32_t *status)
{
	mpd_uint_t p10data[2] = {0, mpd_pow10[MPD_RDIGITS-2]}; /* 10**(2*MPD_RDIGITS-2) */
	mpd_uint_t dummy, word;
	int n;

	_mpd_get_msdigits(&dummy, &word, v, MPD_RDIGITS);
	n = mpd_word_digits(word);
	word *= mpd_pow10[MPD_RDIGITS-n];

	mpd_qresize(z, 2, status);
	(void)_mpd_shortdiv(z->data, p10data, 2, word);

	mpd_clear_flags(z);
	z->exp = -(v->exp + v->digits) - (MPD_RDIGITS-2);
	z->len = (z->data[1] == 0) ? 1 : 2;
	mpd_setdigits(z);
}

/* Reciprocal, calculated with Newton's Method */
static void
_mpd_qreciprocal(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
                 uint32_t *status)
{
	mpd_context_t varcontext, maxcontext;
	mpd_t *z = result;         /* current approximation */
	mpd_t *v;                  /* a, normalized to a number between 0.1 and 1 */
	MPD_NEW_SHARED(vtmp, a);   /* by default v will share data with a */
	MPD_NEW_STATIC(s,0,0,0,0); /* temporary variable */
	MPD_NEW_STATIC(t,0,0,0,0); /* temporary variable */
	MPD_NEW_CONST(two,0,0,1,1,1,2); /* const 2 */
	mpd_ssize_t klist[MPD_MAX_PREC_LOG2];
	mpd_ssize_t adj, maxprec, initprec;
	uint8_t sign = mpd_sign(a);
	int i;

	v = &vtmp;
	if (result == a) {
		if ((v = mpd_qncopy(a)) == NULL) { /* GCOV_NOT_REACHED */
			mpd_seterror(result, MPD_Malloc_error, status); /* GCOV_NOT_REACHED */
			goto finish; /* GCOV_NOT_REACHED */
		}
	}

	mpd_clear_flags(v);
	adj = v->digits + v->exp;
	v->exp = -v->digits;

	/* initial approximation */
	_mpd_qreciprocal_approx(z, v, status);

	mpd_maxcontext(&varcontext);
	mpd_maxcontext(&maxcontext);
	varcontext.round = MPD_ROUND_TRUNC;
	maxcontext.round = MPD_ROUND_TRUNC;

	maxprec = (v->digits > ctx->prec) ? v->digits : ctx->prec;
	maxprec += 2;
	initprec = MPD_RDIGITS-3;

	i = recpr_schedule_prec(klist, maxprec, initprec);
	for (; i >= 0; i--) {
		mpd_qmul(&s, z, z, &maxcontext, status);
		varcontext.prec = 2*klist[i] + 5;
		if (v->digits > varcontext.prec) {
			mpd_qshiftr(&t, v, v->digits-varcontext.prec, status);
			t.exp = -varcontext.prec;
			mpd_qmul(&t, &t, &s, &varcontext, status);
		}
		else {
			mpd_qmul(&t, v, &s, &varcontext, status);
		}
		mpd_qmul(&s, z, &two, &maxcontext, status);
		mpd_qsub(z, &s, &t, &maxcontext, status);
	}

	if (!mpd_isspecial(z)) {
		z->exp -= adj;
		mpd_set_flags(z, sign);
	}

finish:
	mpd_del(&s);
	mpd_del(&t);
	if (v != &vtmp) mpd_del(v);
	mpd_qfinalize(z, ctx, status);
}

/*
 * Integer division with remainder of the coefficients: coeff(a) / coeff(b).
 * This function is for large numbers where it is faster to divide by
 * multiplying the dividend by the reciprocal of the divisor.
 * The inexact result is fixed by a small loop, which should not take
 * more than 2 iterations.
 */
static void
_mpd_qbarrett_divmod(mpd_t *q, mpd_t *r, const mpd_t *a, const mpd_t *b,
                     uint32_t *status)
{
	mpd_context_t workctx;
	mpd_t *qq = q, *rr = r;
	mpd_t aa, bb;
	int k;

	mpd_maxcontext(&workctx);
	_mpd_copy_shared(&aa, a);
	_mpd_copy_shared(&bb, b);

	mpd_set_positive(&aa);
	mpd_set_positive(&bb);
	aa.exp = 0;
	bb.exp = 0;

	if (q == a || q == b) {
		if ((qq = mpd_qnew()) == NULL) {
			*status |= MPD_Malloc_error;
			goto nanresult;
		}
	}
	if (r == a || r == b) {
		if ((rr = mpd_qnew()) == NULL) {
			*status |= MPD_Malloc_error;
			goto nanresult;
		}
	}

	/* maximum length of q + 3 digits */
	workctx.prec = aa.digits - bb.digits + 1 + 3;
	/* we get the reciprocal with precision maxlen(q) + 3 */
	_mpd_qreciprocal(rr, &bb, &workctx, &workctx.status);

	mpd_qmul(qq, &aa, rr, &workctx, &workctx.status);
	mpd_qtrunc(qq, qq, &workctx, &workctx.status);

	workctx.prec = aa.digits + 3;
	/* get the remainder */
	mpd_qmul(rr, &bb, qq, &workctx, &workctx.status);
	mpd_qsub(rr, &aa, rr, &workctx, &workctx.status);

	/* Fix the result. Algorithm from: Karl Hasselstrom, Fast Division of Large Integers */
	for (k = 0;; k++) {
		if (mpd_isspecial(rr)) {
			*status |= (workctx.status&MPD_Errors);
			goto nanresult;
		}
		if (k > 2) {
			mpd_err_warn("_mpd_barrett_divmod: k > 2 in correcting loop"); /* GCOV_NOT_REACHED */
			abort(); /* GCOV_NOT_REACHED */
		}
		else if (_mpd_cmp(&zero, rr) == 1) {
			mpd_qadd(rr, rr, &bb, &workctx, &workctx.status);
			mpd_qadd(qq, qq, &minus_one, &workctx, &workctx.status);
		}
		else if (_mpd_cmp(rr, &bb) == -1) {
			break;
		}
		else {
			mpd_qsub(rr, rr, &bb, &workctx, &workctx.status);
			mpd_qadd(qq, qq, &one, &workctx, &workctx.status);
		}
	}

	if (qq != q) {
		if (!mpd_qcopy(q, qq, status)) {
			goto nanresult; /* GCOV_UNLIKELY */
		}
		mpd_del(qq);
	}
	if (rr != r) {
		if (!mpd_qcopy(r, rr, status)) {
			goto nanresult; /* GCOV_UNLIKELY */
		}
		mpd_del(rr);
	}

	*status |= (workctx.status&MPD_Errors);
	return;


nanresult:
	if (qq && qq != q) mpd_del(qq);
	if (rr && rr != r) mpd_del(rr);
	mpd_setspecial(q, MPD_POS, MPD_NAN);
	mpd_setspecial(r, MPD_POS, MPD_NAN);
}

static inline int
invroot_schedule_prec(mpd_ssize_t klist[MPD_MAX_PREC_LOG2],
                      mpd_ssize_t maxprec, mpd_ssize_t initprec)
{
	mpd_ssize_t k;
	int i;

	assert(maxprec >= 3 && initprec >= 3);
	if (maxprec <= initprec) return -1;

	i = 0; k = maxprec;
	do {
		k = (k+3) / 2;
		klist[i++] = k;
	} while (k > initprec);

	return i-1;
}

/*
 * Initial approximation for the inverse square root.
 *
 *   Input:
 *     v := 7 or 8 decimal digits with an implicit exponent of 10**-6,
 *          representing a number 1 <= x < 100.
 *
 *   Output:
 *     An approximation to 1/sqrt(v)
 */
static inline void
_invroot_init_approx(mpd_t *z, mpd_uint_t v)
{
	mpd_uint_t lo = 1000;
	mpd_uint_t hi = 10000;
	mpd_uint_t a, sq;

	assert(v >= lo*lo && v < (hi+1)*(hi+1));

	for(;;) {
		a = (lo + hi) / 2;
		sq = a * a;
		if (v >= sq) {
			if (v < sq + 2*a + 1) {
				break;
			}
			lo = a + 1;
		}
		else {
			hi = a - 1;
		}
	}

	/* At this point a/1000 is an approximation to sqrt(v). */
	mpd_minalloc(z);
	mpd_clear_flags(z);
	z->data[0] = 1000000000UL / a;
	z->len = 1;
	z->exp = -6;
	mpd_setdigits(z);
}

static void
_mpd_qinvroot(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
              uint32_t *status)
{
	uint32_t workstatus = 0;
	mpd_context_t varcontext, maxcontext;
	mpd_t *z = result;         /* current approximation */
	mpd_t *v;                  /* a, normalized to a number between 1 and 100 */
	MPD_NEW_SHARED(vtmp, a);   /* by default v will share data with a */
	MPD_NEW_STATIC(s,0,0,0,0); /* temporary variable */
	MPD_NEW_STATIC(t,0,0,0,0); /* temporary variable */
	MPD_NEW_CONST(one_half,0,-1,1,1,1,5);
	MPD_NEW_CONST(three,0,0,1,1,1,3);
	mpd_ssize_t klist[MPD_MAX_PREC_LOG2];
	mpd_ssize_t ideal_exp, shift;
	mpd_ssize_t adj, tz;
	mpd_ssize_t maxprec, fracdigits;
	mpd_uint_t x, dummy;
	int i, n;


	ideal_exp = -(a->exp - (a->exp & 1)) / 2;

	v = &vtmp;
	if (result == a) {
		if ((v = mpd_qncopy(a)) == NULL) {
			mpd_seterror(result, MPD_Malloc_error, status);
			return;
		}
	}

	/* normalize a to 1 <= v < 100 */
	if ((v->digits+v->exp) & 1) {
		fracdigits = v->digits - 1;
		v->exp = -fracdigits;
		n = (v->digits > 7) ? 7 : (int)v->digits;
		_mpd_get_msdigits(&dummy, &x, v, n);
		if (n < 7) {
			x *= mpd_pow10[7-n];
		}
	}
	else {
		fracdigits = v->digits - 2;
		v->exp = -fracdigits;
		n = (v->digits > 8) ? 8 : (int)v->digits;
		_mpd_get_msdigits(&dummy, &x, v, n);
		if (n < 8) {
			x *= mpd_pow10[8-n];
		}
	}
	adj = (a->exp-v->exp) / 2;

	/* initial approximation */
	_invroot_init_approx(z, x);

	mpd_maxcontext(&maxcontext);
	mpd_maxcontext(&varcontext);
	varcontext.round = MPD_ROUND_TRUNC;
	maxprec = ctx->prec + 2;

	i = invroot_schedule_prec(klist, maxprec, 3);
	for (; i >= 0; i--) {
		varcontext.prec = 2*klist[i]+2;
		mpd_qmul(&s, z, z, &maxcontext, &workstatus);
		if (v->digits > varcontext.prec) {
			shift = v->digits - varcontext.prec;
			mpd_qshiftr(&t, v, shift, &workstatus);
			t.exp += shift;
			mpd_qmul(&t, &t, &s, &varcontext, &workstatus);
		}
		else {
			mpd_qmul(&t, v, &s, &varcontext, &workstatus);
		}
		mpd_qsub(&t, &three, &t, &maxcontext, &workstatus);
		mpd_qmul(z, z, &t, &varcontext, &workstatus);
		mpd_qmul(z, z, &one_half, &maxcontext, &workstatus);
	}

	z->exp -= adj;

	tz = mpd_trail_zeros(result);
	shift = ideal_exp - result->exp;
	shift = (tz > shift) ? shift : tz;
	if (shift > 0) {
		mpd_qshiftr_inplace(result, shift);
		result->exp += shift;
	}


	mpd_del(&s);
	mpd_del(&t);
	if (v != &vtmp) mpd_del(v);
	*status |= (workstatus&MPD_Errors);
	varcontext = *ctx;
	varcontext.round = MPD_ROUND_HALF_EVEN;
	mpd_qfinalize(result, &varcontext, status);
}

void
mpd_qinvroot(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
             uint32_t *status)
{

	if (mpd_isspecial(a)) {
		if (mpd_qcheck_nan(result, a, ctx, status)) {
			return;
		}
		if (mpd_isnegative(a)) {
			mpd_seterror(result, MPD_Invalid_operation, status);
			return;
		}
		/* positive infinity */
		_settriple(result, MPD_POS, 0, mpd_etiny(ctx));
		*status |= MPD_Clamped;
		return;
	}
	if (mpd_iszero(a)) {
		mpd_setspecial(result, mpd_sign(a), MPD_INF);
		*status |= MPD_Division_by_zero;
		return;
	}
	if (mpd_isnegative(a)) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}

	_mpd_qinvroot(result, a, ctx, status);
}

/*
 * Ensure correct rounding. Algorithm after Hull & Abrham, "Properly Rounded
 * Variable Precision Square Root", ACM Transactions on Mathematical Software,
 * Vol. 11, No. 3.
 */
static void
_mpd_fix_sqrt(mpd_t *result, const mpd_t *a, mpd_t *tmp,
              const mpd_context_t *ctx, uint32_t *status)
{
	mpd_context_t maxctx;
	MPD_NEW_CONST(u,0,0,1,1,1,5);

	mpd_maxcontext(&maxctx);
	u.exp = u.digits - ctx->prec + result->exp - 1;

	_mpd_qsub(tmp, result, &u, &maxctx, status);
	if (*status&MPD_Errors)	goto nanresult;

	_mpd_qmul(tmp, tmp, tmp, &maxctx, status);
	if (*status&MPD_Errors)	goto nanresult;

	if (_mpd_cmp(tmp, a) == 1) {
		u.exp += 1;
		u.data[0] = 1;
		_mpd_qsub(result, result, &u, &maxctx, status);
	}
	else {
		_mpd_qadd(tmp, result, &u, &maxctx, status);
		if (*status&MPD_Errors)	goto nanresult;

		_mpd_qmul(tmp, tmp, tmp, &maxctx, status);
		if (*status&MPD_Errors)	goto nanresult;

		if (_mpd_cmp(tmp, a) == -1) {
			u.exp += 1;
			u.data[0] = 1;
			_mpd_qadd(result, result, &u, &maxctx, status);
		}
	}

	return;

nanresult:
	mpd_setspecial(result, MPD_POS, MPD_NAN);
}

void
mpd_qsqrt(mpd_t *result, const mpd_t *a, const mpd_context_t *ctx,
          uint32_t *status)
{
	uint32_t workstatus = 0;
	mpd_context_t varcontext;
	mpd_t *z = result;         /* current approximation */
	MPD_NEW_STATIC(v,0,0,0,0); /* a, normalized to a number between 1 and 10 */
	MPD_NEW_STATIC(vtmp,0,0,0,0);
	MPD_NEW_STATIC(tmp,0,0,0,0);
	mpd_ssize_t ideal_exp, shift;
	mpd_ssize_t target_prec, fracdigits;
	mpd_ssize_t a_exp, a_digits;
	mpd_ssize_t adj, tz;
	mpd_uint_t dummy, t;
	int exact = 0;


	varcontext = *ctx;
	varcontext.round = MPD_ROUND_HALF_EVEN;
	ideal_exp = (a->exp - (a->exp & 1)) / 2;

	if (mpd_isspecial(a)) {
		if (mpd_qcheck_nan(result, a, ctx, status)) {
			return;
		}
		if (mpd_isnegative(a)) {
			mpd_seterror(result, MPD_Invalid_operation, status);
			return;
		}
		mpd_setspecial(result, MPD_POS, MPD_INF);
		return;
	}
	if (mpd_iszero(a)) {
		_settriple(result, mpd_sign(a), 0, ideal_exp);
		mpd_qfinalize(result, ctx, status);
		return;
	}
	if (mpd_isnegative(a)) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}

	if (!mpd_qcopy(&v, a, status)) {
		mpd_seterror(result, MPD_Malloc_error, status);
		goto finish;
	}

	a_exp = a->exp;
	a_digits = a->digits;

	/* normalize a to 1 <= v < 100 */
	if ((v.digits+v.exp) & 1) {
		fracdigits = v.digits - 1;
		v.exp = -fracdigits;
		_mpd_get_msdigits(&dummy, &t, &v, 3);
		t = t < 100 ? t*10 : t;
		t = t < 100 ? t*10 : t;
	}
	else {
		fracdigits = v.digits - 2;
		v.exp = -fracdigits;
		_mpd_get_msdigits(&dummy, &t, &v, 4);
		t = t < 1000 ? t*10 : t;
		t = t < 1000 ? t*10 : t;
		t = t < 1000 ? t*10 : t;
	}
	adj = (a_exp-v.exp) / 2;


	/* use excess digits */
	target_prec = (a_digits > ctx->prec) ? a_digits : ctx->prec;
	target_prec += 2;
	varcontext.prec = target_prec + 3;

	/* invroot is much faster for large numbers */
	_mpd_qinvroot(&tmp, &v, &varcontext, &workstatus);

	varcontext.prec = target_prec;
	_mpd_qdiv(NO_IDEAL_EXP, z, &one, &tmp, &varcontext, &workstatus);


	tz = mpd_trail_zeros(result);
	if ((result->digits-tz)*2-1 <= v.digits) {
		_mpd_qmul(&tmp, result, result, &varcontext, &workstatus);
		if (workstatus&MPD_Errors) {
			mpd_seterror(result, workstatus&MPD_Errors, status);
			goto finish;
		}
		exact = (_mpd_cmp(&tmp, &v) == 0);
	}
	*status |= (workstatus&MPD_Errors);

	if (!exact && !mpd_isspecial(result) && !mpd_iszero(result)) {
		_mpd_fix_sqrt(result, &v, &tmp, &varcontext, status);
		if (mpd_isspecial(result)) goto finish;
		*status |= (MPD_Rounded|MPD_Inexact);
	}

	result->exp += adj;
	if (exact) {
		shift = ideal_exp - result->exp;
		shift = (tz > shift) ? shift : tz;
		if (shift > 0) {
			mpd_qshiftr_inplace(result, shift);
			result->exp += shift;
		}
	}


finish:
	mpd_del(&v);
	mpd_del(&vtmp);
	mpd_del(&tmp);
	varcontext.prec = ctx->prec;
	mpd_qfinalize(result, &varcontext, status);
}


/******************************************************************************/
/*                              Base conversions                              */
/******************************************************************************/

/*
 * Returns the space needed to represent an integer mpd_t in base 'base'.
 * The result is undefined for non-integers.
 *
 * Max space needed:
 *
 *   base^n >= 10^(digits+exp)
 *   n >= log10(10^(digits+exp))/log10(base) = (digits+exp) / log10(base)
 */
size_t
mpd_sizeinbase(mpd_t *a, uint32_t base)
{
	size_t x;

	assert(mpd_isinteger(a));
	if (mpd_iszero(a)) {
		return 1;
	}

	x = a->digits+a->exp;

#ifdef CONFIG_64
  #ifdef USE_80BIT_LONG_DOUBLE
	return (long double)x / log10(base) + 3;
  #else
	/* x > floor(((1ULL<<53)-3) * log10(2)) */
	if (x > 2711437152599294ULL) {
		return SIZE_MAX;
	}
	return (double)x / log10(base) + 3;
  #endif
#else /* CONFIG_32 */
{
	double y =  x / log10(base) + 3;
	return (y > SIZE_MAX) ? SIZE_MAX : (size_t)y;
}
#endif
}

/*
 * Returns the space needed to import a base 'base' integer of length 'srclen'.
 */
static inline mpd_ssize_t
_mpd_importsize(size_t srclen, uint32_t base)
{
#if SIZE_MAX == UINT64_MAX
  #ifdef USE_80BIT_LONG_DOUBLE
	long double x = (long double)srclen * (log10(base)/MPD_RDIGITS) + 3;
  #else
	double x;
	if (srclen > (1ULL<<53)) {
		return MPD_SSIZE_MAX;
	}
	x = (double)srclen * (log10(base)/MPD_RDIGITS) + 3;
  #endif
#else
	double x = srclen * (log10(base)/MPD_RDIGITS) + 3;
#endif
	return (x > MPD_MAXIMPORT) ? MPD_SSIZE_MAX : (mpd_ssize_t)x;
}


static inline size_t
_to_base_u16(uint16_t *w, size_t wlen, mpd_uint_t wbase,
             mpd_uint_t *u, mpd_ssize_t ulen)
{
	size_t n = 0;

	assert(wlen > 0 && ulen > 0);

	do {
		w[n++] = (uint16_t)_mpd_shortdiv(u, u, ulen, wbase);
		/* ulen will be at least 1. u[ulen-1] can only be zero if ulen == 1 */
		ulen = _mpd_real_size(u, ulen);

	} while (u[ulen-1] != 0 && n < wlen);

	/* proper termination condition */
	assert(u[ulen-1] == 0);

	return n;
}

static inline void
_from_base_u16(mpd_uint_t *w, mpd_ssize_t wlen,
               const mpd_uint_t *u, size_t ulen, uint32_t ubase)
{
	mpd_ssize_t m = 1;
	mpd_uint_t carry;

	assert(wlen > 0 && ulen > 0);

	w[0] = u[--ulen];
	while (--ulen != SIZE_MAX && m < wlen) {
		_mpd_shortmul(w, w, m, ubase);
		m = _mpd_real_size(w, m+1);
		carry = _mpd_shortadd(w, m, u[ulen]);
		if (carry) w[m++] = carry;
	}

	/* proper termination condition */
	assert(ulen == SIZE_MAX);
}

/* target base wbase <= source base ubase */
static inline size_t
_baseconv_to_smaller(uint32_t *w, size_t wlen, mpd_uint_t wbase,
                     mpd_uint_t *u, mpd_ssize_t ulen, mpd_uint_t ubase)
{
	size_t n = 0;

	assert(wlen > 0 && ulen > 0);

	do {
		w[n++] = (uint32_t)_mpd_shortdiv_b(u, u, ulen, wbase, ubase);
		/* ulen will be at least 1. u[ulen-1] can only be zero if ulen == 1 */
		ulen = _mpd_real_size(u, ulen);

	} while (u[ulen-1] != 0 && n < wlen);

	/* proper termination condition */
	assert(u[ulen-1] == 0);

	return n;
}

/* target base wbase >= source base ubase */
static inline void
_baseconv_to_larger(mpd_uint_t *w, mpd_ssize_t wlen, mpd_uint_t wbase,
                    const mpd_uint_t *u, size_t ulen, mpd_uint_t ubase)
{
	mpd_ssize_t m = 1;
	mpd_uint_t carry;

	assert(wlen > 0 && ulen > 0);

	w[0] = u[--ulen];
	while (--ulen != SIZE_MAX && m < wlen) {
		_mpd_shortmul_b(w, w, m, ubase, wbase);
		m = _mpd_real_size(w, m+1);
		carry = _mpd_shortadd_b(w, m, u[ulen], wbase);
		if (carry) w[m++] = carry;
	}

	/* proper termination condition */
	assert(ulen == SIZE_MAX);
}


/*
 * Converts an integer mpd_t to a multiprecision integer with
 * base <= UINT16_MAX+1. The least significant word of the result
 * is rdata[0].
 */
size_t
mpd_qexport_u16(uint16_t *rdata, size_t rlen, uint32_t rbase,
                const mpd_t *src, uint32_t *status)
{
	mpd_t *tsrc;
	size_t n;

	assert(rbase <= (1U<<16));
	assert(rlen <= SIZE_MAX/(sizeof *rdata));

	if (mpd_isspecial(src) || !_mpd_isint(src)) {
		*status |= MPD_Invalid_operation;
		return SIZE_MAX;
	}

	memset(rdata, 0, rlen * (sizeof *rdata));

	if (mpd_iszero(src)) {
		return 1;
	}

	if ((tsrc = mpd_qnew()) == NULL) {
		*status |= MPD_Malloc_error;
		return SIZE_MAX;
	}

	if (src->exp >= 0) {
		if (!mpd_qshiftl(tsrc, src, src->exp, status)) {
			mpd_del(tsrc);
			return SIZE_MAX;
		}
	}
	else {
		if (mpd_qshiftr(tsrc, src, -src->exp, status) == MPD_UINT_MAX) {
			mpd_del(tsrc);
			return SIZE_MAX;
		}
	}

	n = _to_base_u16(rdata, rlen, rbase, tsrc->data, tsrc->len);

	mpd_del(tsrc);
	return n;
}

/*
 * Converts an integer mpd_t to a multiprecision integer with
 * base <= UINT32_MAX. The least significant word of the result
 * is rdata[0].
 */
size_t
mpd_qexport_u32(uint32_t *rdata, size_t rlen, uint32_t rbase,
                const mpd_t *src, uint32_t *status)
{
	mpd_t *tsrc;
	size_t n;

	if (mpd_isspecial(src) || !_mpd_isint(src)) {
		*status |= MPD_Invalid_operation;
		return SIZE_MAX;
	}
#if MPD_SIZE_MAX < SIZE_MAX
	if (rlen > MPD_SSIZE_MAX) {
		*status |= MPD_Invalid_operation;
		return SIZE_MAX;
	}
#endif

	assert(rlen <= SIZE_MAX/(sizeof *rdata));
	memset(rdata, 0, rlen * (sizeof *rdata));

	if (mpd_iszero(src)) {
		return 1;
	}

	if ((tsrc = mpd_qnew()) == NULL) {
		*status |= MPD_Malloc_error;
		return SIZE_MAX;
	}

	if (src->exp >= 0) {
		if (!mpd_qshiftl(tsrc, src, src->exp, status)) {
			mpd_del(tsrc);
			return SIZE_MAX;
		}
	}
	else {
		if (mpd_qshiftr(tsrc, src, -src->exp, status) == MPD_UINT_MAX) {
			mpd_del(tsrc);
			return SIZE_MAX;
		}
	}

#ifdef CONFIG_64
	n = _baseconv_to_smaller(rdata, rlen, rbase,
	                         tsrc->data, tsrc->len, MPD_RADIX);
#else
	if (rbase <= MPD_RADIX) {
		n = _baseconv_to_smaller(rdata, rlen, rbase,
		                         tsrc->data, tsrc->len, MPD_RADIX);
	}
	else {
		_baseconv_to_larger(rdata, (mpd_ssize_t)rlen, rbase,
		                    tsrc->data, tsrc->len, MPD_RADIX);
		n = _mpd_real_size(rdata, (mpd_ssize_t)rlen);
	}
#endif

	mpd_del(tsrc);
	return n;
}


/*
 * Converts a multiprecision integer with base <= UINT16_MAX+1 to an mpd_t.
 * The least significant word of the source is srcdata[0].
 */
void
mpd_qimport_u16(mpd_t *result,
                const uint16_t *srcdata, size_t srclen,
                uint8_t srcsign, uint32_t srcbase,
                const mpd_context_t *ctx, uint32_t *status)
{
	mpd_uint_t *usrc; /* uint16_t src copied to an mpd_uint_t array */
	mpd_ssize_t rlen; /* length of the result */
	size_t n = 0;

	assert(srclen > 0);
	assert(srcbase <= (1U<<16));

	if ((rlen = _mpd_importsize(srclen, srcbase)) == MPD_SSIZE_MAX) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	if (srclen > MPD_SIZE_MAX/(sizeof *usrc)) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	if ((usrc = mpd_alloc((mpd_size_t)srclen, sizeof *usrc)) == NULL) {
		mpd_seterror(result, MPD_Malloc_error, status);
		return;
	}
	for (n = 0; n < srclen; n++) {
		usrc[n] = srcdata[n];
	}

	/* result->data is initialized to zero */
	if (!mpd_qresize_zero(result, rlen, status)) {
		goto finish;
	}

	_from_base_u16(result->data, rlen, usrc, srclen, srcbase);

	mpd_set_flags(result, srcsign);
	result->exp = 0;
	result->len = _mpd_real_size(result->data, rlen);
	mpd_setdigits(result);

	mpd_qresize(result, result->len, status);
	mpd_qfinalize(result, ctx, status);


finish:
	mpd_free(usrc);
}

/*
 * Converts a multiprecision integer with base <= UINT32_MAX to an mpd_t.
 * The least significant word of the source is srcdata[0].
 */
void
mpd_qimport_u32(mpd_t *result,
                const uint32_t *srcdata, size_t srclen,
                uint8_t srcsign, uint32_t srcbase,
                const mpd_context_t *ctx, uint32_t *status)
{
	mpd_uint_t *usrc; /* uint32_t src copied to an mpd_uint_t array */
	mpd_ssize_t rlen; /* length of the result */
	size_t n = 0;

	assert(srclen > 0);

	if ((rlen = _mpd_importsize(srclen, srcbase)) == MPD_SSIZE_MAX) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	if (srclen > MPD_SIZE_MAX/(sizeof *usrc)) {
		mpd_seterror(result, MPD_Invalid_operation, status);
		return;
	}
	if ((usrc = mpd_alloc((mpd_size_t)srclen, sizeof *usrc)) == NULL) {
		mpd_seterror(result, MPD_Malloc_error, status);
		return;
	}
	for (n = 0; n < srclen; n++) {
		usrc[n] = srcdata[n];
	}

	/* result->data is initialized to zero */
	if (!mpd_qresize_zero(result, rlen, status)) {
		goto finish;
	}

#ifdef CONFIG_64
	_baseconv_to_larger(result->data, rlen, MPD_RADIX,
	                    usrc, srclen, srcbase);
#else
	if (srcbase <= MPD_RADIX) {
		_baseconv_to_larger(result->data, rlen, MPD_RADIX,
		                    usrc, srclen, srcbase);
	}
	else {
		_baseconv_to_smaller(result->data, rlen, MPD_RADIX,
		                     usrc, (mpd_ssize_t)srclen, srcbase);
	}
#endif

	mpd_set_flags(result, srcsign);
	result->exp = 0;
	result->len = _mpd_real_size(result->data, rlen);
	mpd_setdigits(result);

	mpd_qresize(result, result->len, status);
	mpd_qfinalize(result, ctx, status);


finish:
	mpd_free(usrc);
}


/*********************************************************************/
/*                   Testcases for Newton Division                   */
/*********************************************************************/

static void
mpd_qtest_newtondiv(mpd_t *q, const mpd_t *a, const mpd_t *b,
                    const mpd_context_t *ctx, uint32_t *status)
{
	MPD_NEW_STATIC(aligned,0,0,0,0);
	mpd_uint_t ld;
	mpd_ssize_t shift, exp, tz;
	mpd_ssize_t newsize;
	mpd_uint_t rem;
	uint8_t sign_a = mpd_sign(a);
	uint8_t sign_b = mpd_sign(b);


	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(q, a, b, ctx, status)) {
			return;
		}
		_mpd_qdiv_inf(q, a, b, ctx, status);
		return;
	}
	if (mpd_iszerocoeff(b)) {
		if (mpd_iszerocoeff(a)) {
			mpd_seterror(q, MPD_Division_undefined, status);
		}
		else {
			mpd_setspecial(q, sign_a^sign_b, MPD_INF);
			*status |= MPD_Division_by_zero;
		}
		return;
	}
	if (mpd_iszerocoeff(a)) {
		exp = a->exp - b->exp;
		_settriple(q, sign_a^sign_b, 0, exp);
		mpd_qfinalize(q, ctx, status);
		return;
	}

	shift = (b->digits - a->digits) + ctx->prec + 1;
	/* exp = ideal_exp - shift */
	exp = (a->exp - b->exp) - shift;
	if (shift > 0) {
		if (!mpd_qshiftl(&aligned, a, shift, status)) {
			mpd_seterror(q, MPD_Malloc_error, status);
			goto finish;
		}
		a = &aligned;
	}
	else if (shift < 0) {
		shift = -shift;
		if (!mpd_qshiftl(&aligned, b, shift, status)) {
			mpd_seterror(q, MPD_Malloc_error, status);
			goto finish;
		}
		b = &aligned;
	}


	newsize = a->len - b->len + 1;
	if ((q != b && q != a) || (q == b && newsize > b->len)) {
		if (!mpd_qresize(q, newsize, status)) {
			mpd_seterror(q, MPD_Malloc_error, status);
			goto finish;
		}
	}


	{
		MPD_NEW_STATIC(r,0,0,0,0);
		_mpd_qbarrett_divmod(q, &r, a, b, status);
		if (mpd_isspecial(q) || mpd_isspecial(&r)) {
			mpd_del(&r);
			goto finish;
		}
		rem = !mpd_iszerocoeff(&r);
		mpd_del(&r);
		newsize = q->len;
	}


	newsize = _mpd_real_size(q->data, newsize);
	/* resize to smaller cannot fail */
	mpd_qresize(q, newsize, status);
	q->len = newsize;
	mpd_setdigits(q);

	if (rem) {
		ld = mpd_lsd(q->data[0]);
		if (ld == 0 || ld == 5) {
			q->data[0] += 1;
		}
	}
	else if (1) { /* SET_IDEAL_EXP */
		tz = mpd_trail_zeros(q);
		/* right now: shift = ideal_exp - exp */
		shift = (tz > shift) ? shift : tz;
		mpd_qshiftr_inplace(q, shift);
		exp += shift;
	}

	mpd_set_flags(q, sign_a^sign_b);
	q->exp = exp;


finish:
	mpd_del(&aligned);
	mpd_qfinalize(q, ctx, status);
}

static void
_mpd_qtest_barrett_divmod(mpd_t *q, mpd_t *r, const mpd_t *a, const mpd_t *b,
                          const mpd_context_t *ctx, uint32_t *status)
{
	MPD_NEW_STATIC(aligned,0,0,0,0);
	mpd_ssize_t qsize, rsize;
	mpd_ssize_t ideal_exp, expdiff, shift;
	uint8_t sign_a = mpd_sign(a);
	uint8_t sign_ab = mpd_sign(a)^mpd_sign(b);


	ideal_exp = (a->exp > b->exp) ?  b->exp : a->exp;
	if (mpd_iszerocoeff(a)) {
		if (!mpd_qcopy(r, a, status)) {
			goto nanresult; /* GCOV_NOT_REACHED */
		}
		r->exp = ideal_exp;
		_settriple(q, sign_ab, 0, 0);
		return;
	}

	expdiff = mpd_adjexp(a) - mpd_adjexp(b);
	if (expdiff < 0) {
		if (a->exp > b->exp) {
			/* positive and less than b->digits - a->digits */
			shift = a->exp - b->exp;
			if (!mpd_qshiftl(r, a, shift, status)) {
				goto nanresult;
			}
			r->exp = ideal_exp;
		}
		else {
			if (!mpd_qcopy(r, a, status)) {
				goto nanresult;
			}
		}
		_settriple(q, sign_ab, 0, 0);
		return;
	}
	if (expdiff > ctx->prec) {
		*status |= MPD_Division_impossible;
		goto nanresult;
	}


	/*
	 * At this point we have:
	 *   (1) 0 <= a->exp + a->digits - b->exp - b->digits <= prec
	 *   (2) a->exp - b->exp >= b->digits - a->digits
	 *   (3) a->exp - b->exp <= prec + b->digits - a->digits
	 */
	if (a->exp != b->exp) {
		shift = a->exp - b->exp;
		if (shift > 0) {
			/* by (3), after the shift a->digits <= prec + b->digits */
			if (!mpd_qshiftl(&aligned, a, shift, status)) {
				goto nanresult;
			}
			a = &aligned;
		}
		else  {
			shift = -shift;
			/* by (2), after the shift b->digits <= a->digits */
			if (!mpd_qshiftl(&aligned, b, shift, status)) {
				goto nanresult;
			}
			b = &aligned;
		}
	}


	qsize = a->len - b->len + 1;
	if (!(q == a && qsize < a->len) && !(q == b && qsize < b->len)) {
		if (!mpd_qresize(q, qsize, status)) {
			goto nanresult;
		}
	}

	rsize = b->len;
	if (!(r == a && rsize < a->len)) {
		if (!mpd_qresize(r, rsize, status)) {
			goto nanresult;
		}
	}

	_mpd_qbarrett_divmod(q, r, a, b, status);
	if (mpd_isspecial(q) || mpd_isspecial(r)) {
		goto nanresult;
	}
	if (mpd_isinfinite(q) || q->digits > ctx->prec) {
		*status |= MPD_Division_impossible;
		goto nanresult;
	}
	qsize = q->len;
	rsize = r->len;

	qsize = _mpd_real_size(q->data, qsize);
	/* resize to smaller cannot fail */
	mpd_qresize(q, qsize, status);
	q->len = qsize;
	mpd_setdigits(q);
	mpd_set_flags(q, sign_ab);
	q->exp = 0;
	if (q->digits > ctx->prec) {
		*status |= MPD_Division_impossible; /* GCOV_NOT_REACHED */
		goto nanresult; /* GCOV_NOT_REACHED */
	}

	rsize = _mpd_real_size(r->data, rsize);
	/* resize to smaller cannot fail */
	mpd_qresize(r, rsize, status);
	r->len = rsize;
	mpd_setdigits(r);
	mpd_set_flags(r, sign_a);
	r->exp = ideal_exp;

out:
	mpd_del(&aligned);
	return;

nanresult:
	mpd_setspecial(q, MPD_POS, MPD_NAN);
	mpd_setspecial(r, MPD_POS, MPD_NAN);
	goto out;
}

static void
mpd_qtest_newtondivint(mpd_t *q, const mpd_t *a, const mpd_t *b,
                       const mpd_context_t *ctx, uint32_t *status)
{
	MPD_NEW_STATIC(r,0,0,0,0);
	uint8_t sign = mpd_sign(a)^mpd_sign(b);

	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(q, a, b, ctx, status)) {
			return;
		}
		if (mpd_isinfinite(a) && mpd_isinfinite(b)) {
			mpd_seterror(q, MPD_Invalid_operation, status);
			return;
		}
		if (mpd_isinfinite(a)) {
			mpd_setspecial(q, sign, MPD_INF);
			return;
		}
		if (mpd_isinfinite(b)) {
			_settriple(q, sign, 0, 0);
			return;
		}
		/* debug */
		abort(); /* GCOV_NOT_REACHED */
	}
	if (mpd_iszerocoeff(b)) {
		if (mpd_iszerocoeff(a)) {
			mpd_seterror(q, MPD_Division_undefined, status);
		}
		else {
			mpd_setspecial(q, sign, MPD_INF);
			*status |= MPD_Division_by_zero;
		}
		return;
	}

	_mpd_qtest_barrett_divmod(q, &r, a, b, ctx, status);
	mpd_del(&r);
	mpd_qfinalize(q, ctx, status);
}

static void
mpd_qtest_newtonrem(mpd_t *r, const mpd_t *a, const mpd_t *b,
                    const mpd_context_t *ctx, uint32_t *status)
{
	MPD_NEW_STATIC(q,0,0,0,0);

	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(r, a, b, ctx, status)) {
			return;
		}
		if (mpd_isinfinite(a)) {
			mpd_seterror(r, MPD_Invalid_operation, status);
			return;
		}
		if (mpd_isinfinite(b)) {
			mpd_qcopy(r, a, status);
			return;
		}
		/* debug */
		abort(); /* GCOV_NOT_REACHED */
	}
	if (mpd_iszerocoeff(b)) {
		if (mpd_iszerocoeff(a)) {
			mpd_seterror(r, MPD_Division_undefined, status);
		}
		else {
			mpd_seterror(r, MPD_Invalid_operation, status);
		}
		return;
	}

	_mpd_qtest_barrett_divmod(&q, r, a, b, ctx, status);
	mpd_del(&q);
	mpd_qfinalize(r, ctx, status);
}

static void
mpd_qtest_newtondivmod(mpd_t *q, mpd_t *r, const mpd_t *a, const mpd_t *b,
                       const mpd_context_t *ctx, uint32_t *status)
{
	uint8_t sign = mpd_sign(a)^mpd_sign(b);

	if (mpd_isspecial(a) || mpd_isspecial(b)) {
		if (mpd_qcheck_nans(q, a, b, ctx, status)) {
			mpd_qcopy(r, q, status);
			return;
		}
		if (mpd_isinfinite(a)) {
			/* decimal.py returns Inf for q if b is normal. */
			mpd_setspecial(q, MPD_POS, MPD_NAN);
			mpd_setspecial(r, MPD_POS, MPD_NAN);
			*status |= MPD_Invalid_operation;
			return;
		}
		if (mpd_isinfinite(b)) {
			if (!mpd_qcopy(r, a, status)) {
				mpd_seterror(q, MPD_Malloc_error, status);
			}
			else {
				_settriple(q, sign, 0, 0);
			}
			return;
		}
		/* debug */
		abort(); /* GCOV_NOT_REACHED */
	}
	if (mpd_iszerocoeff(b)) {
		if (mpd_iszerocoeff(a)) {
			mpd_setspecial(q, MPD_POS, MPD_NAN);
			mpd_setspecial(r, MPD_POS, MPD_NAN);
			*status |= MPD_Division_undefined;
		}
		else {
			mpd_setspecial(q, MPD_POS, MPD_NAN);
			mpd_setspecial(r, MPD_POS, MPD_NAN);
			*status |= (MPD_Division_by_zero|MPD_Invalid_operation);
		}
		return;
	}

	_mpd_qtest_barrett_divmod(q, r, a, b, ctx, status);
	mpd_qfinalize(q, ctx, status);
	mpd_qfinalize(r, ctx, status);
}

void
mpd_test_newtondiv(mpd_t *q, const mpd_t *a, const mpd_t *b, mpd_context_t *ctx)
{
	uint32_t status = 0;
	mpd_qtest_newtondiv(q, a, b, ctx, &status);
	mpd_addstatus_raise(ctx, status);
}

void
mpd_test_newtondivint(mpd_t *q, const mpd_t *a, const mpd_t *b, mpd_context_t *ctx)
{
	uint32_t status = 0;
	mpd_qtest_newtondivint(q, a, b, ctx, &status);
	mpd_addstatus_raise(ctx, status);
}

void
mpd_test_newtonrem(mpd_t *r, const mpd_t *a, const mpd_t *b, mpd_context_t *ctx)
{
	uint32_t status = 0;
	mpd_qtest_newtonrem(r, a, b, ctx, &status);
	mpd_addstatus_raise(ctx, status);
}

void
mpd_test_newtondivmod(mpd_t *q, mpd_t *r, const mpd_t *a, const mpd_t *b, mpd_context_t *ctx)
{
	uint32_t status = 0;
	mpd_qtest_newtondivmod(q, r, a, b, ctx, &status);
	mpd_addstatus_raise(ctx, status);
}


