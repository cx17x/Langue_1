[section code, code]
start:
    li r0, 5
loop:
    outd r0
    li r1, 1
    sub r0, r1
    li r2, 0
    cmp r0, r2
    jz done
    jn done
    jmp loop
done:
    hlt
