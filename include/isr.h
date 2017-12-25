#ifndef __ISR_H__
#define __ISR_H__

/* Interrupt Service Routines */

void ISR_div0(void);
void ISR_debug(void);
void ISR_nmi(void);
void ISR_breakpoint(void);
void ISR_overflow(void);
void ISR_boundrange(void);
void ISR_invopcode(void);
void ISR_nodev(void);
void ISR_df(void);
void ISR_copoverrun(void);
void ISR_invtss(void);
void ISR_noseg(void);
void ISR_sf(void);
void ISR_gp(void);
void ISR_pf(void);
void ISR_r15(void);
void ISR_fpu(void);
void ISR_align(void);
void ISR_machine(void);
void ISR_simd(void);
void ISR_exception(void);

/* From PIC */
void ISR_timer(void);
void ISR_keyboard(void);
void ISR_slave(void);
void ISR_serial2(void);
void ISR_serial1(void);
void ISR_parallel2(void);
void ISR_floppy(void);
void ISR_parallel1(void);
void ISR_rtc(void);
void ISR_res(void);
void ISR_nouse1(void);
void ISR_nouse2(void);
void ISR_mouse(void);
void ISR_cop(void);
void ISR_hdd1(void);
void ISR_hdd2(void);
void ISR_interrupt(void);


#endif
