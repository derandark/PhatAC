
#include "StdAfx.h"

extern void *SubTableEntry3__VTBL[5];

#define SubTableEntry2_seqnum				0
#define SubTableEntry2_encryption_key		4
#define SubTableEntry2_ptr_SubTableEntry3	8

#define SubTableEntry3_ptr_VTBL			0
#define SubTableEntry3_counter			4
#define SubTableEntry3_ptr_xortable		8
#define SubTableEntry3_ptr_unktable		12 
#define SubTableEntry3_arg0				16
#define SubTableEntry3_arg1				20
#define SubTableEntry3_arg2				24

#define SubTableEntry4_counter			0
#define SubTableEntry4_ptr_xortable		4
#define SubTableEntry4_ptr_unktable		8
#define SubTableEntry4_arg0				12
#define SubTableEntry4_arg1				16
#define SubTableEntry4_arg2				20


//check
__declspec(naked) void SubTableEntry4__Constructor(void)
{
	ULONG *xortab, *unktab;
	__asm
	{
		push	ebp
		mov		ebp, esp
		sub		esp, __LOCAL_SIZE
		push	ecx

		push	256
		push	4
		call	calloc
		mov[xortab], eax
		add		esp, 8h

		push	256
		push	4
		call	calloc
		mov[unktab], eax
		add		esp, 8h

		pop		ecx
		mov		eax, [xortab]
		mov		dword ptr[ecx + SubTableEntry4_ptr_xortable], eax
		mov		eax, [unktab]
		mov		dword ptr[ecx + SubTableEntry4_ptr_unktable], eax

		mov     esp, ebp
		pop     ebp
		ret
	}
}


//check
#if 0 
__declspec(naked) void SubTableEntry3__DESTRUCTOR(void)
{
	//void *woot;
	__asm
	{
		push	ebp
		mov		ebp, esp
		sub		esp, __LOCAL_SIZE
		push	ecx
		int		3
		call	free
		add		esp, 4h
		; mov		dword ptr[woot], ecx
	}
	//delete [] woot;
	__asm
	{
		mov     esp, ebp
		pop     ebp
		ret
	}
}
#endif



#define ptr_SubTableEntry3		-48
#define local_unk				-44
#define local_counter			-40
#define xor0					-36
#define xor1					-32
#define xor2					-28
#define xor3					-24
#define xor4					-20
#define xor5					-16
#define xor6					-12
#define local_xor				-8
#define xor7					-4
#define ptr_SubTableEntry4		8
#define bool_use_keys			12

