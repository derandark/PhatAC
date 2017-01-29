
#include "StdAfx.h"

extern void *_SubTableEntry3__VTBL[5];

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

struct __SubTableEntry4
{
	__SubTableEntry4();
	~__SubTableEntry4();
	DWORD dwCounter;
	DWORD *pdwXORTable;
	DWORD *pdwUnkTable;

	DWORD dwVar1;
	DWORD dwVar2;
	DWORD dwVar3;
};

__SubTableEntry4::__SubTableEntry4()
{
	pdwXORTable = new DWORD[256];
	pdwUnkTable = new DWORD[256];
}

__SubTableEntry4::~__SubTableEntry4()
{
	delete[] pdwXORTable;
	delete[] pdwUnkTable;
}

struct __SubTableEntry3
{
	__SubTableEntry3();
	~__SubTableEntry3();

	void Fill_Out_Tables_Part2(__SubTableEntry4 *pTable4, BOOL bUseGivenKeys);
	void Fill_Out_Tables_Part(DWORD dwArg0, DWORD dwArg4, DWORD Arg8, DWORD *pdwInitValues);
	void InitStage(__SubTableEntry4 *pTable4);
	void XorLoop(DWORD *pdwValue1, DWORD *pdwValue2, DWORD *pdwValue3, DWORD *pdwValue4,
		DWORD *pdwValue5, DWORD *pdwValue6, DWORD *pdwValue7, DWORD *pdwValue8);

	__SubTableEntry4 *pMyTable4;

	DWORD dwCounter;
	DWORD *pdwXORTable;
	DWORD *pdwUnkTable;

	DWORD dwArg1;
	DWORD dwArg2;
	DWORD dwArg3;
};

void __SubTableEntry3::Fill_Out_Tables_Part(DWORD dwArg0, DWORD dwArg4, DWORD dwArg8, DWORD *pdwInitValues)
{
	for (DWORD i = 0; i < 256; i++)
	{
		if (pdwInitValues)
			pdwXORTable[i] = pdwInitValues[i];
		else
			pdwXORTable[i] = 0;
	}

	dwArg1 = dwArg0;
	dwArg2 = dwArg4;
	dwArg3 = dwArg8;

	Fill_Out_Tables_Part2(pMyTable4, TRUE);
}

void __SubTableEntry3::Fill_Out_Tables_Part2(__SubTableEntry4* pTable4, BOOL bUseGivenKeys)
{
	DWORD dwValue[8];
	dwValue[0] = 0x9E3779B9;
	dwValue[1] = 0x9E3779B9;
	dwValue[2] = 0x9E3779B9;
	dwValue[3] = 0x9E3779B9;
	dwValue[4] = 0x9E3779B9;
	dwValue[5] = 0x9E3779B9;
	dwValue[6] = 0x9E3779B9;
	dwValue[7] = 0x9E3779B9;

	DWORD *pdwSubXORTable = pTable4->pdwXORTable;
	DWORD *pdwSubUnkTable = pTable4->pdwUnkTable;

	if (!bUseGivenKeys)
	{
		pTable4->dwVar1 = 0;
		pTable4->dwVar2 = 0;
		pTable4->dwVar3 = 0;
	}

	for (DWORD i = 0; i < 4; i++)
	{
		XorLoop(&dwValue[7], &dwValue[6], &dwValue[5], &dwValue[4],
			&dwValue[3], &dwValue[2], &dwValue[1], &dwValue[0]);
	}

	if (bUseGivenKeys)
	{
		for (DWORD i = 0; i < 256; i += 8)
		{
			dwValue[7] += pdwSubXORTable[i + 0];
			dwValue[6] += pdwSubXORTable[i + 1];
			dwValue[5] += pdwSubXORTable[i + 2];
			dwValue[4] += pdwSubXORTable[i + 3];
			dwValue[3] += pdwSubXORTable[i + 4];
			dwValue[2] += pdwSubXORTable[i + 5];
			dwValue[1] += pdwSubXORTable[i + 6];
			dwValue[0] += pdwSubXORTable[i + 7];

			XorLoop(&dwValue[7], &dwValue[6], &dwValue[5], &dwValue[4],
				&dwValue[3], &dwValue[2], &dwValue[1], &dwValue[0]);

			pdwSubUnkTable[i + 0] = dwValue[7];
			pdwSubUnkTable[i + 1] = dwValue[6];
			pdwSubUnkTable[i + 2] = dwValue[5];
			pdwSubUnkTable[i + 3] = dwValue[4];
			pdwSubUnkTable[i + 4] = dwValue[3];
			pdwSubUnkTable[i + 5] = dwValue[2];
			pdwSubUnkTable[i + 6] = dwValue[1];
			pdwSubUnkTable[i + 7] = dwValue[0];
		}

		for (DWORD i = 0; i < 256; i += 8)
		{
			dwValue[7] += pdwSubUnkTable[i + 0];
			dwValue[6] += pdwSubUnkTable[i + 1];
			dwValue[5] += pdwSubUnkTable[i + 2];
			dwValue[4] += pdwSubUnkTable[i + 3];
			dwValue[3] += pdwSubUnkTable[i + 4];
			dwValue[2] += pdwSubUnkTable[i + 5];
			dwValue[1] += pdwSubUnkTable[i + 6];
			dwValue[0] += pdwSubUnkTable[i + 7];

			XorLoop(&dwValue[7], &dwValue[6], &dwValue[5], &dwValue[4],
				&dwValue[3], &dwValue[2], &dwValue[1], &dwValue[0]);

			pdwSubUnkTable[i + 0] = dwValue[7];
			pdwSubUnkTable[i + 1] = dwValue[6];
			pdwSubUnkTable[i + 2] = dwValue[5];
			pdwSubUnkTable[i + 3] = dwValue[4];
			pdwSubUnkTable[i + 4] = dwValue[3];
			pdwSubUnkTable[i + 5] = dwValue[2];
			pdwSubUnkTable[i + 6] = dwValue[1];
			pdwSubUnkTable[i + 7] = dwValue[0];
		}
	}
	else
	{
		XorLoop(&dwValue[7], &dwValue[6], &dwValue[5], &dwValue[4],
			&dwValue[3], &dwValue[2], &dwValue[1], &dwValue[0]);

		pdwSubUnkTable[4] = dwValue[7];
		pdwSubUnkTable[5] = dwValue[6];
		pdwSubUnkTable[6] = dwValue[5];
		pdwSubUnkTable[7] = dwValue[4];
		pdwSubUnkTable[8] = dwValue[3];
		pdwSubUnkTable[9] = dwValue[2];
		pdwSubUnkTable[10] = dwValue[1];
		pdwSubUnkTable[11] = dwValue[0];
	}

	InitStage(pTable4);
}

