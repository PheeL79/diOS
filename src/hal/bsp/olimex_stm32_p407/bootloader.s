#include "config.asm"

        MODULE  ?bootloader

        SECTION .bootloader:CODE:NOROOT(2)

        PUBLIC  __vector_bootloader

        DATA
__vector_bootloader
        DCD     0
        DCD     Bootloader                          ; Reset Handler
        DCD     NMI_Handler                         ; NMI Handler
        DCD     HardFault_Handler                   ; Hard Fault Handler

        THUMB
        PUBWEAK Bootloader
        SECTION .text:CODE:NOROOT:REORDER(1)
Bootloader

        LDR     R1, =FIRMWARE_ADDRESS_FLASH         ;__firmware_stack_addr
        LDR     R0, [R1]
        MSR     MSP, R0
        ADD     R1, R1, #0x4                        ;__firmware_reset_addr
        LDR     R0, [R1]
        BX      R0

        PUBWEAK NMI_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
NMI_Handler
        B NMI_Handler

        PUBWEAK HardFault_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
HardFault_Handler
        B HardFault_Handler

        END