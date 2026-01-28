[section code, code]
start:
  ldsp 0xfff0
  ldbp 0xfff0
  call fib_recur_txt_main
  hlt
; function fib_recur_txt_main
fib_recur_txt_main:
; Nop(Identifier) [var:var]
  li r0, 1
  li r5, 2
  add r0, r5
; store -> x
; CALL print_int
  outd r0
fib_recur_txt_main_end:
  ret

