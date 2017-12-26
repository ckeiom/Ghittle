[BITS 64]

SECTION .text

global load_gdtr, load_tss, load_idtr, read_rflags
global read_tsc
global context_switch

%macro SAVE_CONTEXT 0
push rbp
push rax
push rbx
push rcx
push rdx
push rdi
push rsi
push r8
push r9
push r10
push r11
push r12
push r13
push r14
push r15

mov ax, ds
push rax
mov ax,es
push rax
push fs
push gs

%endmacro

%macro LOAD_CONTEXT 0 
pop gs
pop fs
pop rax
mov es, ax
pop rax
mov ds, ax
pop r15
pop r14
pop r13
pop r12
pop r11
pop r10
pop r9
pop r8
pop rsi
pop rdi
pop rdx
pop rcx
pop rbx
pop rax
pop rbp
%endmacro

; RDI: current task context
context_switch:
push rbp
mov rbp, rsp

pushfq
cmp rdi, 0
je .load_context
popfq

push rax
mov ax, ss
mov qword[ rdi+(23*8) ], rax

mov rax, rbp
add rax, 16  ; exclude RBP and retern addr
mov qword[rdi + (22*8)], rax

pushfq
pop rax
mov qword[rdi + (21*8)], rax

mov ax, cs
mov qword[rdi + (20*8)], rax

mov rax, qword[rbp + 8]	; Return address
mov qword[rdi + (19*8)], rax

pop rax
pop rbp

add rdi, (19*8)
mov rsp, rdi
sub rdi, (19*8)

SAVE_CONTEXT
.load_context:
mov rsp, rsi
LOAD_CONTEXT
iretq

load_gdtr:

lgdt [rdi]
ret

load_tss:

ltr di
ret

load_idtr:

lidt [rdi]
ret

read_rflags:

pushfq
pop rax
ret

read_tsc:

push rdx
rdtsc
shl rdx, 32
or rax, rdx
pop rdx
ret

