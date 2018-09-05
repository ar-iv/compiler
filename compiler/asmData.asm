		.data 
; SUPPORTING VARIABLES 
		@buffer   db		6 
		blength   db      (?) 
		@buf      db      256 DUP (?) 
		clrf      db      0Dh, 0Ah, "$" 
		output    db      6 DUP (?), "$" 
		err_msg   db      "Input error, try again", 0Dh, 0Ah, "$" 
		@true     db      "true" 
		@@true    db      "true$" 
		@false    db      "false" 
		@@false   db      "false$" 
; USING VARIABLES 
x dw (?) 
y dw (?) 
a dw (?) 
b dw (?) 
c dw (?) 
mas dw 10 DUP (?) 
i dw (?) 
s db 2 DUP (?) 
@1p db "Enter char: $" 
@4p db "Enter x: $" 
@10p db "Enter y: $" 
@26p db "b = $" 
@35p db "y = $" 
@40p db "a = $" 
@45p db "x = $" 
@51p db "x= $" 
