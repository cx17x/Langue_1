[section code, code]
start:
  ldsp 0xfff0
  ldbp 0xfff0
  call calc_txt_main
  hlt
.text
; function calc_txt_main
calc_txt_main:
; CALL send_byte
; Nop(Literal) [const:'1']
  li r0, 49
  call calc_txt_send_byte
; CALL send_byte
; Nop(Literal) [const:'\n']
  li r0, 10
  call calc_txt_send_byte
calc_txt_main_end:
  ret

; function calc_txt_send_byte
calc_txt_send_byte:
; empty
calc_txt_send_byte_end:
  outb r0
  ret

