#include <Windows.h>
#include <print>

int main( ) {
  HMODULE detector = LoadLibraryA( "detector.dll" );

  if ( detector == NULL ) {
    std::println( "[-] failed to load detector module" );
    return -1;
  }

  std::println( "[*] loaded detector module!" );
}