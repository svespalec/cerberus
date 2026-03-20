#include "callback.hxx"

// RAII spinlock guard to eliminate duplicate acquire/release logic
struct spinlock_guard
{
  char& lock;

  spinlock_guard( char& lock ) : lock( lock )
  {
    while ( _InterlockedExchange8( &lock, true ) == true )
    {
      _mm_pause( );
    }
  }

  ~spinlock_guard( )
  {
    _InterlockedExchange8( &lock, false );
  }
};

NTSTATUS engine::initialize( )
{
  RtlZeroMemory( g_callbacks, sizeof( g_callbacks ) );

  INSTRUMENTATION_CALLBACK_INFORMATION callback_info { .callback = callback_entry };

  return NtSetInformationProcess(
    reinterpret_cast< HANDLE >( -1 ),
    static_cast< PROCESS_INFORMATION_CLASS >( 40 ),
    &callback_info,
    sizeof( callback_info )
  );
}

bool engine::add_callback( VECTORED_INSTRUMENTATION_CALLBACK callback )
{
  spinlock_guard guard( g_callback_list_spinlock );

  for ( uint32_t i = 0; i < MAX_INSTRUMENTATION_CALLBACKS; i++ )
  {
    if ( g_callbacks[ i ] == nullptr )
    {
      g_callbacks[ i ] = callback;
      return true;
    }
  }

  return false;
}

bool engine::remove_callback( VECTORED_INSTRUMENTATION_CALLBACK callback )
{
  spinlock_guard guard( g_callback_list_spinlock );

  for ( uint32_t i = 0; i < MAX_INSTRUMENTATION_CALLBACKS; i++ )
  {
    if ( g_callbacks[ i ] == callback )
    {
      g_callbacks[ i ] = nullptr;
      return true;
    }
  }

  return false;
}

void engine::callback_handler( PCONTEXT previous_context )
{
  auto callback_disabled = reinterpret_cast< char* >( __readgsqword( 0x30 ) + 0x2EC );

  if ( _InterlockedExchange8( callback_disabled, true ) == false )
  {
    for ( uint32_t i = 0; i < MAX_INSTRUMENTATION_CALLBACKS; i++ )
    {
      if ( g_callbacks[ i ] != nullptr )
        g_callbacks[ i ]( previous_context );
    }

    *callback_disabled = false;
  }

  RtlRestoreContext( previous_context, nullptr );
}
