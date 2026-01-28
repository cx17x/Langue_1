[section code, code]
main:
    ldsp -4    ; init sp
    ldbp -4    ; init bp
    li r7, 0    ; frame init
    push r7
    push r7
    push r7
bb_stmt_651e80cfd0d0:
    li r0, 0
    getbp r6    ; bp
    li r7, 12    ; off x
    sub r6, r7
    store r6, r0    ; store x
    jmp bb_while_651e80cfd2b0
bb_while_651e80cfd2b0:
    li r0, 1
    jnz r0, bb_stmt_651e80d03730
    jmp bb_after_while_651e80d03610
bb_after_while_651e80d03610:
    jmp bb_exit_651e80d0b720
bb_exit_651e80d0b720:
    li r0, 0
    ret
    hlt
bb_stmt_651e80d03730:
    call readChar
    getbp r6    ; bp
    li r7, 8    ; off op
    sub r6, r7
    store r6, r0    ; store op
    jmp bb_stmt_651e80d08c40
bb_stmt_651e80d08c40:
    call readInt
    getbp r6    ; bp
    li r7, 4    ; off num
    sub r6, r7
    store r6, r0    ; store num
    jmp bb_if_651e80d08e40
bb_if_651e80d08e40:
    getbp r6    ; bp
    li r7, 8    ; off op
    sub r6, r7
    load r0, r6    ; load op
    li r1, 102
    eq r0, r1
    jnz r0, bb_stmt_651e80d09160
    jmp bb_merge_651e80d09040
bb_stmt_651e80d09160:
    getbp r6    ; bp
    li r7, 4    ; off num
    sub r6, r7
    load r0, r6    ; load num
    call fib
    mov r1, r0
    getbp r6    ; bp
    li r7, 12    ; off x
    sub r6, r7
    store r6, r1    ; store x
    jmp bb_merge_651e80d09040
bb_merge_651e80d09040:
    jmp bb_if_651e80d094a0
bb_if_651e80d094a0:
    getbp r6    ; bp
    li r7, 8    ; off op
    sub r6, r7
    load r1, r6    ; load op
    li r2, 43
    eq r1, r2
    jnz r1, bb_stmt_651e80d097c0
    jmp bb_merge_651e80d096a0
bb_stmt_651e80d097c0:
    getbp r6    ; bp
    li r7, 12    ; off x
    sub r6, r7
    load r1, r6    ; load x
    getbp r6    ; bp
    li r7, 4    ; off num
    sub r6, r7
    load r2, r6    ; load num
    add r1, r2
    getbp r6    ; bp
    li r7, 12    ; off x
    sub r6, r7
    store r6, r1    ; store x
    jmp bb_merge_651e80d096a0
bb_merge_651e80d096a0:
    jmp bb_if_651e80d09b20
bb_if_651e80d09b20:
    getbp r6    ; bp
    li r7, 8    ; off op
    sub r6, r7
    load r1, r6    ; load op
    li r2, 45
    eq r1, r2
    jnz r1, bb_stmt_651e80d09e40
    jmp bb_merge_651e80d09d20
bb_stmt_651e80d09e40:
    getbp r6    ; bp
    li r7, 12    ; off x
    sub r6, r7
    load r1, r6    ; load x
    getbp r6    ; bp
    li r7, 4    ; off num
    sub r6, r7
    load r2, r6    ; load num
    sub r1, r2
    getbp r6    ; bp
    li r7, 12    ; off x
    sub r6, r7
    store r6, r1    ; store x
    jmp bb_merge_651e80d09d20
bb_merge_651e80d09d20:
    jmp bb_if_651e80d0a1a0
bb_if_651e80d0a1a0:
    getbp r6    ; bp
    li r7, 8    ; off op
    sub r6, r7
    load r1, r6    ; load op
    li r2, 42
    eq r1, r2
    jnz r1, bb_stmt_651e80d0a4c0
    jmp bb_merge_651e80d0a3a0
