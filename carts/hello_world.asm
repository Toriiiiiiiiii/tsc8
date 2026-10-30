.FEATURE STRING_ESCAPES
.SEGMENT "CART"

start:	ldx #$00
@lp:	lda msg,x
		beq @hlt
		sta $8000
		inx
		jmp @lp
@hlt:	jmp $e100


.SEGMENT "CARTDAT"

msg: .ASCIIZ "Hello, World!\n"
