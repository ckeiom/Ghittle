[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x1000:START

SECTOR_COUNT: dw 0x0000
MAX_SECTOR equ 1024


START:
mov ax, cs
mov ds, ax
mov es, ax


mov ax, 0x2401
int 0x15

jc A20_BIOS_ERROR
jmp A20_SUCCESS

A20_BIOS_ERROR:
in al, 0x92
or al, 0x02
and al, 0xFE
out 0x92, al

A20_SUCCESS:

cli
lgdt [GDTR]		; Load GDTR

mov eax, 0x4000003B	; 32bits protection mode
mov cr0, eax
jmp dword 0x18:(PROTECTION_MODE - $$ +0x10000 )

[BITS 32]
PROTECTION_MODE:

mov ax, 0x20
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax

mov esp, 0xFFFE
mov ebp, 0xFFFE


jmp dword 0x18:0x10200
SIMPLE_DEBUG:
mov byte[0xB8444],'D'

align 8, db 0
dw 0x0000

GDTR:
dw GDT_END - GDT - 1
dd ( GDT - $$ + 0x10000 )

GDT:
NULL_DESCRIPTOR:
dw 0x0000
dw 0x0000
db 0x00
db 0x00
db 0x00
db 0x00

CODE_DESCRIPTOR_64:
dw 0xFFFF	; Limit
dw 0x0000
db 0x00
db 0x9A
db 0xAF	
db 0x00		; Base[31:24]

DATA_DESCRIPTOR_64:
dw 0xFFFF
dw 0x0000
db 0x00
db 0x92
db 0xAF
db 0x00

CODE_DESCRIPTOR:
dw 0xFFFF	; Limit
dw 0x0000
db 0x00
db 0x9A
db 0xCF	
db 0x00		; Base[31:24]

DATA_DESCRIPTOR:
dw 0xFFFF
dw 0x0000
db 0x00
db 0x92
db 0xCF
db 0x00


GDT_END:

times 512 - ( $-$$ ) db 0x00
