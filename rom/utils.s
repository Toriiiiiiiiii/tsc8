;-- putc ----------------------------
;  START = $f000
;  SIZE  = 4B
;  END   = $f003
;
;  Outputs an ASCII character to serial
putc:	sta $8000
		rts


;-- getc ----------------------------
;  START = $f004
;  SIZE  = 4B
;  END   = $f007
;
;  Reads an ASCII character from serial
getc:	lda $8001
		rts


;-- stoi ----------------------------
;  START = $f008
;  SIZE  = 78B
;  END   = $f04d
;
;  Converts a 2-byte string to an integer.
;  Expects a valid hex string using 0..9 and A..F
;  Address passed through the stack.
;  Eg, to convert a string at $1234:
;    1. Push high byte ($34)
;    2. Push low byte ($12)
;    3. Call subroutine

STOI_ADDR = $e0
STOI_VAL  = $e2

stoi:	
		; Preserve call address
		pla
		tax
		pla
		tay

		pla
		sta STOI_ADDR
		pla
		sta STOI_ADDR+1

		; Restore call address
		tya
		pha
		txa
		pha

		ldy #$00
		ldx #$02
@loop:	lda (STOI_ADDR),y
		
		cmp #'A'
		bpl @ualph
		cmp #'a'
		bpl @lalph
		cmp #'0'
		bpl @num
		rts

@ualph:	sbc #'A'
		jmp @app
@lalph: sbc #'a'
		jmp @app
@num:   sbc #'0'
		jmp @app

@app:	asl STOI_VAL
		asl STOI_VAL
		asl STOI_VAL
		asl STOI_VAL

		ora STOI_VAL
		sta STOI_VAL

		iny
		dex
		beq @done
		jmp @loop

@done:	lda STOI_VAL
		rts


LOADF_PATH = $e3
LOADF_ADDR = $e5

SDCTRL = $9000

;-- loadf ---------------------
;  START = $f04e
;  SIZE  = 
;  END   = 
loadf:	ldx #$00
		ldy #$00
@wpath: lda (LOADF_PATH),y
		beq @wdone
		sta SDCTRL
		iny
		jmp @wpath
@wdone: lda #$ff
		sta SDCTRL
		lda SDCTRL
		cmp #$ff
		bne @err

		; File is ready to read!
@rloop:	lda SDCTRL
		beq @done

		ldy #0
		lda SDCTRL+1
		sta (LOADF_ADDR),y

		inc LOADF_ADDR
		bcs @rloop
		inc LOADF_ADDR+1
		jmp @rloop

@done:	lda #'O'
		jsr putc
		lda #'K'
		jsr putc
		rts

@err:	lda #'E'
		jsr putc
		rts
