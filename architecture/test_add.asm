[section code, code]
start:
    li r0, 7
    li r1, 7
    cmp r0, r1
    jz equal

    li r0, 0
    outd r0
    hlt

equal:
    li r0, 1
    outd r0
    hlt
