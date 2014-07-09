        MODULE  ?.text
        
        SECTION .text:CODE:ROOT(2)
 
        THUMB
        PUBWEAK BitCount
;R0 - Value 
;R0 = Number of ones
;Uses R0-R1
;Thumb-2 version
BitCount
        and	r1,r0,#0xaaaaaaaa
        sub	r0,r0,r1,lsr #1

        and	r1,r0,#0xcccccccc
        and	r0,r0,#0x33333333
        add	r0,r0,r1,lsr #2

        add	r0,r0,r0,lsr #4
        and	r0,r0,#0x0f0f0f0f

        add	r0,r0,r0,lsr #8
        add	r0,r0,r0,lsr #16
        and	r0,r0,#63
        bx  lr
        
        END    