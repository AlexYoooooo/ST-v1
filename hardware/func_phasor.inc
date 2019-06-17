FUNC_PHASOR: ;Instruction phasor
;Provide the address of the phasor struct, that is
;	+0	PHASOR (2)
;	+2	NESTLEVEL (1)
;	+3	CURRENT (2)
;	+5	NESTCTR (14*2)
;	+33	NESTLOCATION (14*2)
;	+61	(END, remainder 3)
;Return the next instruction, which is the direction and the time delay
;The struct will be modified

;Param 1:	[RT7:RT6]	Pointer to phasor struct

;Return:	[RT7:RT6]

#define	STRUCT			_RTB
#define	STRUCT_PHASOR_L		STRUCT+0
#define	STRUCT_PHASOR_H		STRUCT+1
#define	STRUCT_NESTLEVEL	STRUCT+2
#define	STRUCT_CURRENT_L	STRUCT+3
#define	STRUCT_CURRENT_H	STRUCT+4
#define	STRUCT_NESTCTR		STRUCT+5
#define	STRUCT_NESTLOCATION	STRUCT+33
#define	STRUCT_NESTCTR_A	5
#define	STRUCT_NESTLOCATION_A	33

;Get pointer of @0 array in RTA with displacement of nestLevel
.MACRO	RTAPTR
	;Addr_array[nestLevel] = Addr_struct + Addr_(array-struct) + 2 * nestLevel
	LDD	_RA, STRUCT_NESTLEVEL			;Get nestLevel
	LSL	_RA					;The array is 16-bit, therefore the pointer should x2
	MOVW	_RTAL, _RTBL				;Get base address of the struct
	ADIW	_RTAL, @0				;Get base address of the array
	ADD	_RTAL, _RA				;Get address of the array element
	ADC	_RTAH, _RZERO
	.ENDMACRO

;Get pointer of @0 array in RTA with displacement of nestLevel - 1
.MACRO	RTAPTRS
	;Addr_array[nestLevel] = Addr_struct + Addr_(array-struct) + 2 * nestLevel
	LDD	_RA, STRUCT_NESTLEVEL			;Get nestLevel
	DEC	_RA					;Get nestLevel - 1
	LSL	_RA					;The array is 16-bit, therefore the pointer should x2
	MOVW	_RTAL, _RTBL				;Get base address of the struct
	ADIW	_RTAL, @0				;Get base address of the array
	ADD	_RTAL, _RA				;Get address of the array element
	ADC	_RTAH, _RZERO
	.ENDMACRO

			
	MOVW	_RTBL, _RT6				;Address of the struct

