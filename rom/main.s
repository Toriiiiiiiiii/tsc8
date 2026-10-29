.FEATURE STRING_ESCAPES

.SEGMENT "ZPG"
.SEGMENT "CODE"

start:	; Check for a program vector in RAM
        ; The vector should be stored at $7ffe.
		; If the vector != $7ffe, jump to it.
		lda $7ffe
		
		; Check if the two bytes of the vector are equal.
		; If they are, check for 0. If not, jump to the vector.
		cmp $7fff
		bne @skip

		cmp #$00
		beq noprg

@skip:  jmp ($7ffe)

noprg:  lda #<noprg_msg
		sta $0002
		lda #>noprg_msg
		sta $0003
		jsr puts

		jmp mon

puts:   ; Prints a string to serial terminal.
		; Pointer to the string should be stored at $0002
		ldy #$00
@loop:	lda ($0002),y
		beq @done

		sta $8000
		iny
		jmp @loop

@done:  rts

.INCLUDE "monitor.s"


.SEGMENT "DATA"

noprg_msg: .ASCIIZ "No program found in RAM (Vector $7ffe = $0000).\r\nDefaulting to machine language monitor.\r\n"


; Define the vectors for the rom.
;  $fffa  = NMI vector
;  $fffc  = RESET vector
;  $fffe  = IRQ vector
.SEGMENT "VECTOR"

nmi_vec: .WORD 0
rst_vec: .WORD start
irq_vec: .WORD 0
