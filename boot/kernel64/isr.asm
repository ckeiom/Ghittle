[BITS 64]

SECTION .text

extern exception_handler, interrupt_handler, keyboard_handler
extern timer_handler
extern hdd_handler

global ISR_div0, ISR_debug, ISR_nmi, ISR_breakpoint, ISR_overflow
global ISR_boundrange, ISR_invopcode, ISR_nodev, ISR_df
global ISR_copoverrun, ISR_invtss, ISR_noseg, ISR_sf, ISR_gp, ISR_pf, ISR_r15
global ISR_fpu, ISR_align, ISR_machine, ISR_simd, ISR_exception

global ISR_timer, ISR_keyboard, ISR_slave, ISR_serial2, ISR_serial1, ISR_parallel2
global ISR_floppy, ISR_parallel1, ISR_rtc, ISR_res, ISR_nouse1, ISR_nouse2
global ISR_mouse, ISR_cop, ISR_hdd1, ISR_hdd2, ISR_interrupt


%macro save_context 0

push rbp
mov rbp, rsp
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
mov ax, es
push rax
push fs
push gs

mov ax, 0x10
mov ds, ax
mov es, ax
mov gs, ax
mov fs, ax

%endmacro

%macro load_context 0

pop gs
pop fs
pop rax
mov es,ax
pop rax
mov ds,ax
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

ISR_div0:

save_context
mov rdi, 0
call exception_handler
load_context
iretq


ISR_debug:

save_context
mov rdi, 1
call exception_handler
load_context
iretq

ISR_nmi:

save_context
mov rdi, 2
call exception_handler
load_context
iretq

ISR_breakpoint:

save_context
mov rdi, 3
call exception_handler
load_context
iretq

ISR_overflow:

save_context
mov rdi, 4
call exception_handler
load_context
iretq

ISR_boundrange:

save_context
mov rdi, 5
call exception_handler
load_context
iretq

ISR_invopcode:

save_context
mov rdi, 6
call exception_handler
load_context
iretq


ISR_nodev:

save_context
mov rdi, 7
call exception_handler
load_context
iretq

ISR_df:

save_context
mov rdi, 8
mov rsi, qword[rbp+8]
call exception_handler
load_context
add rsp, 8
iretq

ISR_copoverrun:

save_context
mov rdi, 9
call exception_handler
load_context
iretq

ISR_invtss:

save_context
mov rdi, 10
mov rsi, qword [rbp+8]
call exception_handler
load_context
add rsp, 8
iretq

ISR_noseg:

save_context
mov rdi, 11
mov rsi, qword [rbp+8]
call exception_handler
load_context
add rsp, 8
iretq

ISR_sf:

save_context
mov rdi, 12
mov rsi, qword [rbp+8]
call exception_handler
load_context
add rsp, 8
iretq

ISR_gp:

save_context
mov rdi, 13
mov rsi, qword [rbp+8]
call exception_handler
load_context
add rsp, 8
iretq

ISR_pf:

save_context
mov rdi, 14
mov rsi, qword [rbp+8]
call exception_handler
load_context
add rsp, 8
iretq

ISR_r15:

save_context
mov rdi, 15
call exception_handler
load_context
iretq

ISR_fpu:

save_context
mov rdi, 16
call exception_handler
load_context
iretq

ISR_align:

save_context
mov rdi, 17
mov rsi, qword [rbp+8]
call exception_handler
load_context
add rsp, 8
iretq

ISR_machine:

save_context
mov rdi, 18
call exception_handler
load_context
iretq

ISR_simd:

save_context
mov rdi, 19
call exception_handler
load_context
iretq

ISR_exception:

save_context
mov rdi, 20
call exception_handler
load_context
iretq

ISR_timer:

save_context
mov rdi, 32
call timer_handler
load_context
iretq

ISR_keyboard:

save_context
mov rdi, 33
call keyboard_handler
load_context
iretq

ISR_slave:

save_context
mov rdi, 34
call interrupt_handler
load_context
iretq

ISR_serial2:

save_context
mov rdi, 35
call interrupt_handler
load_context
iretq

ISR_serial1:

save_context
mov rdi, 36
call interrupt_handler
load_context
iretq

ISR_parallel2:

save_context
mov rdi, 37
call interrupt_handler
load_context
iretq

ISR_floppy:

save_context
mov rdi, 38
call interrupt_handler
load_context
iretq

ISR_parallel1:

save_context
mov rdi, 39
call interrupt_handler
load_context
iretq

ISR_rtc:

save_context
mov rdi, 40
call interrupt_handler
load_context
iretq

ISR_res:

save_context
mov rdi, 41
call interrupt_handler
load_context
iretq

ISR_nouse1:

save_context
mov rdi, 42
call interrupt_handler
load_context
iretq

ISR_nouse2:

save_context
mov rdi, 43
call interrupt_handler
load_context
iretq

ISR_mouse:

save_context
mov rdi, 44
call interrupt_handler
load_context
iretq

ISR_cop:

save_context
mov rdi, 45
call interrupt_handler
load_context
iretq

ISR_hdd1:

save_context
mov rdi, 46
call hdd_handler
load_context
iretq

ISR_hdd2:

save_context
mov rdi, 47
call hdd_handler
load_context
iretq

ISR_interrupt:

save_context
mov rdi, 48
call interrupt_handler
load_context
iretq


