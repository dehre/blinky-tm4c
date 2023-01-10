        .thumb
        .text
        .align 2

        .ref  runPt            ; currently running thread
        .ref  OS_Scheduler
        .def  OSAsm_Start
        .def  OSAsm_ThreadSwitch

runPtAddr .field runPt, 32

OSAsm_Start:   .asmfunc
    CPSID   I                  ; Disable interrupts at processor level
    LDR     R0, runPtAddr      ; currently running thread
    LDR     R2, [R0]           ; R2 = value of RunPt
    LDR     SP, [R2]           ; new thread SP; SP = RunPt->stackPointer;
    POP     {R4-R11}           ; restore regs r4-11
    POP     {R0-R3}            ; restore regs r0-3
    POP     {R12}
    POP     {LR}               ; discard LR from initial stack
    POP     {LR}               ; start location
    POP     {R1}               ; discard PSR
    CPSIE   I                  ; enable interrupts at processor level
    BX      LR                 ; start first thread
   .endasmfunc

OSAsm_ThreadSwitch:  .asmfunc  ; Save R0-R3,R12,LR,PC,PSR
    CPSID   I                  ; prevent interrupt during switch
    PUSH    {R4-R11}           ; save remaining regs r4-11
    LDR     R0, runPtAddr      ; R0=pointer to RunPt, old thread
    LDR     R1, [R0]           ; R1 = RunPt
    STR     SP, [R1]           ; save SP into TCB
    PUSH    {R0, LR}
    BL      OS_Scheduler
    POP     {R0, LR}
    LDR     R1, [R0]           ; R1 = RunPt, new thread
    LDR     SP, [R1]           ; new thread SP; SP = RunPt->sp;
    POP     {R4-R11}           ; restore regs r4-11
    CPSIE   I                  ; tasks run with interrupts enabled
    BX      LR                 ; restore R0-R3,R12,LR,PC,PSR
   .endasmfunc

   .end
