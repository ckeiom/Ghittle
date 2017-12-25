[BITS 64]

SECTION .text

global test_set

test_set:
mov rax, rsi
lock cmpxchg byte [rdi], dl
je LOCK_SUCCESS

LOCK_FAIL:
mov rax, 0x00
ret

LOCK_SUCCESS:
mov rax, 0x01
ret

