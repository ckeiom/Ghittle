[ORG 0x00]	; Start address of code: 0x00
[BITS 16]	; 16bits code below

SECTION .text

jmp 0x07C0:START	; Set cs=0x07c0, jmp to START

; Will be used for image generator (mkimage)
SECTOR_COUNT:		dw 0x2
KERN32_SECTOR_COUNT:	dw 0x2

START:

mov ax, 0x07C0		; DS for data
mov ds, ax
mov ax, 0xB800		; ES for Video memory
mov es, ax
mov ax, 0x0000		; SS for stack: from 0x10000
mov ss, ax
mov sp, 0xFFFE
mov bp, 0xFFFE		

call CLEAR_SCREEN

push word START_MESSAGE ; Message
push word 1				; Y-axis   Push 3 parameters
push word 0				; X-axis
call PRINT_MESSAGE
add sp, 6				; Restore stack state which is changed by above 3 pushes

mov si,0x1000			; Set ES 0x1000 < address to copy 32bit kernel image
mov es, si

mov bl, byte [0x1FC]	; Here to check if PM boot
						; We use address (0x7C00+0x1FC) to check
						; if current boot mode is NM boot
						; bootloader image looks like below
						;
						;           |----------------------| 
						; 0x7C00 >  | START FROM HERE .....|
						; 0x7C10 >  | .....................|
						; ...				
						; 0x7DF0 >  | ..........[A][][B][C]|
						;			|----------------------|
						; A: Persistent boot checker
						; B: Magic number 0x55
						; C: Magic number 0xAA

cmp bl, 0				; If not persistent boot mode, read data from floppy disk				
jne READ_DATA

mov bx, 0x0000
mov di, word[SECTOR_COUNT]	; Sector location of OS image

READ_DATA:	

cmp di,0	
je READ_DATA_END			; If persistent boot, di must be 0

sub di, 1

mov ah, 0x02				; BIOS service number 2: Read sector
mov al, 0x01				; The number of Sector to read
mov ch, byte[TRACK_NUM]		; The position of track to read
mov cl, byte[SECTOR_NUM]	; The position of sector to read
mov dh, byte[HEAD_NUM]		; The position of head to read
mov dl, 0x00				; 0: Floppy disk
int 0x13					; BIOS disk service
jc PANIC

add si, 0x0020	; Add 512 bytes
mov es, si

add byte[SECTOR_NUM], 0x01
mov al, byte[SECTOR_NUM]
cmp al, 19			; Until reach at sector 18
jl READ_DATA

xor byte[HEAD_NUM], 0x01	; Turn head over
mov byte[SECTOR_NUM], 0x01
cmp byte[HEAD_NUM], 0x00
jne READ_DATA

add byte[TRACK_NUM], 0x01
jmp READ_DATA

READ_DATA_END:


push word END_MESSAGE
push word 2
push word 0
call PRINT_MESSAGE
add sp,6



jmp 0x1000:0x0000		; jump to 0x10000



PANIC:

mov si, 0xB800
mov ds, si
mov byte [3990], 'P'
mov byte [3992], 'A'
mov byte [3994], 'N'
mov byte [3996], 'I'
mov byte [3998], 'C'
jmp $



CLEAR_SCREEN:

push bp
mov bp, sp
push si
push es
push ds
push ax

mov si, 0xB800
mov es, si
mov si, 0x07C0
mov ds, si
mov al, byte [0x1FC]
cmp al, 0
je FIRST_BOOT
mov al, 3			; During second boot, BGC will be blue(3)

FIRST_BOOT:
mov si,0
CLEAR_SCREEN_LOOP:
mov byte [es:si],0
mov byte [es:si+1],al
add si,2
cmp si, 80*25*2		; Screen size 80 charactors * 25 lines
jl CLEAR_SCREEN_LOOP

pop ax
pop ds
pop es
pop si
pop bp

ret

PRINT_MESSAGE:

push bp
mov bp,sp

push ax
push si
push cx
push di
push es

mov ax, 0xB800		; ES for Video memory
mov es, ax

mov ax, word [bp+6]	; Y index: line * 160	
mov si, 160
mul si
mov di, ax	

mov ax, word [bp+4]	; X index col*2
mov si,2
mul si
add di, ax

mov si, word [bp+8]	; Addr of string

PRINT_MESSAGE_LOOP:
mov cl, byte [si]
cmp cl, 0
je PRINT_MESSAGE_LOOP_END
mov byte [es: di], cl
add di,2
add si,1
jmp PRINT_MESSAGE_LOOP
PRINT_MESSAGE_LOOP_END:

pop es
pop di
pop cx
pop si
pop ax
pop bp

ret


; Data but included in text section
START_MESSAGE: 		db 'Ghittle Bootloader Start',0
END_MESSAGE		db 'Ghittle Bootloader End',0

SECTOR_NUM: 	db 0x02
HEAD_NUM: 		db 0
TRACK_NUM:		db 0


times 510 - ( $-$$ ) db 0x00	; Fill 0x00 from here to 510 byte position 
				; $: current line
				; $$: section start line

db 0x55		; Magic numbers 0x55, 0xAA at 511, 512 bytes position
db 0xAA


