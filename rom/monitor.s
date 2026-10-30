keyboard_buffer = $04
keyboard_size   = $40

int_lo = $45
int_hi = $46

putint_buf = $47 ; 4 bytes

addr = putint_buf+4
max  = addr+2

tty_in = $8001
tty_out = $8000

mon:	jsr clear

		lda #'?'
		sta tty_out

		ldy #$00
gkey:	lda tty_in
		beq gkey

		cmp #$0a ; Newline
		beq docmd

		cmp #$08
		beq bsp

		sta keyboard_buffer,y
		sta tty_out
		iny
		cpy #keyboard_size
		beq mon
		jmp gkey

bsp:	sta tty_out
		dey
		bpl gkey
		iny
		jmp gkey

docmd:	sta tty_out

		ldy #$00
@chr:	jsr getch
		cmp #$20
		beq @chr

		cmp #'r'    ; (R)ead memory address
		beq readm

		cmp #'w'	; (W)rite memory address
		beq writem

		cmp #'e'    ; (E)xecute memory address
		beq execat

		cmp #'h'	; (H)elp
		beq help

		cmp #'l'	; (L)oad file
		beq load

		jmp mon

readm:	jsr doread
		jmp mon
writem: jsr getint
		cpx #0
		bne @dowr
		jmp mon

@dowr:	lda int_lo
		sta addr
		lda int_hi
		sta addr+1
		sty max
	
@lp:	ldy max
		jsr getint
		sty max
		ldy #0
		lda int_lo
		sta (addr),y
		inc addr
		cpx #0		
		bne @lp
		jmp mon

execat: jsr getint
		jmp (int_lo)

load:	jsr doload
		jmp mon	

help:	ldx #$00	
@lp:	lda helpmsg,x
		beq @dn
		sta $8000
		inx
		jmp @lp
@dn:	jmp mon

doread: jsr getint
		lda #1
		sta max

		lda int_lo
		sta addr
		lda int_hi
		sta addr+1	

		cpx #0
		beq @one

		jsr getint
		lda int_lo
		sta max 

@one:	ldy #0
@loop:	lda (addr),y
		tax
		tya
		pha
		txa
		jsr putint
		tax
		pla
		tay
		txa

		iny
		tya
		and #%1111
		bne @nocr

		lda #$0a
		sta tty_out
		jmp @cont

@nocr:	lda #$20
		sta tty_out
@cont:	cpy max
		bne @loop

		lda #$0a
		sta tty_out
		rts

doload: jsr getint
		lda int_lo
		sta $e5
		lda int_hi
		sta $e6

		tya
		clc
		adc #$04
		sta $e3
		lda #0
		sta $e4
		jsr loadf
		rts

helpmsg:
		.ASCIIZ "How to use the monitor:\r\n  * r <addr> [n]: Print [n] bytes from <addr>\r\n  * w <addr> [dat]: Write [dat] to <addr>\r\n  * e <addr>: Start execution at <addr>\r\n  * l <addr> [path]: Loads file [path] from storage device\r\n"


; Read a 16-bit integer
getint: lda #$00
		sta int_lo
		sta int_hi
		lda keyboard_buffer,y
		cmp #' '
		bne @loop
		jsr getch
		jmp getint
@loop:  lda #$00
		ldx #0
		jsr getch
		cmp #0
		beq @done
		cmp #' '
		beq @done2

		cmp #'0'
		bmi @inval
		beq @zero
			
		cmp #'a'
		bmi @dig
		beq @ten
		jmp @do

@zero:	lda #$00
		jmp @app

@ten:   lda #$0a
		jmp @app

@do:	clc
		sbc #'a'
		adc #$0a
		jmp @app

@dig:	sec
		sbc #'0'

@app:   asl int_lo ; Shift everything left by 4 bits
		rol int_hi
		asl int_lo
		rol int_hi
		asl int_lo
		rol int_hi
		asl int_lo
		rol int_hi
		clc
		ora int_lo
		sta int_lo
		jmp @loop
@done2: ldx #1
@done:	rts

@inval: jmp mon



putint: ldx #1
		ldy #0
		sty putint_buf+3
		sty putint_buf
		sty putint_buf+1

		cmp #0
		beq @zero

@loop:	lsr a
		ror putint_buf+3
		lsr a
		ror putint_buf+3
		lsr a
		ror putint_buf+3
		lsr a
		ror putint_buf+3

		clc
		ror putint_buf+3
		ror putint_buf+3
		ror putint_buf+3
		ror putint_buf+3
		clc
	
		tay
		lda putint_buf+3
		cmp #10
		bmi @dig

		sbc #10
		clc
		adc #'A'
		jmp @out

@dig:	adc #'0'
@out:	sta putint_buf,x
		lda #0
		sta putint_buf+3
		dex
		tya
		cpx #$ff
		bne @loop 	
@done:  lda #<putint_buf
		sta $02
		lda #>putint_buf
		sta $03
		jsr puts
		rts

@zero:  lda #'0'
		sta tty_out
		sta tty_out
		rts

getch:  lda keyboard_buffer,y
		iny
		rts



clear:  ldy #$00
		lda #$00
@loop:	sta keyboard_buffer,y
		iny
		cpy #keyboard_size
		bne @loop
		rts

