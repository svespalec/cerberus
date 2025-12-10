#include <Windows.h>
#include <print>

// direct syscall stub - mov r10, rcx; mov eax, ssn; syscall; ret
static unsigned char syscall_stub[] = {
  0x4C, 0x8B, 0xD1,             // mov r10, rcx
  0xB8, 0x00, 0x00, 0x00, 0x00, // mov eax, <ssn>
  0x0F, 0x05,                   // syscall
  0xC3                          // ret
};

// get syscall number from ntdll export
static DWORD get_ssn( const char* func_name ) {
  HMODULE ntdll = GetModuleHandleA( "ntdll.dll" );

  if ( !ntdll )
    return -1;

  auto func = reinterpret_cast< unsigned char* >( GetProcAddress( ntdll, func_name ) );

  if ( !func )
    return -1;

  // ntdll stub: mov r10, rcx; mov eax, <ssn>
  // bytes: 4C 8B D1 B8 XX XX XX XX
  if ( func[ 0 ] == 0x4C && func[ 1 ] == 0x8B && func[ 2 ] == 0xD1 && func[ 3 ] == 0xB8 ) {
    return *reinterpret_cast< DWORD* >( &func[ 4 ] );
  }

  return -1;
}

int main( ) {
  HMODULE detector = LoadLibraryA( "detector.dll" );

  if ( detector == NULL ) {
    std::println( "[-] failed to load detector module" );
    return -1;
  }

  std::println( "[*] loaded detector module!" );

  // legit ntdll call through winapi
  std::println( "[*] doing legit ntdll call via GetModuleHandleA..." );

  HMODULE ntdll = GetModuleHandleA( "ntdll.dll" );

  std::println( "[+] legit call succeeded! ntdll: {:p}", static_cast< void* >( ntdll ) );
  std::println( "[*] triggering direct syscall in 3 seconds..." );

  Sleep( 3000 );

  // get NtClose syscall number
  DWORD ssn = get_ssn( "NtClose" );

  if ( ssn == -1 ) {
    std::println( "[-] failed to get ssn" );
    return -1;
  }

  std::println( "[*] NtClose SSN: {:#x}", ssn );
  std::println( "[*] executing direct syscall..." );

  // patch ssn into stub
  *reinterpret_cast< DWORD* >( &syscall_stub[ 4 ] ) = ssn;

  // make stub executable
  DWORD old_protect;
  VirtualProtect( syscall_stub, sizeof( syscall_stub ), PAGE_EXECUTE_READWRITE, &old_protect );

  // call direct syscall with invalid handle which should trigger detector
  auto direct_syscall = reinterpret_cast< NTSTATUS( __stdcall* )( HANDLE ) >( static_cast< void* >( syscall_stub ) );
  direct_syscall( reinterpret_cast< HANDLE >( static_cast< uintptr_t >( 0xDEADBEEF ) ) );

  // if we get here, detector didn't catch it
  std::println( "[-] direct syscall was NOT detected!" );

  system( "pause" );

  return 0;
}