bb_stmt_651e80d0a4c0:
    getbp r6    ; bp
    li r7, 12    ; off x
    sub r6, r7
    load r1, r6    ; load x
    getbp r6    ; bp
    li r7, 4    ; off num
    sub r6, r7
    load r2, r6    ; load num
    mul r1, r2
    getbp r6    ; bp
    li r7, 12    ; off x
    sub r6, r7
    store r6, r1    ; store x
    jmp bb_merge_651e80d0a3a0
bb_merge_651e80d0a3a0:
    jmp bb_if_651e80d0a820
bb_if_651e80d0a820:
    getbp r6    ; bp
    li r7, 8    ; off op
    sub r6, r7
    load r1, r6    ; load op
    li r2, 47
    eq r1, r2
    jnz r1, bb_if_651e80d0b060
    jmp bb_merge_651e80d0aa20
bb_if_651e80d0b060:
    getbp r6    ; bp
    li r7, 4    ; off num
    sub r6, r7
    load r1, r6    ; load num
    li r2, 0
    eq r1, r2
    jnz r1, bb_stmt_651e80d0b2a0
    jmp bb_stmt_651e80d0b3c0
bb_stmt_651e80d0b3c0:
    getbp r6    ; bp
    li r7, 12    ; off x
    sub r6, r7
    load r1, r6    ; load x
    getbp r6    ; bp
    li r7, 4    ; off num
    sub r6, r7
    load r2, r6    ; load num
    div r1, r2
    getbp r6    ; bp
    li r7, 12    ; off x
    sub r6, r7
    store r6, r1    ; store x
    jmp bb_merge_651e80d0b180
bb_stmt_651e80d0b2a0:
    li r1, 0
    getbp r6    ; bp
    li r7, 12    ; off x
    sub r6, r7
    store r6, r1    ; store x
    jmp bb_merge_651e80d0b180
bb_merge_651e80d0b180:
    jmp bb_merge_651e80d0aa20
bb_merge_651e80d0aa20:
    jmp bb_call_writeInt_651e80d0b4e0
bb_call_writeInt_651e80d0b4e0:
    getbp r6    ; bp
    li r7, 12    ; off x
    sub r6, r7
    load r1, r6    ; load x
    mov r0, r1    ; arg
    call writeInt
    jmp bb_call_writeChar_651e80d0b600
bb_call_writeChar_651e80d0b600:
    li r1, 10
    mov r0, r1    ; arg
    call writeChar
    jmp bb_while_651e80cfd2b0
fib:
    li r7, 0    ; frame init
    push r7
    push r7
    getbp r6    ; bp
    li r7, 4    ; off n
    sub r6, r7
    store r6, r0    ; arg n
bb_if_651e80d02fe0:
    getbp r6    ; bp
    li r7, 4    ; off n
    sub r6, r7
    load r0, r6    ; load n
    li r1, 2
    lt r0, r1
    jnz r0, bb_stmt_651e80d03ea0
    jmp bb_stmt_651e80d03fc0
bb_stmt_651e80d03fc0:
    getbp r6    ; bp
    li r7, 4    ; off n
    sub r6, r7
    load r0, r6    ; load n
    li r1, 1
    sub r0, r1
    call fib
    mov r1, r0
    push r1    ; spill lhs
    getbp r6    ; bp
    li r7, 4    ; off n
    sub r6, r7
    load r2, r6    ; load n
    li r3, 2
    sub r2, r3
    mov r0, r2
    call fib
    mov r3, r0
    pop r1    ; restore lhs
    add r1, r3
    getbp r6    ; bp
    li r7, 8    ; off r
    sub r6, r7
    store r6, r1    ; store r
    jmp bb_merge_651e80d03220
bb_stmt_651e80d03ea0:
    li r1, 1
    getbp r6    ; bp
    li r7, 8    ; off r
    sub r6, r7
    store r6, r1    ; store r
    jmp bb_merge_651e80d03220
bb_merge_651e80d03220:
    jmp bb_exit_651e80d04200
bb_exit_651e80d04200:
    getbp r6    ; bp
    li r7, 8    ; off r
    sub r6, r7
    load r1, r6    ; load r
    mov r0, r1    ; return
    ret
