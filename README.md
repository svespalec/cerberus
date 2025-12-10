## syscall-detector

a lightweight antivirus proof-of-concept that detects and blocks malware using direct syscalls via windows instrumentation callbacks.

## how it works

registers a process instrumentation callback that intercepts every kernel→user transition. on each syscall return, validates that the return address (r10) points to legitimate system modules (ntdll.dll / win32u.dll). if not, thesyscall originated from unauthorized code (direct syscall) and the process is terminated.

## detects

- inline syscall stubs
- copied/stolen syscall stubs
- manual `mov eax, SSN; syscall` sequences
- syswhispers1-style direct syscalls

## limitations

- does not detect indirect syscalls (jmp-to-ntdll techniques)
- most edrs mitigate indirect syscalls via inline hooks that destroy the syscall stub

 ## credits

- [@Peribunt](https://github.com/Peribunt) for instrumentation callback reference