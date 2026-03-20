#include <Windows.h>
#include <print>

//
// direct syscall stub
//
static unsigned char syscall_stub[] = {
  0x4C, 0x8B, 0xD1,             // mov r10, rcx
  0xB8, 0x00, 0x00, 0x00, 0x00, // mov eax, <ssn>
  0x0F, 0x05,                   // syscall
  0xC3                          // ret
};

using nt_close_t = NTSTATUS( __stdcall* )( HANDLE );

// get syscall number from ntdll export
static DWORD get_ssn( const char* func_name )
{
  HMODULE ntdll = GetModuleHandleA( "ntdll.dll" );

  if ( !ntdll )
    return -1;

  auto func = static_cast< unsigned char* >( static_cast< void* >( GetProcAddress( ntdll, func_name ) ) );

  if ( !func )
    return -1;

  // ntdll stub: mov r10, rcx; mov eax, <ssn>
  if ( func[ 0 ] == 0x4C && func[ 1 ] == 0x8B && func[ 2 ] == 0xD1 && func[ 3 ] == 0xB8 )
    return *reinterpret_cast< DWORD* >( &func[ 4 ] );

  return -1;
}

static bool load_detector( )
{
  HMODULE detector = LoadLibraryA( "detector.dll" );

  if ( !detector )
  {
    std::println( "[-] failed to load detector module" );
    return false;
  }

  std::println( "[*] loaded detector module" );

  return true;
}

static void test_legit_syscall( )
{
  std::println( "[*] doing legit ntdll call via GetModuleHandleA..." );

  HMODULE ntdll = GetModuleHandleA( "ntdll.dll" );

  std::println( "[+] legit call succeeded! ntdll: {:p}", static_cast< void* >( ntdll ) );
}

static void test_direct_syscall( )
{
  DWORD ssn = get_ssn( "NtClose" );

  if ( ssn == -1 )
  {
    std::println( "[-] failed to get ssn" );
    return;
  }

  std::println( "[*] NtClose syscall number: {:#x}", ssn );
  std::println( "[*] executing direct syscall..." );

  // patch ssn into stub
  *reinterpret_cast< DWORD* >( &syscall_stub[ 4 ] ) = ssn;

  // make stub executable
  DWORD old_protect { };
  VirtualProtect( syscall_stub, sizeof( syscall_stub ), PAGE_EXECUTE_READWRITE, &old_protect );

  // call direct syscall with invalid handle which should trigger detector
  auto nt_close = reinterpret_cast< nt_close_t >( &syscall_stub );

  nt_close( HANDLE( -1 ) );

  // if we get here, detector didn't catch it
  std::println( "[-] direct syscall was NOT detected!" );
}

int main( )
{
  if ( !load_detector( ) )
    return -1;

  test_legit_syscall( );

  std::println( "[*] triggering direct syscall in 3 seconds..." );

  Sleep( 3000 );

  test_direct_syscall( );

  std::println( "[*] press enter to exit..." );
  std::getchar( );
}
