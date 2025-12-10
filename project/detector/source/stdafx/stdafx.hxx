#pragma once

#include <print>
#include <optional>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winnt.h>
#include <winternl.h>
#include <Psapi.h>

#pragma comment( lib, "ntdll.lib" )
#pragma comment( lib, "psapi.lib" )