;DO NOT MODIFY _RTB (R31:R30): Frame pointer to phasor data struct
	
	FUNC_PHASOR_read:
	LDD	_RTAH, STRUCT_PHASOR_H			;Get current phasor location
	LDD	_RTAL, STRUCT_PHASOR_L
	MOVW	_RT2, _RTAL				;Save curreent phasor location in [RT3:RT2]
	LD	_RT4, _RTA+				;Read instruction, save instruction in [RT7:RT4]
	LD	_RT5, _RTA+				;	as Execution count[16], Dir[1], Delay[15]
	LD	_RT6, _RTA+
	LD	_RT7, _RTA

	;=======================================================================================;
	;	RT7:RT6			RT5:RT4			RT3:RT2				;
	;	Execution count		Direction + Delay	Current phasor location		;
	;=======================================================================================;

	CPSE	_RZERO, _RT4				;If Instruction_Delay low is not 0x00, that is a normal instruction
	JMP	FUNC_PHASOR_normal

	TST	_RT5					;If Instruction_Delay is 0x0000, that is LOOP OUT
	BREQ	FUNC_PHASOR_loopout

	LDI	_RT0, 0x80				;If Instruction_Delay is not 0x8000, that is normal instruction
	CPSE	_RT0, _RT5
	JMP	FUNC_PHASOR_normal			;Otherwise, that is LOOP ENTER


	FUNC_PHASOR_loopenter:
	RTAPTR	STRUCT_NESTLOCATION_A			;Pointer to nestLocation[nestLevel]
	ST	_RTA+, _RT2				;Save current phasor location
	ST	_RTA, _RT3

	RTAPTR	STRUCT_NESTCTR_A			;Pointer to nestCtr[nestLevel]
	ST	_RTA+, _RT6				;Save execution count
	ST	_RTA, _RT7

	LDD	_RA, STRUCT_NESTLEVEL			;nestLevel++
	INC	_RA
	STD	STRUCT_NESTLEVEL, _RA

	MOVW	_RTAL, _RT2				;Save phasor++
	ADIW	_RTAL, 4
	STD	STRUCT_PHASOR_H, _RTAH
	STD	STRUCT_PHASOR_L, _RTAL
	
	JMP	FUNC_PHASOR_read			;Read next instruction


	FUNC_PHASOR_loopout:
	RTAPTRS	STRUCT_NESTCTR_A			;Pointer to nestCtr[nestLevel-1]
	LD	_RT0, _RTA+				;Get value in nestCtr[nestLevel-1]
	LD	_RT1, _RTA

	FUNC_PHASOR_loopout_deadloop:
	CLR	_RA					;Is nestCtr[nestLevel-1] 0xFFFF?
	DEC	_RA					;If yes, then it is a dead loop
	CPSE	_RA, _RT0				;Otherwaise, check for loop end
	JMP	FUNC_PHASOR_loopout_end
	CPSE	_RA, _RT1
	JMP	FUNC_PHASOR_loopout_end

	RTAPTRS	STRUCT_NESTLOCATION_A			;Pointer to nestLocation[nestLevel-1]
	LD	_RT0, _RTA+				;Get value in nestLocation[nestLevel-1]
	LD	_RT1, _RTA
	MOVW	_RTAL, _RT0				;Notice: the location saved in nestLocation[] is the beginning of the loop
	ADIW	_RTAL, 4				;	however, we need to return to the beginning of the content of the loop
							;	instead of the entire boad of the loop.
	STD	STRUCT_PHASOR_H, _RTAH
	STD	STRUCT_PHASOR_L, _RTAL

	JMP	FUNC_PHASOR_read			;Read next instruction

	FUNC_PHASOR_loopout_end:
	CPSE	_RZERO, _RT0				;Is nestCtr[nestLevel-1] 0x0000?
	JMP	FUNC_PHASOR_loopout_continue		;If yes, then it is the end of the loop
	CPSE	_RZERO, _RT1				;Otherwaise, loop in progress
	JMP	FUNC_PHASOR_loopout_continue

	LDD	_RA, STRUCT_NESTLEVEL			;nestLevel--
	DEC	_RA
	STD	STRUCT_NESTLEVEL, _RA

	MOVW	_RTAL, _RT2				;Save phasor++
	ADIW	_RTAL, 4
	STD	STRUCT_PHASOR_H, _RTAH
	STD	STRUCT_PHASOR_L, _RTAL
	
	JMP	FUNC_PHASOR_read			;Read next instruction

	FUNC_PHASOR_loopout_continue:
	RTAPTRS	STRUCT_NESTCTR_A			;nestCtr[nestLevel-1]--
	SUBI	_RT0, 1
	SBC	_RT1, _RZERO
	ST	_RTA+, _RT0
	ST	_RTA, _RT1
	
	RTAPTRS	STRUCT_NESTLOCATION_A			;Return to the beginning of the loop
	LD	_RT0, _RTA+				;See FUNC_PHASOR_loopout_deadloop
	LD	_RT1, _RTA
	MOVW	_RTAL, _RT0
	ADIW	_RTAL, 4
	STD	STRUCT_PHASOR_H, _RTAH
	STD	STRUCT_PHASOR_L, _RTAL

	JMP	FUNC_PHASOR_read			;Read next instruction

	
	FUNC_PHASOR_normal:
	LDD	_RT0, STRUCT_CURRENT_L			;Is currentCrt 0?
	LDD	_RT1, STRUCT_CURRENT_H			;If yes, then, new instruction
	CPSE	_RT1, _RZERO				;Otherwise, old instruction
	JMP	FUNC_PHASOR_normal_old
	CPSE	_RT0, _RZERO
	JMP	FUNC_PHASOR_normal_old

	FUNC_PHASOR_normal_new:
	STD	STRUCT_CURRENT_H, _RT7			;Set currentCtr
	STD	STRUCT_CURRENT_L, _RT6

	MOVW	_RT6, _RT4				;Return Delay and Direction
	RET

	FUNC_PHASOR_normal_old:
	SUBI	_RT0, 1					;Save currentCtr--
	SBC	_RT1, _RZERO
	STD	STRUCT_CURRENT_H, _RT1
	STD	STRUCT_CURRENT_L, _RT0
	
	CPSE	_RT0, _RZERO				;If new currentCtr == 0, phasor++
	JMP	FUNC_PHASOR_normal_old_continue
	CPSE	_RT1, _RZERO
	JMP	FUNC_PHASOR_normal_old_continue

	MOVW	_RTAL, _RT2				;Save phasor++
	ADIW	_RTAL, 4
	STD	STRUCT_PHASOR_H, _RTAH
	STD	STRUCT_PHASOR_L, _RTAL

	FUNC_PHASOR_normal_old_continue:
	MOVW	_RT6, _RT4				;Return Delay and Direction
	RET


#undef	STRUCT
#undef	STRUCT_PHASOR_L
#undef	STRUCT_PHASOR_H
#undef	STRUCT_NESTLEVEL
#undef	STRUCT_CURRENT_L
#undef	STRUCT_CURRENT_H
#undef	STRUCT_NESTCTR
#undef	STRUCT_NESTLOCATION
#undef	STRUCT_NESTCTR_A
#undef	STRUCT_NESTLOCATION_A
