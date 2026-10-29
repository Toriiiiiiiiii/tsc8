.FEATURE STRING_ESCAPES
.SEGMENT "CART"

PUTS = $e01f

start:	lda #<msg
		sta $02
		lda #>msg
		sta $03
		jsr PUTS
@hlt:	jmp @hlt


.SEGMENT "CARTDAT"

msg: .ASCIIZ "Hello, World!\n"


.SEGMENT "CARTVEC"
.WORD start
