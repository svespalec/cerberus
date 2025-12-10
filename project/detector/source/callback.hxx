#pragma once

#define MAX_INSTRUMENTATION_CALLBACKS 64

typedef struct _INSTRUMENTATION_CALLBACK_INFORMATION {
  ULONG version;
  ULONG reserved;
  PVOID callback;
} INSTRUMENTATION_CALLBACK_INFORMATION, *PINSTRUMENTATION_CALLBACK_INFORMATION;

typedef VOID( WINAPI* VECTORED_INSTRUMENTATION_CALLBACK )( PCONTEXT previous_context );

namespace engine {
  // registered instrumentation callbacks
  inline VECTORED_INSTRUMENTATION_CALLBACK g_callbacks[ MAX_INSTRUMENTATION_CALLBACKS ];

  // spinlock for thread-safe callback list access
  inline CHAR g_callback_list_spinlock = NULL;

  // initialize instrumentation callbacks for the current process
  NTSTATUS initialize( VOID );

  // add a vectored instrumentation callback to the global list of callbacks
  // returns TRUE if the function succeeds, FALSE if it fails
  BOOLEAN add_callback( VECTORED_INSTRUMENTATION_CALLBACK callback );

  // remove a vectored instrumentation callback from the global list of callbacks
  // returns TRUE if the function succeeds, FALSE if it fails
  BOOLEAN remove_callback( LPVOID callback );

  // handler called by asm stub on each syscall return
  extern "C" VOID callback_handler( PCONTEXT previous_context );
} // namespace engine

extern "C" {
  NTSTATUS NTSYSAPI NtSetInformationProcess( HANDLE, PROCESS_INFORMATION_CLASS, LPVOID, DWORD );

  // asm entry point for instrumentation callback
  VOID callback_entry( VOID );
}
