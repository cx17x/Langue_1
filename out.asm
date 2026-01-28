[section code, code]
start:
  ldsp 0xfff0
  ldbp 0xfff0
  call fib_recur_txt_main
  hlt
; function fib_recur_txt_main
fib_recur_txt_main:
; CALL print_int
; Call(fib) { ...
  outd r0
fib_recur_txt_main_end:
  ret

; function fib_recur_txt_fib
fib_recur_txt_fib_B1:
; op
  li r5, 0
  cmp r0, r5
  jz fib_recur_txt_fib_B3
fib_recur_txt_fib_B2:
  li r0, 1
; store -> r
fib_recur_txt_fib_B4:
; join
fib_recur_txt_fib_B5:
; return
; Nop(Identifier) [var:r]
  ret
  jmp fib_recur_txt_fib_end
fib_recur_txt_fib_B3:
; CALL fib
; Call(fib) { BinaryOp(AddExpr) { ...
; Call(fib) { BinaryOp(AddExpr) { ...
; ...
; ...
  call fib_recur_txt_fib
  jmp fib_recur_txt_fib_B4
fib_recur_txt_fib_end:
  ret

