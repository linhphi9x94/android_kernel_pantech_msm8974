/* 
 * Copyright (C) 2002 Jeff Dike (jdike@karaya.com)
 * Licensed under the GPL
 */

#ifndef __FRAME_KERN_H_
#define __FRAME_KERN_H_

#define _S(nr) (1<<((nr)-1))
#define _BLOCKABLE (~(_S(SIGKILL) | _S(SIGSTOP)))

extern int setup_signal_stack_sc(unsigned long stack_top, int sig, 
				 struct k_sigaction *ka,
<<<<<<< HEAD
				 struct pt_regs *regs, 
				 sigset_t *mask);
extern int setup_signal_stack_si(unsigned long stack_top, int sig, 
				 struct k_sigaction *ka,
				 struct pt_regs *regs, siginfo_t *info, 
=======
				 struct pt_regs *regs,
				 sigset_t *mask);
extern int setup_signal_stack_si(unsigned long stack_top, int sig,
				 struct k_sigaction *ka,
				 struct pt_regs *regs, struct siginfo *info,
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
				 sigset_t *mask);

#endif

