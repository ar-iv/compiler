     .model small 
     include asmData.asm 
     .stack 100h 
     .code 
     .386 
start: 
     mov   ax, @data 
     mov   ds, ax 
; LET() 
     mov   di, 0 
     push  di 
     mov   ax, 0 
     push  ax 
     pop   ax 
     pop   di 
     mov   i[di], ax 
; REPEAT() 
@0:; WRITE() 
; WRITE LITERAL 
     lea   dx, @1p 
     mov   ah, 9 
     int   21h 
; READLN() 
     push  0 
     mov   di, 0 
     mov   ax, i[di] 
     push  ax 
; READ CHAR 
     mov   ah, 0Ah 
     lea   dx, @buffer 
     int   21h 
     xor   dx, dx 
     mov   dl, @buf[0] 
     pop   di 
     mov   s[di], dl 
     lea   dx, clrf 
     mov   ah, 9 
     int   21h 
; LET() 
     mov   di, 0 
     push  di 
     mov   di, 0 
     mov   ax, i[di] 
     push  ax 
     mov   ax, 1 
     push  ax 
     pop   bx 
     pop   ax 
     add   ax, bx 
     push  ax 
     pop   ax 
     pop   di 
     mov   i[di], ax 
     mov   di, 0 
     mov   ax, i[di] 
     push  ax 
     mov   ax, 3 
     push  ax 
     pop   bx 
     pop   ax 
     cmp   ax, bx 
     jb    @2 
     push  0 
     jmp   @3 
@2: 
     push  1 
@3: 
     pop   ax 
     cmp   ax, 0 
     jne   @0 
; WRITE() 
; WRITE LITERAL 
     lea   dx, @4p 
     mov   ah, 9 
     int   21h 
; READLN() 
     push  0 
; READ INTEGER 
@7: 
     mov   ah, 0Ah 
     lea   dx, @buffer 
     int   21h 
     mov   ax, 0 
     mov   cx, 0 
     mov   cl, byte ptr[blength] 
     mov   bx, cx 
@5: 
     dec   bx 
     mov   al, @buf[bx] 
     cmp   al, "9" 
     ja    @6 
     cmp   al, "0" 
     jb    @6 
     loop  @5 
     mov   cl, byte ptr[blength] 
     mov   di, 0 
     mov   ax, 0 
@9: 
     mov   bl, @buf[di] 
     inc   di 
     sub   bl, 30h 
     add   ax, bx 
     mov   si, ax 
     mov   bx, 10 
     mul   bx 
     loop  @9 
     mov   ax, si 
     pop   di 
     shl   di, 1 
     mov   x[di],ax 
     jmp   @8 
@6: 
     lea   dx, err_msg 
     mov   ah, 9 
     int   21h 
     jmp   @7 
@8: 
     lea   dx, clrf 
     mov   ah, 9 
     int   21h 
; WRITE() 
; WRITE LITERAL 
     lea   dx, @10p 
     mov   ah, 9 
     int   21h 
; READLN() 
     push  0 
; READ INTEGER 
@13: 
     mov   ah, 0Ah 
     lea   dx, @buffer 
     int   21h 
     mov   ax, 0 
     mov   cx, 0 
     mov   cl, byte ptr[blength] 
     mov   bx, cx 
@11: 
     dec   bx 
     mov   al, @buf[bx] 
     cmp   al, "9" 
     ja    @12 
     cmp   al, "0" 
     jb    @12 
     loop  @11 
     mov   cl, byte ptr[blength] 
     mov   di, 0 
     mov   ax, 0 
@15: 
     mov   bl, @buf[di] 
     inc   di 
     sub   bl, 30h 
     add   ax, bx 
     mov   si, ax 
     mov   bx, 10 
     mul   bx 
     loop  @15 
     mov   ax, si 
     pop   di 
     shl   di, 1 
     mov   y[di],ax 
     jmp   @14 
@12: 
     lea   dx, err_msg 
     mov   ah, 9 
     int   21h 
     jmp   @13 
@14: 
     lea   dx, clrf 
     mov   ah, 9 
     int   21h 
; LET() 
     mov   di, 0 
     mov   di, 0 
     mov   ax, x[di] 
     push  ax 
     mov   ax, 2 
     push  ax 
     pop   bx 
     pop   ax 
     sub   ax, bx 
     push  ax 
     pop   di 
     push  di 
     mov   ax, 4 
     push  ax 
     mov   di, 0 
     mov   ax, y[di] 
     push  ax 
     pop   bx 
     pop   ax 
     mul   bx 
     push  ax 
     mov   ax, 3 
     push  ax 
     mov   di, 0 
     mov   ax, y[di] 
     push  ax 
     mov   ax, 2 
     push  ax 
     pop   bx 
     pop   ax 
     add   ax, bx 
     push  ax 
     pop   bx 
     pop   ax 
     mul   bx 
     push  ax 
     pop   bx 
     pop   ax 
     sub   ax, bx 
     push  ax 
     pop   ax 
     pop   di 
     shl   di, 1 
     mov   mas[di], ax 
