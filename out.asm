[section code, code]
start:
  ldsp 0xfff0
  ldbp 0xfff0
  call calc_txt_main
  hlt
; function calc_txt_main
calc_txt_main:
  li r0, 0
; store -> a
  li r1, 1
; store -> b
  li r2, 0
; store -> i
calc_txt_main_B2:
  li r3, 10
; store -> n
calc_txt_main_B3:
; ERROR
calc_txt_main_B4:
; op
  li r5, 0
  cmp r0, r5
  jz calc_txt_main_B7
calc_txt_main_B5:
; CALL print_int
; Nop(Identifier) [var:a]
; Nop(Identifier) [var:a]
  call calc_txt_print_int
  add r0, r1
; store -> next
; Nop(Identifier) [var:b]
; store -> a
calc_txt_main_B6:
; Nop(Identifier) [var:next]
; store -> b
  li r5, 1
  add r2, r5
; store -> i
; Nop(Identifier) [var:goto]
calc_txt_main_B8:
; join
calc_txt_main_B7:
; empty
  jmp calc_txt_main_B8
calc_txt_main_end:
  ret

