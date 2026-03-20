EXTERN callback_handler : PROC

.CODE

; capture a partial version of the current thread context for our instrumentation callback
capture_partial_context MACRO
  pushfq

  ; volatile registers
  mov     [rsp+080h], rax
  mov     [rsp+088h], rcx
  mov     [rsp+090h], rdx
  mov     [rsp+0C0h], r8
  mov     [rsp+0C8h], r9
  mov     [rsp+0D0h], r10
  mov     [rsp+0D8h], r11

  movaps  xmmword ptr [rsp+1A8h], xmm0
  movaps  xmmword ptr [rsp+1B8h], xmm1
  movaps  xmmword ptr [rsp+1C8h], xmm2
  movaps  xmmword ptr [rsp+1D8h], xmm3
  movaps  xmmword ptr [rsp+1E8h], xmm4
  movaps  xmmword ptr [rsp+1F8h], xmm5

  ; non-volatile registers
  mov     [rsp+098h], rbx
  mov     [rsp+0A8h], rbp
  mov     [rsp+0B0h], rsi
  mov     [rsp+0B8h], rdi
  mov     [rsp+0E0h], r12
  mov     [rsp+0E8h], r13
  mov     [rsp+0F0h], r14
  mov     [rsp+0F8h], r15

  ; floating point state
  fnstcw  word ptr [rsp+108h]
  mov     dword ptr [rsp+10Ah], 0

  ; non-volatile xmm registers
  movaps  xmmword ptr [rsp+208h], xmm6
  movaps  xmmword ptr [rsp+218h], xmm7
  movaps  xmmword ptr [rsp+228h], xmm8
  movaps  xmmword ptr [rsp+238h], xmm9
  movaps  xmmword ptr [rsp+248h], xmm10
  movaps  xmmword ptr [rsp+258h], xmm11
  movaps  xmmword ptr [rsp+268h], xmm12
  movaps  xmmword ptr [rsp+278h], xmm13
  movaps  xmmword ptr [rsp+288h], xmm14
  movaps  xmmword ptr [rsp+298h], xmm15
  stmxcsr dword ptr [rsp+120h]
  stmxcsr dword ptr [rsp+03Ch]

  ; original rsp from TEB->InstrumentationCallbackPreviousSp
  mov     rax, qword ptr gs:[2E0h]
  mov     qword ptr gs:[2E0h], 0
  mov     [rsp+0A0h], rax

  ; original rip
  mov     [rsp+100h], r10

  ; eflags
  mov     eax, [rsp]
  mov     [rsp+04Ch], eax

  ; CONTEXT_FULL
  mov     dword ptr [rsp+038h], 10000Bh

  add     rsp, 8
ENDM

callback_entry PROC
  ; store original rsp in TEB->InstrumentationCallbackPreviousSp
  mov     qword ptr gs:[2E0h], rsp

  ; align stack to 16-byte boundary
  and     rsp, 0FFFFFFFFFFFFFFF0h

  ; allocate space for CONTEXT and capture processor state
  sub     rsp, 4D0h
  capture_partial_context

  ; call into c++ handler
  mov     rcx, rsp
  sub     rsp, 20h
  call    callback_handler
callback_entry ENDP

END
