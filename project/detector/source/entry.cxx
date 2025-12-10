#include <source/globals.hxx>

void main_thread( ) {
  std::println( "[+] base: {:p}", reinterpret_cast< void* >( globals::base ) );

  // set instrumentation callback here
  //

  FreeLibraryAndExitThread( globals::module_handle, 0 );
}

BOOL APIENTRY DllMain( HMODULE handle, DWORD reason, [[maybe_unused]] LPVOID reserved ) {
  if ( reason == DLL_PROCESS_ATTACH ) {
    globals::module_handle = handle;

    DisableThreadLibraryCalls( handle );
    CreateThread( nullptr, 0, reinterpret_cast< LPTHREAD_START_ROUTINE >( main_thread ), nullptr, 0, nullptr );
  }

  return TRUE;
}
