.FEATURE STRING_ESCAPES

.SEGMENT "ZPG"
.SEGMENT "CODE"

start:	lda #<noprg_msg
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

noprg_msg: .ASCIIZ "TSC-8 ROM v0.1 ML Monitor.\r\nType 'h' then enter for a list of commands.\r\n"


; Define the vectors for the rom.
;  $fffa  = NMI vector
;  $fffc  = RESET vector
;  $fffe  = IRQ vector
.SEGMENT "VECTOR"

nmi_vec: .WORD 0
rst_vec: .WORD start
irq_vec: .WORD 0