readChar:
__bi_readChar_loop:
    in r0
    mov r1, r0
    li r2, 32
    eq r1, r2
    jnz r1, __bi_readChar_loop
    mov r1, r0
    li r2, 9
    eq r1, r2
    jnz r1, __bi_readChar_loop
    mov r1, r0
    li r2, 10
    eq r1, r2
    jnz r1, __bi_readChar_loop
    mov r1, r0
    li r2, 13
    eq r1, r2
    jnz r1, __bi_readChar_loop
    ret
writeChar:
    out r0
    ret
readInt:
    li r1, 0    ; acc=0
    li r2, 1    ; sign=1
__bi_readInt_skip:
    in r3    ; c=in
    mov r4, r3
    li r5, 32
    eq r4, r5
    jnz r4, __bi_readInt_skip
    mov r4, r3
    li r5, 9
    eq r4, r5
    jnz r4, __bi_readInt_skip
    mov r4, r3
    li r5, 10
    eq r4, r5
    jnz r4, __bi_readInt_skip
    mov r4, r3
    li r5, 13
    eq r4, r5
    jnz r4, __bi_readInt_skip
    mov r4, r3
    li r5, 45
    eq r4, r5
    jnz r4, __bi_readInt_neg
    jmp __bi_readInt_afterws
__bi_readInt_neg:
    li r2, -1    ; sign=-1
    in r3    ; c=in after '-'
__bi_readInt_afterws:
__bi_readInt_digit:
    mov r4, r3
    li r5, 48
    ge r4, r5
    mov r6, r4
    li r7, 1
    eq r6, r7
    jnz r6, __bi_readInt_chk_hi
    jmp __bi_readInt_done
__bi_readInt_chk_hi:
    mov r4, r3
    li r5, 57
    le r4, r5
    mov r6, r4
    li r7, 1
    eq r6, r7
    jnz r6, __bi_readInt_do_digit
    jmp __bi_readInt_done
__bi_readInt_do_digit:
    mov r4, r3
    li r5, 48
    sub r4, r5    ; digit=c-'0'
    li r5, 10
    mul r1, r5    ; acc*=10
    add r1, r4    ; acc+=digit
    in r3    ; c=in next
    jmp __bi_readInt_digit
__bi_readInt_done:
    mov r4, r2
    li r5, -1
    eq r4, r5
    jnz r4, __bi_readInt_applyNeg
    mov r0, r1    ; ret=acc
    ret
__bi_readInt_applyNeg:
    li r5, 0
    mov r0, r5
    sub r0, r1    ; 0-acc
    ret
__writeUInt:
    mov r1, r0
    li r2, 10
    lt r1, r2
    jnz r1, __bi_writeUInt_base
    jmp __bi_writeUInt_recur
__bi_writeUInt_base:
    mov r1, r0
    li r2, 48
    add r1, r2
    out r1
    ret
__bi_writeUInt_recur:
    push r0    ; push n
    mov r1, r0
    li r2, 10
    div r1, r2    ; q=n/10
    mov r0, r1
    call __writeUInt
    pop r3    ; pop n
    mov r1, r3
    li r2, 10
    mod r1, r2    ; rem=n%10
    li r2, 48
    add r1, r2
    out r1
    ret
writeInt:
    mov r1, r0
    li r2, 0
    eq r1, r2
    jnz r1, __bi_writeInt_zero
    mov r1, r0
    li r2, 0
    lt r1, r2
    jnz r1, __bi_writeInt_neg
    call __writeUInt
    ret
__bi_writeInt_zero:
    li r1, 48
    out r1
    ret
__bi_writeInt_neg:
    li r1, 45
    out r1
    li r1, 0
    mov r3, r1
    sub r3, r0    ; negate
    mov r0, r3
    call __writeUInt
    ret

[section data, dataMem]
var_main_num:
    dd 0    ; num
var_main_op:
    dd 0    ; op
var_main_x:
    dd 0    ; x
var_fib_r:
    dd 0    ; r
var_fib_n:
    dd 0    ; n