//check
__declspec(naked) void SubTableEntry3__Fill_Out_Tables_Part2(void)
{
	__asm
	{
		push    ebp
		mov     ebp, esp
		sub     esp, 30h
		mov     dword ptr[ebp + ptr_SubTableEntry3], ecx
		mov     dword ptr[ebp + xor0], 9E3779B9h
		mov     eax, [ebp + xor0]
		mov     dword ptr[ebp + xor1], eax
		mov     ecx, [ebp + xor1]
		mov     dword ptr[ebp + xor2], ecx
		mov     edx, [ebp + xor2]
		mov     dword ptr[ebp + xor3], edx
		mov     eax, [ebp + xor3]
		mov     dword ptr[ebp + xor4], eax
		mov     ecx, [ebp + xor4]
		mov     dword ptr[ebp + xor5], ecx
		mov     edx, [ebp + xor5]
		mov     dword ptr[ebp + xor6], edx
		mov     eax, [ebp + xor6]
		mov     dword ptr[ebp + xor7], eax
		mov     ecx, [ebp + ptr_SubTableEntry4]
		mov     edx, [ecx + SubTableEntry4_ptr_unktable]
		mov     dword ptr[ebp + local_unk], edx
		mov     eax, [ebp + ptr_SubTableEntry4]
		mov     ecx, [eax + SubTableEntry4_ptr_xortable]
		mov     dword ptr[ebp + local_xor], ecx
		cmp     dword ptr[ebp + bool_use_keys], 0
		jnz     short USE_GIVEN_KEYS
		mov     edx, [ebp + ptr_SubTableEntry4]
		mov     dword ptr[edx + SubTableEntry4_arg0], 0
		mov     eax, [ebp + ptr_SubTableEntry4]
		mov     dword ptr[eax + SubTableEntry4_arg1], 0
		mov     ecx, [ebp + ptr_SubTableEntry4]
		mov     dword ptr[ecx + SubTableEntry4_arg2], 0

		USE_GIVEN_KEYS:; CODE XREF : SubTableEntry3__Fill_Out_Tables_Part2 + 50.j
					   mov     dword ptr[ebp + local_counter], 0
					   jmp     short loc_4BD862
					   ; -------------------------------------------------------------------------- -

					   loc_4BD859:; CODE XREF : SubTableEntry3__Fill_Out_Tables_Part2 + B3.j
								  mov     edx, [ebp + local_counter]
								  add     edx, 1
								  mov     dword ptr[ebp + local_counter], edx

								  loc_4BD862 : ; CODE XREF : SubTableEntry3__Fill_Out_Tables_Part2 + 77.j
											   cmp     dword ptr[ebp + local_counter], 4; for (int i = 0; i < 4; i++)
											   ; {
			;    SubTableEntry3::VTBL_func[0x10](
				; }
		jge     short loc_4BD895
			lea     eax, [ebp + xor0]
			push    eax
			lea     ecx, [ebp + xor1]
			push    ecx
			lea     edx, [ebp + xor2]
			push    edx
			lea     eax, [ebp + xor3]
			push    eax
			lea     ecx, [ebp + xor4]
			push    ecx
			lea     edx, [ebp + xor5]
			push    edx
			lea     eax, [ebp + xor6]
			push    eax
			lea     ecx, [ebp + xor7]
			push    ecx
			mov     edx, [ebp + ptr_SubTableEntry3]
			mov     eax, [edx + SubTableEntry3_ptr_VTBL]
			mov     ecx, [ebp + ptr_SubTableEntry3]
			call    dword ptr[eax + 10h]; SubTableEntry3::XOR_LOOP1
			jmp     short loc_4BD859
			; -------------------------------------------------------------------------- -

			loc_4BD895:; CODE XREF : SubTableEntry3__Fill_Out_Tables_Part2 + 86.j
			cmp     dword ptr[ebp + bool_use_keys], 0
			jz      dont_use_keys_route
			mov     dword ptr[ebp + local_counter], 0
			jmp     short loc_4BD8B1
			; -------------------------------------------------------------------------- -

			loc_4BD8A8:; CODE XREF : SubTableEntry3__Fill_Out_Tables_Part2 + 1EF.j
			mov     ecx, [ebp + local_counter]
			add     ecx, 8; note, I += 2
			mov     dword ptr[ebp + local_counter], ecx

			loc_4BD8B1 : ; CODE XREF : SubTableEntry3__Fill_Out_Tables_Part2 + C6.j
			cmp     dword ptr[ebp + local_counter], 100h
			jge     iteration_256_complete
			mov     edx, [ebp + local_counter]
			mov     eax, [ebp + local_xor]
			mov     ecx, [ebp + xor7]
			add     ecx, [eax + edx * 4]
			mov     dword ptr[ebp + xor7], ecx
			mov     edx, [ebp + local_counter]
			mov     eax, [ebp + local_xor]
			mov     ecx, [ebp + xor6]
			add     ecx, [eax + edx * 4 + 4]
			mov     dword ptr[ebp + xor6], ecx
			mov     edx, [ebp + local_counter]
			mov     eax, [ebp + local_xor]
			mov     ecx, [ebp + xor5]
			add     ecx, [eax + edx * 4 + 8]
			mov     dword ptr[ebp + xor5], ecx
			mov     edx, [ebp + local_counter]
			mov     eax, [ebp + local_xor]
			mov     ecx, [ebp + xor4]
			add     ecx, [eax + edx * 4 + 0Ch]
			mov     dword ptr[ebp + xor4], ecx
			mov     edx, [ebp + local_counter]
			mov     eax, [ebp + local_xor]
			mov     ecx, [ebp + xor3]
			add     ecx, [eax + edx * 4 + 10h]
			mov     dword ptr[ebp + xor3], ecx
			mov     edx, [ebp + local_counter]
			mov     eax, [ebp + local_xor]
			mov     ecx, [ebp + xor2]
			add     ecx, [eax + edx * 4 + 14h]
			mov     dword ptr[ebp + xor2], ecx
			mov     edx, [ebp + local_counter]
			mov     eax, [ebp + local_xor]
			mov     ecx, [ebp + xor1]
			add     ecx, [eax + edx * 4 + 18h]
			mov     dword ptr[ebp + xor1], ecx
			mov     edx, [ebp + local_counter]
			mov     eax, [ebp + local_xor]
			mov     ecx, [ebp + xor0]
			add     ecx, [eax + edx * 4 + 1Ch]
			mov     dword ptr[ebp + xor0], ecx
			lea     edx, [ebp + xor0]
			push    edx
			lea     eax, [ebp + xor1]
			push    eax
			lea     ecx, [ebp + xor2]
			push    ecx
			lea     edx, [ebp + xor3]
			push    edx
			lea     eax, [ebp + xor4]
			push    eax
			lea     ecx, [ebp + xor5]
			push    ecx
			lea     edx, [ebp + xor6]
			push    edx
			lea     eax, [ebp + xor7]
			push    eax
			mov     ecx, [ebp + ptr_SubTableEntry3]
			mov     edx, [ecx]
			mov     ecx, [ebp + ptr_SubTableEntry3]
			call    dword ptr[edx + 10h]; SubTableEntry3::XOR_LOOP1
			mov     eax, [ebp + local_counter]
			mov     ecx, [ebp + local_unk]
			mov     edx, [ebp + xor7]
			mov     dword ptr[ecx + eax * 4], edx
			mov     eax, [ebp + local_counter]
			mov     ecx, [ebp + local_unk]
			mov     edx, [ebp + xor6]
			mov     dword ptr[ecx + eax * 4 + 4], edx
			mov     eax, [ebp + local_counter]
			mov     ecx, [ebp + local_unk]
			mov     edx, [ebp + xor5]
			mov     dword ptr[ecx + eax * 4 + 8], edx
			mov     eax, [ebp + local_counter]
			mov     ecx, [ebp + local_unk]
			mov     edx, [ebp + xor4]
			mov     dword ptr[ecx + eax * 4 + 0Ch], edx
			mov     eax, [ebp + local_counter]
			mov     ecx, [ebp + local_unk]
			mov     edx, [ebp + xor3]
			mov     dword ptr[ecx + eax * 4 + 10h], edx
			mov     eax, [ebp + local_counter]
			mov     ecx, [ebp + local_unk]
			mov     edx, [ebp + xor2]
			mov     dword ptr[ecx + eax * 4 + 14h], edx
			mov     eax, [ebp + local_counter]
			mov     ecx, [ebp + local_unk]
			mov     edx, [ebp + xor1]
			mov     dword ptr[ecx + eax * 4 + 18h], edx
			mov     eax, [ebp + local_counter]
			mov     ecx, [ebp + local_unk]
			mov     edx, [ebp + xor0]
			mov     dword ptr[ecx + eax * 4 + 1Ch], edx
			jmp     loc_4BD8A8
			; -------------------------------------------------------------------------- -

			iteration_256_complete:; CODE XREF : SubTableEntry3__Fill_Out_Tables_Part2 + D8.j
			mov     dword ptr[ebp + local_counter], 0
			jmp     short loc_4BD9E6
			; -------------------------------------------------------------------------- -

			loc_4BD9DD:; CODE XREF : SubTableEntry3__Fill_Out_Tables_Part2 + 324.j
			mov     eax, [ebp + local_counter]
			add     eax, 8; note, I += 2
			mov     dword ptr[ebp + local_counter], eax

			loc_4BD9E6 : ; CODE XREF : SubTableEntry3__Fill_Out_Tables_Part2 + 1FB.j
			cmp     dword ptr[ebp + local_counter], 100h
			jge     iteration_256_2_complete
			mov     ecx, [ebp + local_counter]
			mov     edx, [ebp + local_unk]
			mov     eax, [ebp + xor7]
			add     eax, [edx + ecx * 4]
			mov     dword ptr[ebp + xor7], eax
			mov     ecx, [ebp + local_counter]
			mov     edx, [ebp + local_unk]
			mov     eax, [ebp + xor6]
			add     eax, [edx + ecx * 4 + 4]
			mov     dword ptr[ebp + xor6], eax
			mov     ecx, [ebp + local_counter]
			mov     edx, [ebp + local_unk]
			mov     eax, [ebp + xor5]
			add     eax, [edx + ecx * 4 + 8]
			mov     dword ptr[ebp + xor5], eax
			mov     ecx, [ebp + local_counter]
			mov     edx, [ebp + local_unk]
			mov     eax, [ebp + xor4]
			add     eax, [edx + ecx * 4 + 0Ch]
			mov     dword ptr[ebp + xor4], eax
			mov     ecx, [ebp + local_counter]
			mov     edx, [ebp + local_unk]
			mov     eax, [ebp + xor3]
			add     eax, [edx + ecx * 4 + 10h]
			mov     dword ptr[ebp + xor3], eax
			mov     ecx, [ebp + local_counter]
			mov     edx, [ebp + local_unk]
			mov     eax, [ebp + xor2]
			add     eax, [edx + ecx * 4 + 14h]
			mov     dword ptr[ebp + xor2], eax
			mov     ecx, [ebp + local_counter]
			mov     edx, [ebp + local_unk]
			mov     eax, [ebp + xor1]
			add     eax, [edx + ecx * 4 + 18h]
			mov     dword ptr[ebp + xor1], eax
			mov     ecx, [ebp + local_counter]
			mov     edx, [ebp + local_unk]
			mov     eax, [ebp + xor0]
			add     eax, [edx + ecx * 4 + 1Ch]
			mov     dword ptr[ebp + xor0], eax
			lea     ecx, [ebp + xor0]
			push    ecx
			lea     edx, [ebp + xor1]
			push    edx
			lea     eax, [ebp + xor2]
			push    eax
			lea     ecx, [ebp + xor3]
			push    ecx
			lea     edx, [ebp + xor4]
			push    edx
			lea     eax, [ebp + xor5]
			push    eax
			lea     ecx, [ebp + xor6]
			push    ecx
			lea     edx, [ebp + xor7]
			push    edx
			mov     eax, [ebp + ptr_SubTableEntry3]
			mov     edx, [eax]
			mov     ecx, [ebp + ptr_SubTableEntry3]
			call    dword ptr[edx + 10h]; SubTableEntry3::XOR_LOOP1
			mov     eax, [ebp + local_counter]
			mov     ecx, [ebp + local_unk]
			mov     edx, [ebp + xor7]
			mov     dword ptr[ecx + eax * 4], edx
			mov     eax, [ebp + local_counter]
			mov     ecx, [ebp + local_unk]
			mov     edx, [ebp + xor6]
			mov     dword ptr[ecx + eax * 4 + 4], edx
			mov     eax, [ebp + local_counter]
			mov     ecx, [ebp + local_unk]
			mov     edx, [ebp + xor5]
			mov     dword ptr[ecx + eax * 4 + 8], edx
			mov     eax, [ebp + local_counter]
			mov     ecx, [ebp + local_unk]
			mov     edx, [ebp + xor4]
			mov     dword ptr[ecx + eax * 4 + 0Ch], edx
			mov     eax, [ebp + local_counter]
			mov     ecx, [ebp + local_unk]
			mov     edx, [ebp + xor3]
			mov     dword ptr[ecx + eax * 4 + 10h], edx
			mov     eax, [ebp + local_counter]
			mov     ecx, [ebp + local_unk]
			mov     edx, [ebp + xor2]
			mov     dword ptr[ecx + eax * 4 + 14h], edx
			mov     eax, [ebp + local_counter]
			mov     ecx, [ebp + local_unk]
			mov     edx, [ebp + xor1]
			mov     dword ptr[ecx + eax * 4 + 18h], edx
			mov     eax, [ebp + local_counter]
			mov     ecx, [ebp + local_unk]
			mov     edx, [ebp + xor0]
			mov     dword ptr[ecx + eax * 4 + 1Ch], edx
			jmp     loc_4BD9DD
			; -------------------------------------------------------------------------- -

			iteration_256_2_complete:; CODE XREF : SubTableEntry3__Fill_Out_Tables_Part2 + 20D.j
			jmp     last_part
			; -------------------------------------------------------------------------- -

			dont_use_keys_route:; CODE XREF : SubTableEntry3__Fill_Out_Tables_Part2 + B9.j
			lea     eax, [ebp + xor0]
			push    eax
			lea     ecx, [ebp + xor1]
			push    ecx
			lea     edx, [ebp + xor2]
			push    edx
			lea     eax, [ebp + xor3]
			push    eax
			lea     ecx, [ebp + xor4]
			push    ecx
			lea     edx, [ebp + xor5]
			push    edx
			lea     eax, [ebp + xor6]
			push    eax
			lea     ecx, [ebp + xor7]
			push    ecx
			mov     edx, [ebp + ptr_SubTableEntry3]
			mov     eax, [edx]
			mov     ecx, [ebp + ptr_SubTableEntry3]
			call    dword ptr[eax + 10h]; SubTableEntry3::XOR_LOOP1
			mov     ecx, [ebp + local_counter]
			mov     edx, [ebp + local_unk]
			mov     eax, [ebp + xor7]
			mov     dword ptr[edx + ecx * 4], eax
			mov     ecx, [ebp + local_counter]
			mov     edx, [ebp + local_unk]
			mov     eax, [ebp + xor6]
			mov     dword ptr[edx + ecx * 4 + 4], eax
			mov     ecx, [ebp + local_counter]
			mov     edx, [ebp + local_unk]
			mov     eax, [ebp + xor5]
			mov     dword ptr[edx + ecx * 4 + 8], eax
			mov     ecx, [ebp + local_counter]
			mov     edx, [ebp + local_unk]
			mov     eax, [ebp + xor4]
			mov     dword ptr[edx + ecx * 4 + 0Ch], eax
			mov     ecx, [ebp + local_counter]
			mov     edx, [ebp + local_unk]
			mov     eax, [ebp + xor3]
			mov     dword ptr[edx + ecx * 4 + 10h], eax
			mov     ecx, [ebp + local_counter]
			mov     edx, [ebp + local_unk]
			mov     eax, [ebp + xor2]
			mov     dword ptr[edx + ecx * 4 + 14h], eax
			mov     ecx, [ebp + local_counter]
			mov     edx, [ebp + local_unk]
			mov     eax, [ebp + xor1]
			mov     dword ptr[edx + ecx * 4 + 18h], eax
			mov     ecx, [ebp + local_counter]
			mov     edx, [ebp + local_unk]
			mov     eax, [ebp + xor0]
			mov     dword ptr[edx + ecx * 4 + 1Ch], eax

			last_part : ; CODE XREF : SubTableEntry3__Fill_Out_Tables_Part2 + 329.j
			mov     ecx, [ebp + ptr_SubTableEntry4]
			push    ecx
			mov     edx, [ebp + ptr_SubTableEntry3]
			mov     eax, [edx + SubTableEntry3_ptr_VTBL]
			mov     ecx, [ebp + ptr_SubTableEntry3]
			call    dword ptr[eax + 0Ch]
			mov     ecx, [ebp + ptr_SubTableEntry4]
			mov     dword ptr[ecx + SubTableEntry4_counter], 100h
			mov     esp, ebp
			pop     ebp
			retn    8
	}
}



#undef ptr_SubTableEntry3
#undef local_counter
#define temp_int			-12
#define ptr_SubTableEntry3	-8
#define local_counter		-4
#define arg_0				8
#define arg_4				12
#define arg_8				16
#define ptr_initvals		20

//check
__declspec(naked) void SubTableEntry3__Fill_Out_Tables(void)
{
	__asm
	{
		push    ebp
		mov     ebp, esp
		sub     esp, 0Ch
		mov     dword ptr[ebp + ptr_SubTableEntry3], ecx
		mov     dword ptr[ebp + local_counter], 0
		jmp     short loc_4BDBDB
		loc_4BDBD2 : ; CODE XREF : SubTableEntry3__Fill_Out_Tables + 4E.j
					 mov     eax, [ebp + local_counter]
					 add     eax, 1
					 mov     dword ptr[ebp + local_counter], eax
					 loc_4BDBDB : ; CODE XREF : SubTableEntry3__Fill_Out_Tables + 10.j
								  cmp     dword ptr[ebp + local_counter], 100h
								  jge     short loc_4BDC10
								  cmp     dword ptr[ebp + ptr_initvals], 0
								  jz      short loc_4BDBF8
								  mov     ecx, [ebp + local_counter]
								  mov     edx, [ebp + ptr_initvals]
								  mov     eax, [edx + ecx * 4]
								  mov     dword ptr[ebp + temp_int], eax
								  jmp     short loc_4BDBFF
								  loc_4BDBF8 : ; CODE XREF : SubTableEntry3__Fill_Out_Tables + 28.j
											   mov     dword ptr[ebp + temp_int], 0
											   loc_4BDBFF : ; CODE XREF : SubTableEntry3__Fill_Out_Tables + 36.j
															mov     ecx, [ebp + ptr_SubTableEntry3]
															mov     edx, [ecx + SubTableEntry3_ptr_xortable]
															mov     eax, [ebp + local_counter]
															mov     ecx, [ebp + temp_int]
															mov     dword ptr[edx + eax * 4], ecx
															jmp     short loc_4BDBD2
															loc_4BDC10 : ; CODE XREF : SubTableEntry3__Fill_Out_Tables + 22.j
																		 mov     edx, [ebp + ptr_SubTableEntry3]
																		 mov     eax, [ebp + arg_0]
																		 mov     dword ptr[edx + SubTableEntry3_arg0], eax
																		 mov     ecx, [ebp + ptr_SubTableEntry3]
																		 mov     edx, [ebp + arg_4]
																		 mov     dword ptr[ecx + SubTableEntry3_arg1], edx
																		 mov     eax, [ebp + ptr_SubTableEntry3]
																		 mov     ecx, [ebp + arg_8]
																		 mov     dword ptr[eax + SubTableEntry3_arg2], ecx
																		 push    1; USE GIVEN KEYS
																		 mov     edx, [ebp + ptr_SubTableEntry3]
																		 add     edx, 4
																		 push    edx
																		 mov     eax, [ebp + ptr_SubTableEntry3]
																		 mov     edx, [eax + SubTableEntry3_ptr_VTBL]
																		 mov     ecx, [ebp + ptr_SubTableEntry3]
																		 call    dword ptr[edx + 4]; SubTableEntry3::Fill_Out_Tables_Part2
																		 mov     esp, ebp
																		 pop     ebp
																		 retn    10h
	}
}



#undef ptr_SubTableEntry3
#define ptr_SubTableEntry3		-4
#define arg_0					8
#define arg_4					12

//check
__declspec(naked) void SubTableEntry3__Crazy_XOR_01(void)
{
	__asm
	{
		push    ebp
		mov     ebp, esp
		push    ecx
		mov     dword ptr[ebp + ptr_SubTableEntry3], ecx
		mov     eax, [ebp + arg_4]
		and eax, 3FCh
		mov     ecx, [ebp + arg_0]
		mov     eax, [ecx + eax]
		mov     esp, ebp
		pop     ebp
		retn    8
	}
}



#undef ptr_SubTableEntry3
#define ptr_SubTableEntry3		-4
#define arg_0					8
#define arg_4					12
#define arg_8					16
#define arg_C					20
#define arg_10					24
#define arg_14					28
#define arg_18					32
#define arg_1C					36
#define arg_20					40

//check
__declspec(naked) void SubTableEntry3__Crazy_XOR_00(void)
{
	__asm
	{
		push    ebp
		mov     ebp, esp
		push    ecx
		mov     dword ptr[ebp + ptr_SubTableEntry3], ecx
		mov     eax, [ebp + arg_10]
		mov     ecx, [eax]
		mov     edx, [ebp + arg_1C]
		mov     eax, [ecx]
		mov     dword ptr[edx], eax
		mov     ecx, [ebp + arg_4]
		mov     edx, [ecx]
		xor edx, [ebp + arg_0]
		mov     eax, [ebp + arg_14]
		mov     ecx, [eax]
		add     edx, [ecx]
		mov     eax, [ebp + arg_4]
		mov     dword ptr[eax], edx
		mov     ecx, [ebp + arg_14]
		mov     edx, [ecx]
		add     edx, 4
		mov     eax, [ebp + arg_14]
		mov     dword ptr[eax], edx
		mov     ecx, [ebp + arg_1C]
		mov     edx, [ecx]
		push    edx
		mov     eax, [ebp + arg_C]
		mov     ecx, [eax]
		push    ecx
		mov     ecx, [ebp + ptr_SubTableEntry3]
		call    SubTableEntry3__Crazy_XOR_01
		mov     edx, [ebp + arg_4]
		add     eax, [edx]
		mov     ecx, [ebp + arg_8]
		add     eax, [ecx]
		mov     edx, [ebp + arg_20]
		mov     dword ptr[edx], eax
		mov     eax, [ebp + arg_10]
		mov     ecx, [eax]
		mov     edx, [ebp + arg_20]
		mov     eax, [edx]
		mov     dword ptr[ecx], eax
		mov     ecx, [ebp + arg_10]
		mov     edx, [ecx]
		add     edx, 4
		mov     eax, [ebp + arg_10]
		mov     dword ptr[eax], edx
		mov     ecx, [ebp + arg_20]
		mov     edx, [ecx]
		shr     edx, 8
		push    edx
		mov     eax, [ebp + arg_C]
		mov     ecx, [eax]
		push    ecx
		mov     ecx, [ebp + ptr_SubTableEntry3]
		call    SubTableEntry3__Crazy_XOR_01
		mov     edx, [ebp + arg_1C]
		add     eax, [edx]
		mov     ecx, [ebp + arg_8]
		mov     dword ptr[ecx], eax
		mov     edx, [ebp + arg_18]
		mov     eax, [edx]
		mov     ecx, [ebp + arg_8]
		mov     edx, [ecx]
		mov     dword ptr[eax], edx
		mov     eax, [ebp + arg_18]
		mov     ecx, [eax]
		add     ecx, 4
		mov     edx, [ebp + arg_18]
		mov     dword ptr[edx], ecx
		mov     esp, ebp
		pop     ebp
		retn    24h
	}
}



#undef ptr_SubTableEntry3
#undef local_unk
#undef local_xor
#define ptr_SubTableEntry3		-40
#define lc_unk0					-36
#define lc_unk200				-32
#define var_1C					-28
#define var_18					-24
#define local_unk				-20
#define local_xor				-16
#define key2					-12
#define key0					-8
#define lc_unk0_stop_point		-4
#define ptr_SubTableEntry4		8

//check
__declspec(naked) void SubTableEntry3__Final_INIT_Stage(void)
{
	__asm
	{
		push    ebp
		mov     ebp, esp
		sub     esp, 28h
		mov     dword ptr[ebp + ptr_SubTableEntry3], ecx

		//puts argument unktable into localstack
		mov     eax, [ebp + ptr_SubTableEntry4]
		mov     ecx, [eax + SubTableEntry4_ptr_unktable]
		mov     dword ptr[ebp + local_unk], ecx

		//puts argument xortable into localstack
		mov     edx, [ebp + ptr_SubTableEntry4]
		mov     eax, [edx + SubTableEntry4_ptr_xortable]
		mov     dword ptr[ebp + local_xor], eax

		//puts argument arg0 into evilstack
		mov     ecx, [ebp + ptr_SubTableEntry4]
		mov     edx, [ecx + SubTableEntry4_arg0]
		mov     dword ptr[ebp + key0], edx

		//
		mov     eax, [ebp + ptr_SubTableEntry4]
		mov     ecx, [eax + SubTableEntry4_arg2]
		add     ecx, 1
		mov     edx, [ebp + ptr_SubTableEntry4]
		mov     dword ptr[edx + SubTableEntry4_arg2], ecx

		mov     eax, [ebp + ptr_SubTableEntry4]
		mov     ecx, [eax + SubTableEntry4_arg1]
		mov     edx, [ebp + ptr_SubTableEntry4]
		add     ecx, [edx + SubTableEntry4_arg2]
		mov     dword ptr[ebp + key2], ecx
		mov     eax, [ebp + local_unk]
		mov     dword ptr[ebp + lc_unk0], eax
		mov     ecx, [ebp + lc_unk0]
		add     ecx, 200h
		mov     dword ptr[ebp + lc_unk200], ecx
		mov     edx, [ebp + lc_unk200]
		mov     dword ptr[ebp + lc_unk0_stop_point], edx

		loc_4BDCAA : ; CODE XREF : SubTableEntry3__Final_INIT_Stage + 122.j
					 mov     eax, [ebp + lc_unk0]
					 cmp     eax, [ebp + lc_unk0_stop_point]
					 jnb     loc_4BDD77
					 lea     ecx, [ebp + var_1C]
					 push    ecx
					 lea     edx, [ebp + var_18]
					 push    edx
					 lea     eax, [ebp + local_xor]
					 push    eax
					 lea     ecx, [ebp + lc_unk200]
					 push    ecx
					 lea     edx, [ebp + lc_unk0]
					 push    edx
					 lea     eax, [ebp + local_unk]
					 push    eax
					 lea     ecx, [ebp + key2]
					 push    ecx
					 lea     edx, [ebp + key0]
					 push    edx
					 mov     eax, [ebp + key0]
					 shl     eax, 0Dh
					 push    eax
					 mov     ecx, [ebp + ptr_SubTableEntry3]
					 call    SubTableEntry3__Crazy_XOR_00
					 lea     ecx, [ebp + var_1C]
					 push    ecx
					 lea     edx, [ebp + var_18]
					 push    edx
					 lea     eax, [ebp + local_xor]
					 push    eax
					 lea     ecx, [ebp + lc_unk200]
					 push    ecx
					 lea     edx, [ebp + lc_unk0]
					 push    edx
					 lea     eax, [ebp + local_unk]
					 push    eax
					 lea     ecx, [ebp + key2]
					 push    ecx
					 lea     edx, [ebp + key0]
					 push    edx
					 mov     eax, [ebp + key0]
					 shr     eax, 6
					 push    eax
					 mov     ecx, [ebp + ptr_SubTableEntry3]
					 call    SubTableEntry3__Crazy_XOR_00
					 lea     ecx, [ebp + var_1C]
					 push    ecx
					 lea     edx, [ebp + var_18]
					 push    edx
					 lea     eax, [ebp + local_xor]
					 push    eax
					 lea     ecx, [ebp + lc_unk200]
					 push    ecx
					 lea     edx, [ebp + lc_unk0]
					 push    edx
					 lea     eax, [ebp + local_unk]
					 push    eax
					 lea     ecx, [ebp + key2]
					 push    ecx
					 lea     edx, [ebp + key0]
					 push    edx
					 mov     eax, [ebp + key0]
					 shl     eax, 2
					 push    eax
					 mov     ecx, [ebp + ptr_SubTableEntry3]
					 call    SubTableEntry3__Crazy_XOR_00
					 lea     ecx, [ebp + var_1C]
					 push    ecx
					 lea     edx, [ebp + var_18]
					 push    edx
					 lea     eax, [ebp + local_xor]
					 push    eax
					 lea     ecx, [ebp + lc_unk200]
					 push    ecx
					 lea     edx, [ebp + lc_unk0]
					 push    edx
					 lea     eax, [ebp + local_unk]
					 push    eax
					 lea     ecx, [ebp + key2]
					 push    ecx
					 lea     edx, [ebp + key0]
					 push    edx
					 mov     eax, [ebp + key0]
					 shr     eax, 10h
					 push    eax
					 mov     ecx, [ebp + ptr_SubTableEntry3]
					 call    SubTableEntry3__Crazy_XOR_00
					 jmp     loc_4BDCAA
					 ; -------------------------------------------------------------------------- -

					 loc_4BDD77:; CODE XREF : SubTableEntry3__Final_INIT_Stage + 60.j
								mov     ecx, [ebp + local_unk]
								mov     dword ptr[ebp + lc_unk200], ecx

								loc_4BDD7D : ; CODE XREF : SubTableEntry3__Final_INIT_Stage + 1F5.j
											 mov     edx, [ebp + lc_unk200]
											 cmp     edx, [ebp + lc_unk0_stop_point]
											 jnb     loc_4BDE4A
											 lea     eax, [ebp + var_1C]
											 push    eax
											 lea     ecx, [ebp + var_18]
											 push    ecx
											 lea     edx, [ebp + local_xor]
											 push    edx
											 lea     eax, [ebp + lc_unk200]
											 push    eax
											 lea     ecx, [ebp + lc_unk0]
											 push    ecx
											 lea     edx, [ebp + local_unk]
											 push    edx
											 lea     eax, [ebp + key2]
											 push    eax
											 lea     ecx, [ebp + key0]
											 push    ecx
											 mov     edx, [ebp + key0]
											 shl     edx, 0Dh
											 push    edx
											 mov     ecx, [ebp + ptr_SubTableEntry3]
											 call    SubTableEntry3__Crazy_XOR_00
											 lea     eax, [ebp + var_1C]
											 push    eax
											 lea     ecx, [ebp + var_18]
											 push    ecx
											 lea     edx, [ebp + local_xor]
											 push    edx
											 lea     eax, [ebp + lc_unk200]
											 push    eax
											 lea     ecx, [ebp + lc_unk0]
											 push    ecx
											 lea     edx, [ebp + local_unk]
											 push    edx
											 lea     eax, [ebp + key2]
											 push    eax
											 lea     ecx, [ebp + key0]
											 push    ecx
											 mov     edx, [ebp + key0]
											 shr     edx, 6
											 push    edx
											 mov     ecx, [ebp + ptr_SubTableEntry3]
											 call    SubTableEntry3__Crazy_XOR_00
											 lea     eax, [ebp + var_1C]
											 push    eax
											 lea     ecx, [ebp + var_18]
											 push    ecx
											 lea     edx, [ebp + local_xor]
											 push    edx
											 lea     eax, [ebp + lc_unk200]
											 push    eax
											 lea     ecx, [ebp + lc_unk0]
											 push    ecx
											 lea     edx, [ebp + local_unk]
											 push    edx
											 lea     eax, [ebp + key2]
											 push    eax
											 lea     ecx, [ebp + key0]
											 push    ecx
											 mov     edx, [ebp + key0]
											 shl     edx, 2
											 push    edx
											 mov     ecx, [ebp + ptr_SubTableEntry3]
											 call    SubTableEntry3__Crazy_XOR_00
											 lea     eax, [ebp + var_1C]
											 push    eax
											 lea     ecx, [ebp + var_18]
											 push    ecx
											 lea     edx, [ebp + local_xor]
											 push    edx
											 lea     eax, [ebp + lc_unk200]
											 push    eax
											 lea     ecx, [ebp + lc_unk0]
											 push    ecx
											 lea     edx, [ebp + local_unk]
											 push    edx
											 lea     eax, [ebp + key2]
											 push    eax
											 lea     ecx, [ebp + key0]
											 push    ecx
											 mov     edx, [ebp + key0]
											 shr     edx, 10h
											 push    edx
											 mov     ecx, [ebp + ptr_SubTableEntry3]
											 call    SubTableEntry3__Crazy_XOR_00
											 jmp     loc_4BDD7D
											 ; -------------------------------------------------------------------------- -

											 loc_4BDE4A:; CODE XREF : SubTableEntry3__Final_INIT_Stage + 133.j
														mov     eax, [ebp + ptr_SubTableEntry4]
														mov     ecx, [ebp + key2]
														mov     dword ptr[eax + SubTableEntry4_arg1], ecx
														mov     edx, [ebp + ptr_SubTableEntry4]
														mov     eax, [ebp + key0]
														mov     dword ptr[edx + SubTableEntry4_arg0], eax
														mov     esp, ebp
														pop     ebp
														retn    4
	}
}



#undef ptr_SubTableEntry3
#define ptr_SubTableEntry3		-4
#define arg_0					8
#define arg_4					12
#define arg_8					16
#define arg_C					20
#define arg_10					24
#define arg_14					28
#define arg_18					32
#define arg_1C					36

//check
__declspec(naked) void SubTableEntry3__XOR_LOOP1(void)
{
	__asm
	{
		push    ebp
		mov     ebp, esp
		push    ecx
		mov     dword ptr[ebp + ptr_SubTableEntry3], ecx
		mov     eax, [ebp + arg_4]
		mov     ecx, [eax]
		shl     ecx, 0Bh
		mov     edx, [ebp + arg_0]
		mov     eax, [edx]
		xor eax, ecx
		mov     ecx, [ebp + arg_0]
		mov     dword ptr[ecx], eax
		mov     edx, [ebp + arg_C]
		mov     eax, [edx]
		mov     ecx, [ebp + arg_0]
		add     eax, [ecx]
		mov     edx, [ebp + arg_C]
		mov     dword ptr[edx], eax
		mov     eax, [ebp + arg_4]
		mov     ecx, [eax]
		mov     edx, [ebp + arg_8]
		add     ecx, [edx]
		mov     eax, [ebp + arg_4]
		mov     dword ptr[eax], ecx
		mov     ecx, [ebp + arg_8]
		mov     edx, [ecx]
		shr     edx, 2
		mov     eax, [ebp + arg_4]
		mov     ecx, [eax]
		xor ecx, edx
		mov     edx, [ebp + arg_4]
		mov     dword ptr[edx], ecx
		mov     eax, [ebp + arg_10]
		mov     ecx, [eax]
		mov     edx, [ebp + arg_4]
		add     ecx, [edx]
		mov     eax, [ebp + arg_10]
		mov     dword ptr[eax], ecx
		mov     ecx, [ebp + arg_8]
		mov     edx, [ecx]
		mov     eax, [ebp + arg_C]
		add     edx, [eax]
		mov     ecx, [ebp + arg_8]
		mov     dword ptr[ecx], edx
		mov     edx, [ebp + arg_C]
		mov     eax, [edx]
		shl     eax, 8
		mov     ecx, [ebp + arg_8]
		mov     edx, [ecx]
		xor edx, eax
		mov     eax, [ebp + arg_8]
		mov     dword ptr[eax], edx
		mov     ecx, [ebp + arg_14]
		mov     edx, [ecx]
		mov     eax, [ebp + arg_8]
		add     edx, [eax]
		mov     ecx, [ebp + arg_14]
		mov     dword ptr[ecx], edx
		mov     edx, [ebp + arg_C]
		mov     eax, [edx]
		mov     ecx, [ebp + arg_10]
		add     eax, [ecx]
		mov     edx, [ebp + arg_C]
		mov     dword ptr[edx], eax
		mov     eax, [ebp + arg_10]
		mov     ecx, [eax]
		shr     ecx, 10h
		mov     edx, [ebp + arg_C]
		mov     eax, [edx]
		xor eax, ecx
		mov     ecx, [ebp + arg_C]
		mov     dword ptr[ecx], eax
		mov     edx, [ebp + arg_18]
		mov     eax, [edx]
		mov     ecx, [ebp + arg_C]
		add     eax, [ecx]
		mov     edx, [ebp + arg_18]
		mov     dword ptr[edx], eax
		mov     eax, [ebp + arg_10]
		mov     ecx, [eax]
		mov     edx, [ebp + arg_14]
		add     ecx, [edx]
		mov     eax, [ebp + arg_10]
		mov     dword ptr[eax], ecx
		mov     ecx, [ebp + arg_14]
		mov     edx, [ecx]
		shl     edx, 0Ah
		mov     eax, [ebp + arg_10]
		mov     ecx, [eax]
		xor ecx, edx
		mov     edx, [ebp + arg_10]
		mov     dword ptr[edx], ecx
		mov     eax, [ebp + arg_1C]
		mov     ecx, [eax]
		mov     edx, [ebp + arg_10]
		add     ecx, [edx]
		mov     eax, [ebp + arg_1C]
		mov     dword ptr[eax], ecx
		mov     ecx, [ebp + arg_14]
		mov     edx, [ecx]
		mov     eax, [ebp + arg_18]
		add     edx, [eax]
		mov     ecx, [ebp + arg_14]
		mov     dword ptr[ecx], edx
		mov     edx, [ebp + arg_18]
		mov     eax, [edx]
		shr     eax, 4
		mov     ecx, [ebp + arg_14]
		mov     edx, [ecx]
		xor edx, eax
		mov     eax, [ebp + arg_14]
		mov     dword ptr[eax], edx
		mov     ecx, [ebp + arg_0]
		mov     edx, [ecx]
		mov     eax, [ebp + arg_14]
		add     edx, [eax]
		mov     ecx, [ebp + arg_0]
		mov     dword ptr[ecx], edx
		mov     edx, [ebp + arg_18]
		mov     eax, [edx]
		mov     ecx, [ebp + arg_1C]
		add     eax, [ecx]
		mov     edx, [ebp + arg_18]
		mov     dword ptr[edx], eax
		mov     eax, [ebp + arg_1C]
		mov     ecx, [eax]
		shl     ecx, 8
		mov     edx, [ebp + arg_18]
		mov     eax, [edx]
		xor eax, ecx
		mov     ecx, [ebp + arg_18]
		mov     dword ptr[ecx], eax
		mov     edx, [ebp + arg_4]
		mov     eax, [edx]
		mov     ecx, [ebp + arg_18]
		add     eax, [ecx]
		mov     edx, [ebp + arg_4]
		mov     dword ptr[edx], eax
		mov     eax, [ebp + arg_1C]
		mov     ecx, [eax]
		mov     edx, [ebp + arg_0]
		add     ecx, [edx]
		mov     eax, [ebp + arg_1C]
		mov     dword ptr[eax], ecx
		mov     ecx, [ebp + arg_0]
		mov     edx, [ecx]
		shr     edx, 9
		mov     eax, [ebp + arg_1C]
		mov     ecx, [eax]
		xor ecx, edx
		mov     edx, [ebp + arg_1C]
		mov     dword ptr[edx], ecx
		mov     eax, [ebp + arg_8]
		mov     ecx, [eax]
		mov     edx, [ebp + arg_1C]
		add     ecx, [edx]
		mov     eax, [ebp + arg_8]
		mov     dword ptr[eax], ecx
		mov     ecx, [ebp + arg_0]
		mov     edx, [ecx]
		mov     eax, [ebp + arg_4]
		add     edx, [eax]
		mov     ecx, [ebp + arg_0]
		mov     dword ptr[ecx], edx
		mov     esp, ebp
		pop     ebp
		retn    20h
	}
}



#undef ptr_SubTableEntry3
#undef key0
#undef key2
#define ptr_SubTableEntry3		-4
#define key0					8
#define key1					12
#define key2					16

//check
__declspec(naked) void SubTableEntry3__Constructor(void)
{
	__asm
	{
		push    ebp
		mov     ebp, esp
		push    ecx
		mov     dword ptr[ebp + ptr_SubTableEntry3], ecx
		mov     ecx, [ebp + ptr_SubTableEntry3]
		add     ecx, 4
		call    SubTableEntry4__Constructor
		mov     eax, [ebp + ptr_SubTableEntry3]
		lea		ecx, SubTableEntry3__VTBL
		mov     dword ptr[eax + SubTableEntry3_ptr_VTBL], ecx
		push    0; Tables start at zero
		mov     ecx, [ebp + key2]
		push    ecx
		mov     edx, [ebp + key1]
		push    edx
		mov     eax, [ebp + key0]
		push    eax
		mov     ecx, [ebp + ptr_SubTableEntry3]
		call    SubTableEntry3__Fill_Out_Tables
		mov     eax, [ebp + ptr_SubTableEntry3]
		mov     esp, ebp
		pop     ebp
		retn    0Ch
	}
}



#undef ptr_SubTableEntry3
#define result					-8
#define ptr_SubTableEntry3		-4

//check
__declspec(naked) void SubTableEntry3__Fetch_XorVal(void)
{
	__asm
	{
		push    ebp
		mov     ebp, esp
		sub     esp, 8
		mov     dword ptr[ebp + ptr_SubTableEntry3], ecx
		mov     eax, [ebp + ptr_SubTableEntry3]
		mov     ecx, [eax + SubTableEntry3_counter]
		mov     edx, [ebp + ptr_SubTableEntry3]
		mov     eax, [edx + SubTableEntry3_counter]
		sub     eax, 1
		mov     edx, [ebp + ptr_SubTableEntry3]
		mov     dword ptr[edx + SubTableEntry3_counter], eax
		test    ecx, ecx
		jnz     short loc_4BD7C2
		mov     eax, [ebp + ptr_SubTableEntry3]
		add     eax, 4
		push    eax
		mov     ecx, [ebp + ptr_SubTableEntry3]
		mov     edx, [ecx]
		mov     ecx, [ebp + ptr_SubTableEntry3]
		call    dword ptr[edx + 0Ch]; SubTableEntry3::Final_INIT_Stage(SubTableEntry4 *d)
		mov     eax, [ebp + ptr_SubTableEntry3]
		mov     dword ptr[eax + SubTableEntry3_counter], 0FFh
		mov     ecx, [ebp + ptr_SubTableEntry3]
		mov     edx, [ecx + SubTableEntry3_counter]
		mov     eax, [ebp + ptr_SubTableEntry3]
		mov     ecx, [eax + SubTableEntry3_ptr_xortable]
		mov     edx, [ecx + edx * 4]
		mov     dword ptr[ebp + result], edx
		jmp     short loc_4BD7D4
		; -------------------------------------------------------------------------- -

		loc_4BD7C2:; CODE XREF : SubTableEntry3__Fetch_XorVal + 20.j
				   mov     eax, [ebp + ptr_SubTableEntry3]
				   mov     ecx, [eax + SubTableEntry3_counter]
				   mov     edx, [ebp + ptr_SubTableEntry3]
				   mov     eax, [edx + SubTableEntry3_ptr_xortable]
				   mov     ecx, [eax + ecx * 4]
				   mov     dword ptr[ebp + result], ecx

				   loc_4BD7D4 : ; CODE XREF : SubTableEntry3__Fetch_XorVal + 50.j
								mov     eax, [ebp + result]
								mov     esp, ebp
								pop     ebp
								retn
	}
}



void *SubTableEntry3__VTBL[5] = {
	NULL, //	SubTableEntry3__DESTRUCTOR,
	SubTableEntry3__Fill_Out_Tables_Part2,
	SubTableEntry3__Fill_Out_Tables,
	SubTableEntry3__Final_INIT_Stage,
	SubTableEntry3__XOR_LOOP1
};



#undef ptr_SubTableEntry3
#define ptr_SubTableEntry3_2	-12
#define ptr_SubTableEntry2		-8
#define ptr_SubTableEntry3		-4
#define key						8

//check
__declspec(naked) void SubTableEntry2__Constructor(void)
{
	__asm
	{
		push    ebp
		mov     ebp, esp
		sub     esp, 0Ch
		mov     dword ptr[ebp + ptr_SubTableEntry2], ecx
		mov     eax, [ebp + ptr_SubTableEntry2]
		mov     dword ptr[eax + SubTableEntry2_seqnum], 1
		mov     ecx, [ebp + ptr_SubTableEntry2]
		mov     edx, [ebp + key]
		mov     dword ptr[ecx + SubTableEntry2_encryption_key], edx
		mov     eax, [ebp + ptr_SubTableEntry2]
		mov     dword ptr[eax + SubTableEntry2_ptr_SubTableEntry3], 0
		push    1Ch
		call    malloc
		add     esp, 4
		mov     dword ptr[ebp + ptr_SubTableEntry3], eax
		cmp     dword ptr[ebp + ptr_SubTableEntry3], 0
		jz      short loc_4BD5EA
		mov     ecx, [ebp + ptr_SubTableEntry2]
		mov     edx, [ecx + SubTableEntry2_encryption_key]
		push    edx
		mov     eax, [ebp + ptr_SubTableEntry2]
		mov     ecx, [eax + SubTableEntry2_encryption_key]
		push    ecx
		mov     edx, [ebp + ptr_SubTableEntry2]
		mov     eax, [edx + SubTableEntry2_encryption_key]
		push    eax
		mov     ecx, [ebp + ptr_SubTableEntry3]
		call    SubTableEntry3__Constructor; SubTableEntry3::Constructor(int key0, int key1, int key2)
		mov     dword ptr[ebp + ptr_SubTableEntry3_2], eax
		jmp     short loc_4BD5F1
		; -------------------------------------------------------------------------- -

		loc_4BD5EA:; CODE XREF : SubTableEntry2__Constructor + 36.j
				   mov     dword ptr[ebp + ptr_SubTableEntry3_2], 0

				   loc_4BD5F1 : ; CODE XREF : SubTableEntry2__Constructor + 58.j
								mov     ecx, [ebp + ptr_SubTableEntry2]
								mov     edx, [ebp + ptr_SubTableEntry3_2]
								mov     dword ptr[ecx + SubTableEntry2_ptr_SubTableEntry3], edx
								mov     eax, [ebp + ptr_SubTableEntry2]
								mov     esp, ebp
								pop     ebp
								retn    4
	}
}



#undef result
#undef ptr_SubTableEntry2
#define ptr_SubTableEntry2		-8
#define result					-4

//check
__declspec(naked) void SubTableEntry2__Get_xorval_from_table3(void)
{
	__asm
	{
		push    ebp
		mov     ebp, esp
		sub     esp, 8
		mov     dword ptr[ebp + ptr_SubTableEntry2], ecx
		mov     eax, [ebp + ptr_SubTableEntry2]
		mov     ecx, [eax + SubTableEntry2_ptr_SubTableEntry3]
		call    SubTableEntry3__Fetch_XorVal
		mov     dword ptr[ebp + result], eax
		mov     eax, [ebp + result]
		mov     esp, ebp
		pop     ebp
		retn
	}
}



#undef ptr_SubTableEntry2
#define ptr_SubTableEntry2		-4

//check
__declspec(naked) void SubTableEntry2__IncrementSeqnum_and_get_xorval(void)
{
	__asm
	{
		push    ebp
		mov     ebp, esp
		push    ecx
		mov     dword ptr[ebp + ptr_SubTableEntry2], ecx
		mov     eax, [ebp + ptr_SubTableEntry2]
		mov     ecx, [eax + SubTableEntry2_seqnum]
		add     ecx, 1
		mov     edx, [ebp + ptr_SubTableEntry2]
		mov     dword ptr[edx + SubTableEntry2_seqnum], ecx
		mov     ecx, [ebp + ptr_SubTableEntry2]
		call    SubTableEntry2__Get_xorval_from_table3; DWORD SubTableEntry2::Get_CRC_Val_from_table(void)
		mov     esp, ebp
		pop     ebp
		retn
	}
}

DWORD GetSendXORVal(DWORD* lpdwSendCRC)
{
	//DWORD *lpdwSend = lpdwSendCRC, dwResult;
	DWORD dwResult;

	__asm
	{
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		mov ecx, [lpdwSendCRC]
		call SubTableEntry2__IncrementSeqnum_and_get_xorval
		mov[dwResult], eax
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
	}
	return dwResult;
}

//check
void GenerateCRCs(DWORD dwSendSeed, DWORD dwRecvSeed, DWORD* lpdwSendSeed, DWORD* lpdwRecvSeed)
{
	//ULONG *send=lpdwSEND_CRC;
	//ULONG *recv=lpdwRECV_CRC;

	__asm
	{
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push dwSendSeed
		mov ecx, [lpdwSendSeed]
		call SubTableEntry2__Constructor
		push dwRecvSeed
		mov ecx, [lpdwRecvSeed]
		call SubTableEntry2__Constructor
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
	}
	return;
}