    xdef _plasma_render_asm

    ; void __asm plasma_render_asm(
    ;     register __a0 UBYTE *buffer,      ; Output buffer
    ;     register __a1 UBYTE *sine_table,  ; Sine table 1
    ;     register __a2 UBYTE *sine_table2, ; Sine table 2
    ;     register __a3 UBYTE *yAddTable,   ; Y Add Table
    ;     register __a4 UBYTE *ySubTable,   ; Y Sub Table
    ;     register __a5 UBYTE *ySin2Table,  ; Y Sine Table
    ;     register __d0 UBYTE t1,
    ;     register __d1 UBYTE t2,
    ;     register __d2 UBYTE t3
    ; );

_plasma_render_asm:
    ; Save ALL preserved registers - is this needed for SAS/C ?
    movem.l d2-d7/a2-a6,-(sp)

    moveq   #59,d7          ; Y loop counter
    
.y_loop:
    ; Read Y-table values and increment pointers
    moveq   #0,d3
    move.b  (a3)+,d3        ; yAdd = *yAddTable++
    
    moveq   #0,d4
    move.b  (a4)+,d4        ; ySub = *ySubTable++
    
    move.b  (a5)+,d5        ; ySinVal = *ySin2Table++
    
    ; Save the incremented Y-table pointers so we can reuse registers A3-A5
    movem.l a3-a5,-(sp)
    
    ; Setup pointers for the inner loop
    ; P1 (Term 1) = sine_table + t1 + yAdd
    ; P2 (Term 2) = sine_table + t2 + ySub
    ; P3 (Term 3) = sine_table2 + t3
    
    ; Calculate P1 in A3
    move.l  a1,a3           ; A3 = sine_table
    add.w   d0,a3           ; + t1
    add.w   d3,a3           ; + yAdd
    
    ; Calculate P2 in A4
    move.l  a1,a4           ; A4 = sine_table
    add.w   d1,a4           ; + t2
    add.w   d4,a4           ; + ySub
    
    ; Calculate P3 in A6 (we use A6 as scratch for P3)
    move.l  a2,a6           ; A6 = sine_table2
    add.w   d2,a6           ; + t3
    
    ; Now:
    ; A0 = Output Buffer
    ; A3 = P1
    ; A4 = P2
    ; A6 = P3
    ; D5 = ySinVal
    
    moveq   #9,d6           ; X loop counter (10 blocks of 8 pixels = 80 pixels)

.pixel_loop:
    moveq   #0,d4           ; Byte accumulator (reuse D4, we don't need ySub anymore)
    
    ; --- Pixel 0 ---
    moveq   #0,d3           ; Scratch for pixel value
    move.b  (a3)+,d3        ; Term 1
    add.b   (a4),d3         ; Term 2 (P2 increments by 2)
    addq.l  #2,a4
    add.b   (a6)+,d3        ; Term 3
    add.b   d5,d3           ; + ySinVal
    lsr.b   #2,d3
    btst    #0,d3
    beq.s   .p0_skip
    bset    #7,d4
.p0_skip:

    ; --- Pixel 1 ---
    moveq   #0,d3
    move.b  (a3)+,d3
    add.b   (a4),d3
    addq.l  #2,a4
    add.b   (a6)+,d3
    add.b   d5,d3
    lsr.b   #2,d3
    btst    #0,d3
    beq.s   .p1_skip
    bset    #6,d4
.p1_skip:

    ; --- Pixel 2 ---
    moveq   #0,d3
    move.b  (a3)+,d3
    add.b   (a4),d3
    addq.l  #2,a4
    add.b   (a6)+,d3
    add.b   d5,d3
    lsr.b   #2,d3
    btst    #0,d3
    beq.s   .p2_skip
    bset    #5,d4
.p2_skip:

    ; --- Pixel 3 ---
    moveq   #0,d3
    move.b  (a3)+,d3
    add.b   (a4),d3
    addq.l  #2,a4
    add.b   (a6)+,d3
    add.b   d5,d3
    lsr.b   #2,d3
    btst    #0,d3
    beq.s   .p3_skip
    bset    #4,d4
.p3_skip:

    ; --- Pixel 4 ---
    moveq   #0,d3
    move.b  (a3)+,d3
    add.b   (a4),d3
    addq.l  #2,a4
    add.b   (a6)+,d3
    add.b   d5,d3
    lsr.b   #2,d3
    btst    #0,d3
    beq.s   .p4_skip
    bset    #3,d4
.p4_skip:

    ; --- Pixel 5 ---
    moveq   #0,d3
    move.b  (a3)+,d3
    add.b   (a4),d3
    addq.l  #2,a4
    add.b   (a6)+,d3
    add.b   d5,d3
    lsr.b   #2,d3
    btst    #0,d3
    beq.s   .p5_skip
    bset    #2,d4
.p5_skip:

    ; --- Pixel 6 ---
    moveq   #0,d3
    move.b  (a3)+,d3
    add.b   (a4),d3
    addq.l  #2,a4
    add.b   (a6)+,d3
    add.b   d5,d3
    lsr.b   #2,d3
    btst    #0,d3
    beq.s   .p6_skip
    bset    #1,d4
.p6_skip:

    ; --- Pixel 7 ---
    moveq   #0,d3
    move.b  (a3)+,d3
    add.b   (a4),d3
    addq.l  #2,a4
    add.b   (a6)+,d3
    add.b   d5,d3
    lsr.b   #2,d3
    btst    #0,d3
    beq.s   .p7_skip
    bset    #0,d4
.p7_skip:

    move.b  d4,(a0)+
    
    dbf     d6,.pixel_loop
    
    ; Restore Y-table pointers for next line
    movem.l (sp)+,a3-a5
    
    dbf     d7,.y_loop

    ; Restore registers
    movem.l (sp)+,d2-d7/a2-a6
    rts
