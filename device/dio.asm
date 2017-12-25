[BITS 64]

SECTION .text

global in_b, out_b

in_b:

push rdx
mov rdx, rdi	;rdi: port number 
mov rax, 0
in al, dx
pop rdx
ret

out_b:

push rdx
push rax
mov rdx, rdi	;rdi: port number
mov rax, rsi	;rsi: value 
out dx, al
pop rax
pop rdx
ret
