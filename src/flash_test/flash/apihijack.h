/*--------------------------------------------------------------------------------------------------------
  APIHIJACK.H - Based on DelayLoadProfileDLL.CPP, by Matt Pietrek for MSJ February 2000.
  http://msdn.microsoft.com/library/periodic/period00/hood0200.htm
  Adapted by Wade Brainerd, wadeb@wadeb.com
--------------------------------------------------------------------------------------------------------*/
#ifndef APIHIJACK_H
#define APIHIJACK_H

#pragma warning(disable:4200)

// Macro for convenient pointer addition.
// Essentially treats the last two parameters as DWORDs.  The first
// parameter is used to typecast the result to the appropriate pointer type.
#define MakePtr(cast, ptr, addValue ) (cast)( (DWORD)(ptr)+(DWORD)(addValue))

// Default Hook Stub Structure: Contains data about the original function, Name/Ordinal, Address 
// and a Count field.  This is actually a block of assembly code.
#pragma pack( push, 1 )
struct DLPD_IAT_STUB
{
    BYTE    instr_CALL;
    DWORD   data_call;
    BYTE    instr_JMP;
    DWORD   data_JMP;
    DWORD   count;
    DWORD   pszNameOrOrdinal;

    DLPD_IAT_STUB() : instr_CALL( 0xE8 ), instr_JMP( 0xE9 ), count( 0 ) {}
};
#pragma pack( pop )

// Example DefaultHook procedure, called from the DLPD_IAT_STUB stubs.  
// Increments "count" field of the stub.
// See the implementation for more information.
void __cdecl DefaultHook( PVOID dummy );

struct SFunctionHook
{
    char* Name;         // Function name, e.g. "DirectDrawCreateEx".
    void* HookFn;       // Address of your function.
    void* OrigFn;       // Stored by HookAPICalls, the address of the original function.
};

struct SDLLHook
{
    // Name of the DLL, e.g. "DDRAW.DLL"
    char* Name;

    // Set true to call the default for all non-hooked functions before they are executed.
    bool UseDefault;
    void* DefaultFn;

    // Function hook array.  Terminated with a NULL Name field.
    SFunctionHook Functions[];
};

// Hook functions one or more DLLs.
bool HookAPICalls( SDLLHook* Hook, HMODULE hMod, bool bset);

#endif