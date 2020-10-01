.text

.global count_det

// Problem:
// compute det of 3x3 matrix,
// passed to function as int32_t* r0
count_det:
    // mat     idx
    // a b c | 0 1 2
    // d e f | 3 4 5
    // g h i | 6 7 8
    //
    // det = a(ei − fh) − b(di − fg) + c(dh − eg)
    
    // usage of registers:
    // r0 -- input array start
    // r1 -- total sum of det
    // r2 -- ptr for element
    // r3 -- tmp

    // det = 0
    mov r1, #0

    // det += aei
    mov r2, #4
    add r2, r0, r2, lsl #2
    ldr r3, [r2]
    //
    mov r2, #8
    add r2, r0, r2, lsl #2
    ldr r2, [r2]
    mul r3, r3, r2
    //
    mov r2, #0
    add r2, r0, r2, lsl #2
    ldr r2, [r2]
    mul r3, r3, r2
    add r1, r1, r3
    
    // det -= afh
    mov r2, #5
    add r2, r0, r2, lsl #2
    ldr r3, [r2]
    //
    mov r2, #7
    add r2, r0, r2, lsl #2
    ldr r2, [r2]
    mul r3, r3, r2
    //
    mov r2, #0
    add r2, r0, r2, lsl #2
    ldr r2, [r2]
    mul r3, r3, r2
    sub r1, r1, r3

    // det -= bdi
    mov r2, #3
    add r2, r0, r2, lsl #2
    ldr r2, [r2]
    mov r3, r2
    //
    mov r2, #8
    add r2, r0, r2, lsl #2
    ldr r2, [r2]
    mul r3, r3, r2
    //
    mov r2, #1
    add r2, r0, r2, lsl #2
    ldr r2, [r2]
    mul r3, r3, r2
    sub r1, r1, r3

    // det += bfg
    mov r2, #5
    add r2, r0, r2, lsl #2
    ldr r2, [r2]
    mov r3, r2
    //
    mov r2, #6
    add r2, r0, r2, lsl #2
    ldr r2, [r2]
    mul r3, r3, r2
    //
    mov r2, #1
    add r2, r0, r2, lsl #2
    ldr r2, [r2]
    mul r3, r3, r2
    add r1, r1, r3

    // det += cdh
    mov r2, #3
    add r2, r0, r2, lsl #2
    ldr r2, [r2]
    mov r3, r2
    //
    mov r2, #7
    add r2, r0, r2, lsl #2
    ldr r2, [r2]
    mul r3, r3, r2
    //
    mov r2, #2
    add r2, r0, r2, lsl #2
    ldr r2, [r2]
    mul r3, r3, r2
    add r1, r1, r3

    // det -= ceg
    mov r2, #4
    add r2, r0, r2, lsl #2
    ldr r2, [r2]
    mov r3, r2
    //
    mov r2, #6
    add r2, r0, r2, lsl #2
    ldr r2, [r2]
    mul r3, r3, r2
    //
    mov r2, #2
    add r2, r0, r2, lsl #2
    ldr r2, [r2]
    mul r3, r3, r2
    sub r1, r1, r3

    // return
    mov r0, r1
    bx lr
