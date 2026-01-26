[section code, code]
start:
  ldsp 0xfff0
  ldbp 0xfff0
  call calc_txt_main
  hlt
; function calc_txt_main
calc_txt_main:
  li r0, 0
; store -> value
; Expr(:=)
; store -> read_loop
; Nop(Identifier) [var:]
calc_txt_main_B2:
; ERROR
calc_txt_main_B3:
; op
  li r1, 10
  cmp r2, r1
  jnz calc_txt_main_B5
calc_txt_main_B4:
; CALL print_int
; Nop(Identifier) [var:value]
; Nop(Identifier) [var:value]
  call calc_txt_print_int
; Nop(Identifier) [var:return]
calc_txt_main_B6:
; join
calc_txt_main_B7:
; Nop(Identifier) [var:]
; BinaryOp(CompareExpr) { Nop(Identifier) [var:digit] | BinaryOp(AddExpr) { BinaryOp(AddExpr) { BinaryOp(AddExpr) { Nop(Identifier) [var:ch] | Nop(Literal) [const:'0'] } | Expr(digit := ch) } | Nop(Literal) [const:'0'] } }
; Nop(Identifier) [var:]
calc_txt_main_B8:
; BinaryOp(CompareExpr) { Nop(Identifier) [var:value] | BinaryOp(AddExpr) { BinaryOp(AddExpr) { BinaryOp(MulExpr) { Nop(Identifier) [var:value] | Nop(Literal) [const:10] } | BinaryOp(MulExpr) { BinaryOp(MulExpr) { Nop(Identifier) [var:digit] | Expr(value := value) } | Nop(Literal) [const:10] } } | Nop(Identifier) [var:digit] } }
; Nop(Identifier) [var:goto]
calc_txt_main_B5:
; empty
  JMP calc_txt_main_B6
calc_txt_main_end:
  ret

; function calc_txt_print_int
calc_txt_print_int:
; Nop(Identifier) [var:n]
; store -> value
calc_txt_print_int_B2:
; op
  CMP R0, 0
  JZ calc_txt_print_int_B4
calc_txt_print_int_B3:
; CALL send_byte
; Nop(Literal) [const:'-']
  li r0, 45
  call calc_txt_send_byte
  li r0, 0
  add r0, r0
; store -> value
calc_txt_print_int_B5:
; join
calc_txt_print_int_B6:
; op
  li r1, 0
  cmp r0, r1
  jnz calc_txt_print_int_B8
calc_txt_print_int_B7:
; CALL send_byte
; Nop(Identifier) [var:value]
; Nop(Identifier) [var:value]
  call calc_txt_send_byte
calc_txt_print_int_B9:
; join
calc_txt_print_int_B10:
; CALL send_byte
; Nop(Literal) [const:'\n']
  li r0, 10
  call calc_txt_send_byte
calc_txt_print_int_B8:
; empty
  JMP calc_txt_print_int_B9
calc_txt_print_int_B4:
; empty
  JMP calc_txt_print_int_B5
calc_txt_print_int_end:
  ret

; function calc_txt_print_uint
calc_txt_print_uint:
  li r5, 10
  mov r0, r1
  mul r0, r5
; store -> digit
; CALL send_byte
; BinaryOp(AddExpr) { Nop(Literal) [const:'0']
  li r0, 48
; Nop(Identifier) [var:digit]
; Nop(Identifier) [var:digit]
  call calc_txt_send_byte
calc_txt_print_uint_B1:
; op
  CMP R0, 0
  JZ calc_txt_print_uint_B3
calc_txt_print_uint_B2:
; CALL print_uint
; BinaryOp(MulExpr) { Nop(Identifier) [var:n]
; BinaryOp(MulExpr) { Nop(Identifier) [var:n]
; Nop(Literal) [const:10]
  li r1, 10
  call calc_txt_print_uint
calc_txt_print_uint_B4:
; join
  JMP calc_txt_print_uint
calc_txt_print_uint_B3:
; empty
  JMP calc_txt_print_uint_B4
calc_txt_print_uint_end:
  ret

