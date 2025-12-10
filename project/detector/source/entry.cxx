#include <source/callback.hxx>

// get module name containing an address
static const char* get_module_name( void* address ) {
  static char module_name[ MAX_PATH ];

  HMODULE module = nullptr;

  if ( !GetModuleHandleExA( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, static_cast< LPCSTR >( address ), &module ) )
    return "unknown";

  if ( !GetModuleFileNameA( module, module_name, MAX_PATH ) )
    return "unknown";

  // get just the filename
  char* name = strrchr( module_name, '\\' );

  return name ? name + 1 : module_name;
}

// check if address falls within a module's memory range
static BOOLEAN is_in_module( void* address, const char* module_name ) {
  HMODULE module = GetModuleHandleA( module_name );

  if ( !module )
    return FALSE;

  MODULEINFO info { };

  if ( !GetModuleInformation( GetCurrentProcess( ), module, &info, sizeof( info ) ) )
    return FALSE;

  auto base = reinterpret_cast< uintptr_t >( info.lpBaseOfDll );
  auto addr = reinterpret_cast< uintptr_t >( address );

  return addr >= base && addr < base + info.SizeOfImage;
}

// callback invoked on every syscall return
static VOID syscall_detector( PCONTEXT ctx ) {
  void* return_addr = reinterpret_cast< void* >( ctx->R10 );

  if ( !is_in_module( return_addr, "ntdll.dll" ) && !is_in_module( return_addr, "win32u.dll" ) ) {
    std::println( "[!] direct syscall detected!" );
    std::println( "[*] return address: {:p}", return_addr );
    std::println( "[*] origin module: {}", get_module_name( return_addr ) );
    std::println( "[*] thread id: {}", GetCurrentThreadId( ) );
    std::println( "[*] rip: {:p}", reinterpret_cast< void* >( ctx->Rip ) );
    std::println( "[*] rsp: {:p}", reinterpret_cast< void* >( ctx->Rsp ) );
    std::println( "[*] rax (syscall): {:#x}", ctx->Rax );
    std::println( "[!] terminating process..." );

    TerminateProcess( HANDLE( -1 ), 0xDEAD );
  }
}

void main_thread( ) {
  engine::initialize( );
  engine::add_callback( syscall_detector );

  // keep dll loaded. don't free since callback is active
  while ( true ) {
    Sleep( 1000 );
  }
}

BOOL APIENTRY DllMain( HMODULE handle, DWORD reason, [[maybe_unused]] LPVOID reserved ) {
  if ( reason == DLL_PROCESS_ATTACH ) {
    DisableThreadLibraryCalls( handle );
    CreateThread( nullptr, 0, reinterpret_cast< LPTHREAD_START_ROUTINE >( main_thread ), nullptr, 0, nullptr );
  }

  return TRUE;
}