; LET() 
     mov   di, 0 
     push  di 
     mov   di, 0 
     mov   di, 0 
     mov   ax, x[di] 
     push  ax 
     mov   ax, 2 
     push  ax 
     pop   bx 
     pop   ax 
     sub   ax, bx 
     push  ax 
     pop   di 
     shl   di, 1 
     mov   ax, mas[di] 
     push  ax 
     pop   ax 
     pop   di 
     mov   c[di], ax 
; IF() 
     mov   di, 0 
     mov   ax, x[di] 
     push  ax 
     mov   di, 0 
     mov   ax, y[di] 
     push  ax 
     pop   bx 
     pop   ax 
     cmp   ax, bx 
     jb    @16 
     push  0 
     jmp   @17 
@16: 
     push  1 
@17: 
     mov   di, 0 
     mov   ax, y[di] 
     push  ax 
     mov   ax, 5 
     push  ax 
     pop   bx 
     pop   ax 
     cmp   ax, bx 
     jae   @18 
     push  0 
     jmp   @19 
@18: 
     push  1 
@19: 
     pop   bx 
     pop   ax 
     and   ax, bx 
     push  ax 
     pop   ax 
     cmp   ax, 0 
     jz    @20 
; IF THEN 
; LET() 
     mov   di, 0 
     push  di 
     mov   ax, 2 
     push  ax 
     mov   di, 0 
     mov   ax, x[di] 
     push  ax 
     mov   di, 0 
     mov   ax, y[di] 
     push  ax 
     pop   bx 
     pop   ax 
     add   ax, bx 
     push  ax 
     pop   bx 
     pop   ax 
     mul   bx 
     push  ax 
     pop   ax 
     pop   di 
     mov   a[di], ax 
     jmp   @21 
@20: 
; IF ELSE 
; LET() 
     mov   di, 0 
     push  di 
     mov   ax, 10 
     push  ax 
     mov   di, 0 
     mov   ax, x[di] 
     push  ax 
     pop   bx 
     pop   ax 
     mul   bx 
     push  ax 
     pop   ax 
     pop   di 
     mov   a[di], ax 
@21: 
; IF() 
     mov   di, 0 
     mov   ax, x[di] 
     push  ax 
     mov   ax, 2 
     push  ax 
     pop   bx 
     pop   ax 
     cmp   ax, bx 
     jne   @22 
     push  0 
     jmp   @23 
@22: 
     push  1 
@23: 
     push  1 
     pop   bx 
     pop   ax 
     or    ax, bx 
     push  ax 
     pop   ax 
     cmp   ax, 0 
     jz    @24 
; IF THEN 
; LET() 
     mov   di, 0 
     push  di 
     mov   di, 0 
     mov   ax, x[di] 
     push  ax 
     mov   di, 0 
     mov   ax, c[di] 
     push  ax 
     pop   bx 
     pop   ax 
     sub   ax, bx 
     push  ax 
     pop   ax 
     pop   di 
     mov   b[di], ax 
; WRITELN() 
; WRITE LITERAL 
     lea   dx, @26p 
     mov   ah, 9 
     int   21h 
     mov   di, 0 
     mov   ax, b[di] 
     push  ax 
; WRITE ARIPHMETICS 
     pop   ax 
     mov   bx, 10 
     mov   di, 0 
     mov   si, ax 
     cmp   ax, 0 
     jns   @29 
     neg   si 
     mov   ah, 2 
     mov   dl, "-" 
     int   21h 
     mov   ax, si 
@29: 
     mov   dx, 0 
     div   bx 
     add   dl, 30h 
     mov   output[di], dl 
     inc   di 
     cmp   al, 0 
     jnz   @29 
     mov   cx, di 
     dec   di 
     mov   ah, 2 
@30: 
     mov   dl, output[di] 
     dec   di 
     int   21h 
     loop  @30 
     lea   dx, clrf 
     mov   ah, 9 
     int   21h 
     jmp   @25 
@24: 
@25: 
; LET() 
     mov   di, 0 
     push  di 
     mov   ax, 0 
     push  ax 
     pop   ax 
     pop   di 
     mov   x[di], ax 
; WHILE() 
@31: 
     mov   di, 0 
     mov   ax, x[di] 
     push  ax 
     mov   ax, 45 
     push  ax 
     pop   bx 
     pop   ax 
     cmp   ax, bx 
     jb    @32 
     push  0 
     jmp   @33 
@32: 
     push  1 
@33: 
     pop   ax 
     cmp   ax, 0 
     jz    @34 
; LET() 
     mov   di, 0 
     push  di 
     mov   di, 0 
     mov   ax, x[di] 
     push  ax 
     mov   ax, 5 
     push  ax 
     pop   bx 
     pop   ax 
     add   ax, bx 
     push  ax 
     pop   ax 
     pop   di 
     mov   x[di], ax 
