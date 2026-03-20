#pragma once

#define MAX_INSTRUMENTATION_CALLBACKS 64

typedef struct _INSTRUMENTATION_CALLBACK_INFORMATION {
  ULONG version;
  ULONG reserved;
  PVOID callback;
} INSTRUMENTATION_CALLBACK_INFORMATION, *PINSTRUMENTATION_CALLBACK_INFORMATION;

typedef void( WINAPI* VECTORED_INSTRUMENTATION_CALLBACK )( PCONTEXT previous_context );

namespace engine {
  // registered instrumentation callbacks
  inline VECTORED_INSTRUMENTATION_CALLBACK g_callbacks[ MAX_INSTRUMENTATION_CALLBACKS ];

  // spinlock for thread-safe callback list access
  inline char g_callback_list_spinlock = false;

  // initialize instrumentation callbacks for the current process
  NTSTATUS initialize( );

  bool add_callback( VECTORED_INSTRUMENTATION_CALLBACK callback );
  bool remove_callback( VECTORED_INSTRUMENTATION_CALLBACK callback );

  // handler called by asm stub on each syscall return
  extern "C" void callback_handler( PCONTEXT previous_context );
} // namespace engine

extern "C" {
  NTSTATUS NTSYSAPI NtSetInformationProcess( HANDLE, PROCESS_INFORMATION_CLASS, LPVOID, DWORD );

  void callback_entry( );
}
