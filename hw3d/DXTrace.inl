// Debug trace helper (this file is included into the body of
// `DXTraceW` / `DXTraceA` in `dxerr.cpp` with macros that
// define `DX_CHAR`, `DX_SPRINTF_S`, `DX_OUTPUTDEBUGSTRING`, etc.).
//
// Purpose:
// - Format a message that includes file, line, error code and an
//   optional caller-supplied message.
// - Output the formatted string to the debugger via
//   `OutputDebugStringW/A` (mapped to `DX_OUTPUTDEBUGSTRING`).
// - Optionally present a modal message box and break into the
//   debugger if the user requests it (controlled by `bPopMsgBox`).
//
// Note: This file intentionally does not include function
// declarations or braces; it is included inside the body of the
// DXTrace function where the macros above are defined.

// This file expects the including TU to define macros such as
// `DX_CHAR`, `DX_SPRINTF_S`, and `DX_OUTPUTDEBUGSTRING`.
// If it's compiled standalone (which happens when .inl files are
// mistakenly added as source files), wrap the contents so it
// becomes a no-op unless those macros are defined.
#if defined(DX_CHAR) || defined(DX_SPRINTF_S) || defined(DX_OUTPUTDEBUGSTRING)
// Small buffers used to build the diagnostic text. `strBufferLine` is
// used to hold the stringified line number.
DX_CHAR strBufferLine[128];
DX_CHAR strBufferError[256];
DX_CHAR strBuffer[BUFFER_SIZE];

DX_SPRINTF_S( strBufferLine, 128, DX_STR_WRAP("%lu"), dwLine );
if( strFile )
{
    DX_SPRINTF_S( strBuffer, BUFFER_SIZE, DX_STR_WRAP(STR_FMT_SPEC "(" STR_FMT_SPEC "): "), strFile, strBufferLine );
    DX_OUTPUTDEBUGSTRING( strBuffer );
}

size_t nMsgLen = (strMsg) ? DX_STRNLEN_S( strMsg, 1024 ) : 0;
if( nMsgLen > 0 )
{
    DX_OUTPUTDEBUGSTRING( strMsg );
    DX_OUTPUTDEBUGSTRING( DX_STR_WRAP(" ") );
}

DX_SPRINTF_S( strBufferError, 256, DX_STR_WRAP(STR_FMT_SPEC " (0x%0.8x)"), DX_GETERRORSTRING(hr), hr );
DX_SPRINTF_S( strBuffer, BUFFER_SIZE, DX_STR_WRAP("hr=" STR_FMT_SPEC), strBufferError );
DX_OUTPUTDEBUGSTRING( strBuffer );

DX_OUTPUTDEBUGSTRING( DX_STR_WRAP("\n") );

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
if( bPopMsgBox )
{
    DX_CHAR strBufferFile[MAX_PATH];
    DX_STRCPY_S( strBufferFile, MAX_PATH, DX_STR_WRAP("") );
    if( strFile )
        DX_STRCPY_S( strBufferFile, MAX_PATH, strFile );

    DX_CHAR strBufferMsg[1024];
    DX_STRCPY_S( strBufferMsg, 1024, DX_STR_WRAP("") );
    if( nMsgLen > 0 )
        DX_SPRINTF_S( strBufferMsg, 1024, DX_STR_WRAP("Calling: " STR_FMT_SPEC "\n"), strMsg );

    DX_SPRINTF_S( strBuffer, BUFFER_SIZE, DX_STR_WRAP("File: " STR_FMT_SPEC "\nLine: " STR_FMT_SPEC "\nError Code: " STR_FMT_SPEC "\n" STR_FMT_SPEC "Do you want to debug the application?"),
                strBufferFile, strBufferLine, strBufferError, strBufferMsg );

    int nResult = DX_MESSAGEBOX( GetForegroundWindow(), strBuffer, DX_STR_WRAP("Unexpected error encountered"), MB_YESNO | MB_ICONERROR );
    if( nResult == IDYES )
        DebugBreak();
}
#else
UNREFERENCED_PARAMETER(bPopMsgBox);
#endif

return hr;

#endif // defined(DX_CHAR) || defined(DX_SPRINTF_S) || defined(DX_OUTPUTDEBUGSTRING)
