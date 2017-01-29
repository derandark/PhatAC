
extern void SubTableEntry2__IncrementSeqnum_and_get_xorval (void);
extern void SubTableEntry2__Get_xorval_from_table3 (void);
extern void SubTableEntry2__Constructor (void);
extern void SubTableEntry3__Fetch_XorVal (void);
extern void SubTableEntry3__Constructor (void);
extern void SubTableEntry3__XOR_LOOP1 (void);
extern void SubTableEntry3__Final_INIT_Stage (void);
extern void SubTableEntry3__Crazy_XOR_00 (void);
extern void SubTableEntry3__Crazy_XOR_01 (void);
extern void SubTableEntry3__Fill_Out_Tables (void);
extern void SubTableEntry3__Fill_Out_Tables_Part2 (void);
extern void SubTableEntry3__DESTRUCTOR (void);
extern void SubTableEntry4__Constructor (void);

extern DWORD GetSendXORVal(DWORD* lpdwSendCRC );
extern void GenerateCRCs(DWORD dwSendSeed, DWORD dwRecvSeed, DWORD* lpdwSendSeed, DWORD* lpdwRecvSeed);