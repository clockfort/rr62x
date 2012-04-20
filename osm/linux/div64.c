/* $Id: div64.c,v 1.3 2005/10/18 16:03:18 xxj Exp $ */

typedef unsigned long long ull;

#ifndef _MIPS_SZLONG
#define _MIPS_SZLONG 0
#endif

#if (_MIPS_SZLONG == 32) /* mips 32 */

/*
 * No traps on overflows for any of these...
 */

#define do_div64_32(res, high, low, base) ({ \
	unsigned long __quot, __mod; \
	unsigned long __cf, __tmp, __tmp2, __i; \
	\
	__asm__(".set	push\n\t" \
		".set	noat\n\t" \
		".set	noreorder\n\t" \
		"move	%2, $0\n\t" \
		"move	%3, $0\n\t" \
		"b	1f\n\t" \
		" li	%4, 0x21\n" \
		"0:\n\t" \
		"sll	$1, %0, 0x1\n\t" \
		"srl	%3, %0, 0x1f\n\t" \
		"or	%0, $1, %5\n\t" \
		"sll	%1, %1, 0x1\n\t" \
		"sll	%2, %2, 0x1\n" \
		"1:\n\t" \
		"bnez	%3, 2f\n\t" \
		" sltu	%5, %0, %z6\n\t" \
		"bnez	%5, 3f\n" \
		"2:\n\t" \
		" addiu	%4, %4, -1\n\t" \
		"subu	%0, %0, %z6\n\t" \
		"addiu	%2, %2, 1\n" \
		"3:\n\t" \
		"bnez	%4, 0b\n\t" \
		" srl	%5, %1, 0x1f\n\t" \
		".set	pop" \
		: "=&r" (__mod), "=&r" (__tmp), "=&r" (__quot), "=&r" (__cf), \
		  "=&r" (__i), "=&r" (__tmp2) \
		: "Jr" (base), "0" (high), "1" (low)); \
	\
	(res) = __quot; \
	__mod; })

#define do_div(n, base) ({ \
	unsigned long long __quot; \
	unsigned long __mod; \
	unsigned long long __div; \
	unsigned long __upper, __low, __high, __base; \
	\
	__div = (n); \
	__base = (base); \
	\
	__high = __div >> 32; \
	__low = __div; \
	__upper = __high; \
	\
	if (__high) \
		__asm__("divu	$0, %z2, %z3" \
			: "=h" (__upper), "=l" (__high) \
			: "Jr" (__high), "Jr" (__base)); \
	\
	__mod = do_div64_32(__low, __upper, __low, __base); \
	\
	__quot = __high; \
	__quot = __quot << 32 | __low; \
	(n) = __quot; \
	__mod; })

#elif (_MIPS_SZLONG == 64) /* mips 64 */

/*
 * Don't use this one in new code
 */
#define do_div64_32(res, high, low, base) ({ \
	unsigned int __quot, __mod; \
	unsigned long __div; \
	unsigned int __low, __high, __base; \
	\
	__high = (high); \
	__low = (low); \
	__div = __high; \
	__div = __div << 32 | __low; \
	__base = (base); \
	\
	__mod = __div % __base; \
	__div = __div / __base; \
	\
	__quot = __div; \
	(res) = __quot; \
	__mod; })

/*
 * Hey, we're already 64-bit, no
 * need to play games..
 */
#define do_div(n, base) ({ \
	unsigned long __quot; \
	unsigned int __mod; \
	unsigned long __div; \
	unsigned int __base; \
	\
	__div = (n); \
	__base = (base); \
	\
	__mod = __div % __base; \
	__quot = __div / __base; \
	\
	(n) = __quot; \
	__mod; })

#else /* i386 */

#define do_div(n,base) ({ \
	unsigned long __upper, __low, __high, __mod; \
	asm("":"=a" (__low), "=d" (__high):"A" (n)); \
	__upper = __high; \
	if (__high) { \
		__upper = __high % (base); \
		__high = __high / (base); \
	} \
	asm("divl %2":"=a" (__low), "=d" (__mod):"rm" (base), "0" (__low), "1" (__upper)); \
	asm("":"=A" (n):"a" (__low),"d" (__high)); \
	__mod; \
})

#endif

ull __udivdi3(ull n, ull base)
{
	do_div(n, (unsigned int)base);
	return n;
}

ull __umoddi3(ull n, ull base)
{
	return do_div(n, (unsigned int)base);
}
