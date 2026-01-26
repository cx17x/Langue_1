[section code, code]
start:
  ldsp 0xfff0
  ldbp 0xfff0

  call calc_txt_main
  hlt

; function calc_txt_main
calc_txt_main:
  li r0, 49        ; '1'
  call calc_txt_send_byte
  li r0, 10        ; '\n'
  call calc_txt_send_byte
  ret

; function calc_txt_send_byte
calc_txt_send_byte:
  outb r0
  ret