void __SubTableEntry3::XorLoop(DWORD *pdwValue0, DWORD *pdwValue4, DWORD *pdwValue8, DWORD *pdwValueC, DWORD *pdwValue10, DWORD *pdwValue14, DWORD *pdwValue18, DWORD *pdwValue1C)
{
	(*pdwValue0) ^= (*pdwValue4) << 11;
	(*pdwValueC) += (*pdwValue0);
	(*pdwValue4) += (*pdwValue8);
	(*pdwValue4) ^= (*pdwValue8) >> 2;
	(*pdwValue10) += (*pdwValue4);
	(*pdwValue8) += (*pdwValueC);
	(*pdwValue8) ^= (*pdwValueC) << 8;
	(*pdwValue14) += (*pdwValue8);
	(*pdwValueC) += (*pdwValue10);
	(*pdwValueC) ^= (*pdwValue10) >> 10;
	(*pdwValue18) += (*pdwValueC);
	(*pdwValue10) += (*pdwValue14);
	(*pdwValue10) ^= (*pdwValue14) << 10;
	(*pdwValue1C) += (*pdwValue10);
	(*pdwValue14) += (*pdwValue18);
	(*pdwValue14) ^= (*pdwValue18) >> 4;
	(*pdwValue0) += (*pdwValue14);
	(*pdwValue18) += (*pdwValue1C);
	(*pdwValue18) ^= (*pdwValue1C) << 8;
	(*pdwValue4) += (*pdwValue18);
	(*pdwValue1C) += (*pdwValue0);
	(*pdwValue1C) ^= (*pdwValue0) >> 9;
	(*pdwValue8) += (*pdwValue1C);
	(*pdwValue0) += (*pdwValue4);
}

void __SubTableEntry3::InitStage(__SubTableEntry4 *pTable4)
{
	DWORD *pdwSubUnkTable = pTable4->pdwUnkTable;
	DWORD *pdwSubXORTable = pTable4->pdwXORTable;

	DWORD dwVar8 = pTable4->dwVar2 + (++pTable4->dwVar3);

	DWORD *pdwBottom = pdwSubUnkTable;
	DWORD *pdwTop = &pdwSubUnkTable[0x80];

	if (pdwTop >= pdwBottom)
	{
		DWORD dwESI = *pdwBottom;

		DWORD dwEDI;
		dwEDI = pTable4->dwVar1 << 0x0D;
		dwEDI ^= pTable4->dwVar1;

		DWORD dwEBP = dwVar8;
		DWORD dwVarC = dwESI;

		dwESI &= 0x3FC;
		dwESI = *((DWORD *)(dwESI + (BYTE *)pdwSubUnkTable));
	}
}














