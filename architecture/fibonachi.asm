[section code, code]

start:
  ldsp 0xfff0
  ldbp 0xfff0

  li  r0, 10

  call fib
  outd r0
  hlt


fib:
  ; if n == 0 return n
  li  r1, 0
  cmp r0, r1
  je  fib_ret

  ; if n == 1 return n
  li  r1, 1
  cmp r0, r1
  je  fib_ret

  ; save n
  push r0

  ; fib(n-1)
  li  r1, 1
  sub r0, r1
  call fib
  mov  r2, r0

  ; restore n
  pop  r0

  ; preserve fib(n-1)
  push r2

  ; fib(n-2)
  li  r1, 2
  sub r0, r1
  call fib

  pop  r2
  add  r0, r2

fib_ret:
  outd r0
  ret