; LET() 
     mov   di, 0 
     push  di 
     mov   di, 0 
     mov   ax, y[di] 
     push  ax 
     mov   ax, 2 
     push  ax 
     pop   bx 
     pop   ax 
     add   ax, bx 
     push  ax 
     mov   ax, 2 
     push  ax 
     pop   bx 
     pop   ax 
     mul   bx 
     push  ax 
     pop   ax 
     pop   di 
     mov   y[di], ax 
     jmp   @31 
@34: 
; WRITELN() 
; WRITE LITERAL 
     lea   dx, @35p 
     mov   ah, 9 
     int   21h 
     mov   di, 0 
     mov   ax, y[di] 
     push  ax 
; WRITE ARIPHMETICS 
     pop   ax 
     mov   bx, 10 
     mov   di, 0 
     mov   si, ax 
     cmp   ax, 0 
     jns   @38 
     neg   si 
     mov   ah, 2 
     mov   dl, "-" 
     int   21h 
     mov   ax, si 
@38: 
     mov   dx, 0 
     div   bx 
     add   dl, 30h 
     mov   output[di], dl 
     inc   di 
     cmp   al, 0 
     jnz   @38 
     mov   cx, di 
     dec   di 
     mov   ah, 2 
@39: 
     mov   dl, output[di] 
     dec   di 
     int   21h 
     loop  @39 
     lea   dx, clrf 
     mov   ah, 9 
     int   21h 
; WRITELN() 
; WRITE LITERAL 
     lea   dx, @40p 
     mov   ah, 9 
     int   21h 
     mov   di, 0 
     mov   ax, a[di] 
     push  ax 
; WRITE ARIPHMETICS 
     pop   ax 
     mov   bx, 10 
     mov   di, 0 
     mov   si, ax 
     cmp   ax, 0 
     jns   @43 
     neg   si 
     mov   ah, 2 
     mov   dl, "-" 
     int   21h 
     mov   ax, si 
@43: 
     mov   dx, 0 
     div   bx 
     add   dl, 30h 
     mov   output[di], dl 
     inc   di 
     cmp   al, 0 
     jnz   @43 
     mov   cx, di 
     dec   di 
     mov   ah, 2 
@44: 
     mov   dl, output[di] 
     dec   di 
     int   21h 
     loop  @44 
     lea   dx, clrf 
     mov   ah, 9 
     int   21h 
; WRITELN() 
; WRITE LITERAL 
     lea   dx, @45p 
     mov   ah, 9 
     int   21h 
     mov   di, 0 
     mov   ax, x[di] 
     push  ax 
; WRITE ARIPHMETICS 
     pop   ax 
     mov   bx, 10 
     mov   di, 0 
     mov   si, ax 
     cmp   ax, 0 
     jns   @48 
     neg   si 
     mov   ah, 2 
     mov   dl, "-" 
     int   21h 
     mov   ax, si 
@48: 
     mov   dx, 0 
     div   bx 
     add   dl, 30h 
     mov   output[di], dl 
     inc   di 
     cmp   al, 0 
     jnz   @48 
     mov   cx, di 
     dec   di 
     mov   ah, 2 
@49: 
     mov   dl, output[di] 
     dec   di 
     int   21h 
     loop  @49 
     lea   dx, clrf 
     mov   ah, 9 
     int   21h 
; REPEAT() 
@50:; LET() 
     mov   di, 0 
     push  di 
     mov   di, 0 
     mov   ax, x[di] 
     push  ax 
     mov   di, 0 
     mov   ax, x[di] 
     push  ax 
     mov   ax, 4 
     push  ax 
     pop   bx 
     pop   ax 
     xor   dx, dx 
     div   bx 
     push  ax 
     pop   bx 
     pop   ax 
     sub   ax, bx 
     push  ax 
     pop   ax 
     pop   di 
     mov   x[di], ax 
; WRITELN() 
; WRITE LITERAL 
     lea   dx, @51p 
     mov   ah, 9 
     int   21h 
     mov   di, 0 
     mov   ax, x[di] 
     push  ax 
; WRITE ARIPHMETICS 
     pop   ax 
     mov   bx, 10 
     mov   di, 0 
     mov   si, ax 
     cmp   ax, 0 
     jns   @54 
     neg   si 
     mov   ah, 2 
     mov   dl, "-" 
     int   21h 
     mov   ax, si 
@54: 
     mov   dx, 0 
     div   bx 
     add   dl, 30h 
     mov   output[di], dl 
     inc   di 
     cmp   al, 0 
     jnz   @54 
     mov   cx, di 
     dec   di 
     mov   ah, 2 
@55: 
     mov   dl, output[di] 
     dec   di 
     int   21h 
     loop  @55 
     lea   dx, clrf 
     mov   ah, 9 
     int   21h 
