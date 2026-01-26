[section code, code]

start:
  ldsp 0xfff0
  ldbp 0xfff0

  ; пример: считаем число и печатаем его outd
  call read_pos_int
  outd r0
  hlt


; ----------------------------------------
; read_pos_int()
; Reads decimal digits until '\n'
; Returns: r0 = value (TAG_INT)
; Clobbers: r1..r5
; ----------------------------------------
read_pos_int:
  ; value = 0
  li r0, 0
  li r5, 10          ; const '\n' = 10 (будем переиспользовать)
  li r4, 10          ; const 10 для умножения

; ждать первую цифру (пропускаем пустые строки)
rpi_wait_first:
  inb r1             ; r1 = ch
  cmp r1, r5         ; ch == '\n' ?
  jz  rpi_wait_first ; пустая строка -> ждём дальше

; основной цикл: пока не '\n'
rpi_loop:
  ; digit = ch - '0'
  li  r2, 48         ; '0'
  sub r1, r2         ; r1 = digit (0..9) как TAG_INT

  ; value = value*10 + digit
  mov r3, r0
  mul r3, r4         ; r3 = value*10
  add r3, r1         ; r3 = value*10 + digit
  mov r0, r3         ; value = r3

  ; читать следующий символ
  inb r1
  cmp r1, r5         ; '\n' ?
  jnz rpi_loop

  ret
