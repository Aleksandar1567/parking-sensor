;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
;
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file


;-------------------------------------------------------------------------------
				.ref	data					; reference the global variable

;-------------------------------------------------------------------------------

				.text

TIMER1_A0_ISR	;bic.w   #0x00f0,0x0000(SP)
				add.w   &data,&TA1CCR0
				bic.w   #0x00d0,0x0000(SP)
				reti	; back to the main loop

TIMER1_A1_ISR	;bic.w   #0x00f0,0x0000(SP)
				clr.w   &TA1IV
				bic.w   #0x00d0,0x0000(SP)
				reti
;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".int49"                ; MSP430 TIMER1_A0_ISR Vector
            .short  TIMER1_A0_ISR
			.sect   ".int48"				; MSP430 TIMER1_A1_ISR Vector
			.short  TIMER1_A1_ISR
