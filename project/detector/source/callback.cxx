#include "callback.hxx"

NTSTATUS engine::initialize( VOID ) {
  RtlZeroMemory( g_callbacks, sizeof( g_callbacks ) );

  INSTRUMENTATION_CALLBACK_INFORMATION callback_info { .callback = callback_entry };

  return NtSetInformationProcess( ( HANDLE ) -1, ( PROCESS_INFORMATION_CLASS ) 40, &callback_info, sizeof( callback_info ) );
}

BOOLEAN engine::add_callback( VECTORED_INSTRUMENTATION_CALLBACK callback ) {
  // acquire spinlock to prevent race conditions in multi-threaded adds and removes
  while ( _InterlockedExchange8( &g_callback_list_spinlock, TRUE ) == TRUE ) {
    _mm_pause( );
  }

  BOOLEAN result = FALSE;

  for ( UINT32 i = NULL; i < MAX_INSTRUMENTATION_CALLBACKS; i++ ) {
    // search for the next available callback slot to occupy with our callback
    if ( g_callbacks[ i ] == NULL ) {
      g_callbacks[ i ] = callback;
      result           = TRUE;
      break;
    }
  }

  // release the spinlock
  g_callback_list_spinlock = FALSE;

  return result;
}

BOOLEAN engine::remove_callback( LPVOID callback ) {
  // acquire spinlock to prevent race conditions in multi-threaded adds and removes
  while ( _InterlockedExchange8( &g_callback_list_spinlock, TRUE ) == TRUE ) {
    _mm_pause( );
  }

  BOOLEAN result = FALSE;

  for ( UINT32 i = NULL; i < MAX_INSTRUMENTATION_CALLBACKS; i++ ) {
    // search for the callback that matches the function pointer to invalidate the slot
    if ( g_callbacks[ i ] == callback ) {
      g_callbacks[ i ] = NULL;
      result           = TRUE;
      break;
    }
  }

  // release the spinlock
  g_callback_list_spinlock = FALSE;

  return result;
}

VOID engine::callback_handler( PCONTEXT previous_context ) {
  // TEB->InstrumentationCallbackDisabled prevents recursion
  CHAR* callback_disabled = ( CHAR* ) ( __readgsqword( 0x30 ) + 0x2EC );

  // prevent recursion inside our instrumentation callback
  if ( _InterlockedExchange8( callback_disabled, TRUE ) == FALSE ) {
    for ( UINT32 i = NULL; i < MAX_INSTRUMENTATION_CALLBACKS; i++ ) {
      if ( g_callbacks[ i ] != NULL ) {
        g_callbacks[ i ]( previous_context );
      }
    }

    // remove the recursion lock
    *callback_disabled = FALSE;
  }

  // restore context after the instrumentation callback
  RtlRestoreContext( previous_context, NULL );
}
