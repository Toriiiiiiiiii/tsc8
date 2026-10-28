.FEATURE STRING_ESCAPES

start:	; Check for a program vector in RAM
        ; The vector should be stored at $0000.
		; If the vector != $0000, jump to it.
		lda $0000
		
		; Check if the two bytes of the vector are equal.
		; If they are, check for 0. If not, jump to the vector.
		cmp $0001
		bne @skip

		cmp #$00
		beq noprg

@skip:  jmp ($0000)

noprg:  lda #<noprg_msg
		sta $0002
		lda #>noprg_msg
		sta $0003
		jsr puts

halt:	lda $8001
		sta $8000
		jmp halt

puts:   ; Prints a string to serial terminal.
		; Pointer to the string should be stored at $0002
		ldy #$00
@loop:	lda ($0002),y
		beq @done

		sta $8000
		iny
		jmp @loop

@done:  rts


.SEGMENT "DATA"

noprg_msg: .ASCIIZ "No program found in RAM.\r\nPlease write a program to memory, write the address to $0000 and reset.\r\n"


; Define the vectors for the rom.
;  $fffa  = NMI vector
;  $fffc  = RESET vector
;  $fffe  = IRQ vector
.SEGMENT "VECTOR"

nmi_vec: .WORD 0
rst_vec: .WORD start
irq_vec: .WORD 0
