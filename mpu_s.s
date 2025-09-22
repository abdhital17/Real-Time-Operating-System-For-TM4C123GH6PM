; MPU assembly functions

	.def unprivilegedMode
	.def setPSPaddress
	.def getPSPaddress
	.def getMSPaddress
	.def setASPbit


.thumb
.const

.text

setPSPaddress:
		MSR 	PSP, R0							; uses the passed argument to set it as the PSP address and returns
		BX		LR

getPSPaddress:
		MRS 	R0, PSP							; load PSP address in R0 and return
		BX 		LR

getMSPaddress:
		MRS 	R0, MSP							; load MSP address in R0 and return
		BX		LR

unprivilegedMode:
			MRS 	R4, CONTROL
			ORR 	R4, R4, #0x01				; set the TMPL bit in CONTROL register
			MSR		CONTROL, R4
			BX 		LR

setASPbit:
			MRS 	R4, CONTROL
			ORR 	R4, R4, #0x2				; set the ASP bit in CONTROL register
			MSR		CONTROL, R4
			BX 		LR


.endm