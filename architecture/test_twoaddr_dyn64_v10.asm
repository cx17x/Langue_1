[section _code, codeMem]
start:
  ldsp 0xfff0
  ldbp 0xfff0

  ; r0 = 2, r1 = 3, r0 = r0 + r1
  ldi r0, 0x0200000000000002
  ldi r1, 0x0200000000000003
  add r0, r1

  ; output low byte of r0 (5)
  outb r0
  hlt

[section _data, dataMem]
; reserve a small data area (example)
data_start:
  resq 4
