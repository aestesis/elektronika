/*!  
 *	\page FIXED Fixed point math.
 *	Flash uses 16.16 fixed point math. Fixed math operations and #defines are
 *	in the (flash root)/fixed.h file.<p>
 *  If FIXEDUSEFLOAT is NOT defined,
 *	(flash root)/(platform directory)/Fixed.h is included by the fixed point code.
 *  Multiplication and division of 16.16 fixed point numbers can be defined here
 *	as a performance optimization. Plenty of examples are given.
 *  If you have a 64 bit type, implementing Fixed is near trivial - set up the HAVE64BIT
 *  and S64 defines. Other (assembly) examples for your perusal are included.
 */

#define HAVE64BIT
#define S64			__int64			// your 64-bit type goes here.
//#define S64			long long	// for the gcc compilers

#if defined(HAVE64BIT)
// 	#pragma message( "Using 64 bit fixed point" )
	inline SFIXED FixedMul(SFIXED a, SFIXED b)
	{
	  return (S32) (((S64)a * (S64)b) >> 16);
	}

	inline SFIXED FixedDiv(SFIXED a, SFIXED b)
	{
	  if (b == 0) {
		return 0;
	  }
	  return (S32) (((S64)a << 16) / (S64)b);
	}

	inline S32 _FPMul(S32 a, S32 b, int rshift)
	{
	  return (S32) (((S64)a * (S64)b) >> rshift);
	}

	inline S32 _FPDiv(S32 a, S32 b, int rshift)
	{
	  if ( b == 0 ) return a < 0 ? minS32 : maxS32;
	  return (S32) (((S64)a << rshift) / (S64) b);
	}

#elif defined(_MPPC_)
	// Routines for PowerMacs

	S32 _FPMul(S32 a, S32 b, int rshift);
	S32 _FPDiv(S32 a, S32 b, int rshift);

	// Just use the toolbox routines
	//#define FixedMul( a, b ) ((SFIXED)FixMul(((S32)(a)), ((S32)(b))))
	//#define FixedDiv( a, b ) ((SFIXED)FixDiv(((S32)(a)), ((S32)(b))))
	SFIXED FixedMul(SFIXED a, SFIXED b);
	SFIXED FixedDiv(SFIXED a, SFIXED b);

#elif defined(_MAC)
	// Routines for 68K Macs

	// Use the externally assembled versions with Mac names
	//extern "C" S32 FPRoundM(S32 a, S32 shift);
	extern "C" S32 FPMulM(S32 a, S32 b, S32 rshift);
	extern "C" S32 FPMulM16(S32 a, S32 b);
	extern "C" S32 FPDivM(S32 a, S32 b, S32 rshift);

	#define _FPMul FPMulM
	#define _FPDiv FPDivM

	#define FixedMul( a, b ) ((SFIXED)FPMulM16(((S32)(a)), ((S32)(b))))
	#define FixedDiv( a, b ) ((SFIXED)_FPDiv(((S32)(a)), ((S32)(b)), 16))

#elif defined(_MIPS_)

	S32 _FPMul( S32 a, S32 b, int rshift );
	S32 _FPDiv( S32 a, S32 b, int rshift );

	#define FixedMul( a, b ) ((SFIXED)_FPMul(((S32)(a)), ((S32)(b)), 16))
	#define FixedDiv( a, b ) ((SFIXED)_FPDiv(((S32)(a)), ((S32)(b)), 16))

#elif defined(_WIN32)

	// Routines for Win32

	inline S32 _FPDiv(S32 a, S32 b, int rshift)
	{
		S32 result;
		__asm {
			mov		eax, a			;   eax = a
			mov		ebx, b			;   ebx = b

			mov		ecx, rshift
			mov		ch, 32			;   cl = excess bits on the right
			sub		ch, cl			;   ch = excess bits on the left

			xor		di, di			;   product sign is positive

			cmp		eax, 0			;   if a < 0
			jns		short FPD322

			not		di				;     product sign toggles
			neg		eax				;     a = -a
			jns		FPD322			;     if a is still negative
			dec		eax				;       make it positive dammit

	FPD322:	cmp		ebx, 0			;   if b = 0
			jz		short FPD325	;     overflow

			jns		short FPD323	;   if b < 0

			not		di				;     product sign toggles
			neg		ebx				;     b = -b
			jns		FPD323			;     if b is still negative
			dec		ebx				;       make it positive dammit

	FPD323:

			; Check for overflow

			cdq						;   edx:eax = a

			push	eax
			div		ebx				;   edx = int( a / b )
			mov		edx, eax
			pop		eax

			xchg	cl, ch			;   cl = bits in integer part less sign
			dec		cl
			shr		edx, cl			;   edx = overflow bits
			inc		cl
			xchg	cl, ch

			or		edx, edx		;   If overflow bits contain data
			jnz		short FPD325	;     overflow

			; Perform division

			mov		edx, eax		;   edx:eax = a * 2 ** shift
			sal		eax, cl
			xchg	cl, ch
			sar		edx, cl
			xchg	cl, ch

			div		ebx				;   eax = a / b

			shl		edx, 1			;   edx = remainder * 2
			cmp		edx, ebx		;   if (remainder / b) > 0.5
			jc		short FPD326

			inc		eax				;     Round up

			cmp		eax, 0			;     If overflow
			jns		short FPD326

	FPD325:
			mov		eax, 07fffffffh	;       eax = max positve

			or		di, di			;       if quotient is negative
			jz		short FPD329

			inc		eax				;         eax = max negative

			jmp		short FPD329	;       Exit

	FPD326:	or		di, di			;   if result is negative
			jz		FPD329

			neg		eax				;     negate quotient

	FPD329:	mov		result, eax
		}
		return result;
	}

	inline S32 FixedDiv(S32 a, S32 b)
	{
		S32 result;
		__asm {
			mov		eax, a			;   eax = a
			mov		ebx, b			;   ebx = b

			mov		cx, 16			;	right shift count
			mov		ch, 32			;   cl = excess bits on the right
			sub		ch, cl			;   ch = excess bits on the left

			xor		di, di			;   product sign is positive

			cmp		eax, 0			;   if a < 0
			jns		short FPD322

			not		di				;     product sign toggles
			neg		eax				;     a = -a
			jns		FPD322			;     if a is still negative
			dec		eax				;       make it positive dammit

	FPD322:	cmp		ebx, 0			;   if b = 0
			jz		short FPD325	;     overflow

			jns		short FPD323	;   if b < 0

			not		di				;     product sign toggles
			neg		ebx				;     b = -b
			jns		FPD323			;     if b is still negative
			dec		ebx				;       make it positive dammit

	FPD323:

			; Check for overflow

			cdq						;   edx:eax = a

			push	eax
			div		ebx				;   edx = int( a / b )
			mov		edx, eax
			pop		eax

			xchg	cl, ch			;   cl = bits in integer part less sign
			dec		cl
			shr		edx, cl			;   edx = overflow bits
			inc		cl
			xchg	cl, ch

			or		edx, edx		;   If overflow bits contain data
			jnz		short FPD325	;     overflow

			; Perform division

			mov		edx, eax		;   edx:eax = a * 2 ** shift
			sal		eax, cl
			xchg	cl, ch
			sar		edx, cl
			xchg	cl, ch

			div		ebx				;   eax = a / b

			shl		edx, 1			;   edx = remainder * 2
			cmp		edx, ebx		;   if (remainder / b) > 0.5
			jc		short FPD326

			inc		eax				;     Round up

			cmp		eax, 0			;     If overflow
			jns		short FPD326

	FPD325:
			mov		eax, 07fffffffh	;       eax = max positve

			or		di, di			;       if quotient is negative
			jz		short FPD329

			inc		eax				;         eax = max negative

			jmp		short FPD329	;       Exit

	FPD326:	or		di, di			;   if result is negative
			jz		FPD329

			neg		eax				;     negate quotient

	FPD329:	mov		result, eax
		}
		return result;
	}

	// Use the inline assembly
	inline S32 _FPMul(S32 a, S32 b, int rshift)
	{
		FLASHASSERT(rshift > 0);
		S32 result;
		__asm {
			mov		eax, a			;   eax = a
			mov		ebx, b			;   ebx = b

			imul	ebx				;   edx:eax = a * b

			mov		ecx, rshift		;   cl = shift
			shrd	eax, edx, cl	;     result in eax
			adc		eax, 0			;     round up

			mov		result, eax
		}
		return result;
	}

	inline SFIXED FixedMul(SFIXED a, SFIXED b)
	{
		S32 result;
		__asm {
			mov		eax, a			;   eax = a
			mov		ebx, b			;   ebx = b

			imul	ebx				;   edx:eax = a * b
			shrd	eax, edx, 16	;     result in eax
			adc		eax, 0			;     round up

			mov		result, eax
		}
		return result;
	}

#else
	@There are no optimizations for your platform. Either 
	 write them or turn on FixedUseFloat@
#endif

inline S32 _FPRound(S32 a, int shift)
{
	if ( shift > 0 )
		return (a+(1<<(shift-1))) >> shift;
	else
		return a << -shift;
}

// Trig Routines
S32 _FPSin( S32 degrees, int inPrec, int outPrec );
S32 _FPCos( S32 degrees, int inPrec, int outPrec );
S32 _FPATan2( S32 y, S32 x, int inPrec, int outPrec );
//extern void _FPInit( S16 fpuFlag );

#define FixedSin( degree ) ((SFIXED)_FPSin(((S32)(degree)), 16, 16))
#define FixedCos( degree ) ((SFIXED)_FPCos(((S32)(degree)), 16, 16))
SFIXED FixedTan(SFIXED);
#define FixedAtan2( y, x ) ((SFIXED)_FPATan2(((S32)(y)), ((S32)(x)), 16, 16))
