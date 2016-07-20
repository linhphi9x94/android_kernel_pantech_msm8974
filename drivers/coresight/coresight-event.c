<<<<<<< HEAD
/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
=======
/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

<<<<<<< HEAD
/*
 * DLKM to register a callback with a ftrace event
 */
=======
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/tracepoint.h>
#include <linux/coresight.h>

#include <trace/events/exception.h>

<<<<<<< HEAD
static void abort_coresight_tracing(void *ignore, struct task_struct *task,\
					unsigned long addr, unsigned int fsr)
{
	coresight_abort();
	pr_debug("control_trace: task_name: %s, addr: %lu, fsr:%u",\
		(char *)task->comm, addr, fsr);
}

static void abort_tracing_undef_instr(void *ignore, struct pt_regs *regs,\
					void *pc)
{
	if (user_mode(regs)) {
		coresight_abort();
		pr_debug("control_trace: pc: %p", pc);
	}
}

static int __init control_trace_init(void)
{
	int ret_user_fault, ret_undef_instr;
	ret_user_fault = register_trace_user_fault(abort_coresight_tracing,\
							NULL);
	ret_undef_instr = register_trace_undef_instr(abort_tracing_undef_instr,\
							NULL);
	if (ret_user_fault != 0 || ret_undef_instr != 0) {
		pr_info("control_trace: Module Not Registered\n");
		return (ret_user_fault < 0 ?\
			ret_user_fault : ret_undef_instr);
	}
	pr_info("control_trace: Module Registered\n");
	return 0;
}

module_init(control_trace_init);

static void __exit control_trace_exit(void)
{
	unregister_trace_user_fault(abort_coresight_tracing, NULL);
	unregister_trace_undef_instr(abort_tracing_undef_instr, NULL);
	pr_info("control_trace: Module Removed\n");
}

module_exit(control_trace_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Kernel Module to abort tracing");
=======
static int event_abort_enable;
static int event_abort_set(const char *val, struct kernel_param *kp);
module_param_call(event_abort_enable, event_abort_set, param_get_int,
		  &event_abort_enable, 0644);

static void event_abort_user_fault(void *ignore,
				   struct task_struct *task,
				   unsigned long addr,
				   unsigned int fsr)
{
	coresight_abort();
	pr_debug("coresight_event: task_name: %s, addr: %lu, fsr:%u",
		(char *)task->comm, addr, fsr);
}

static void event_abort_undef_instr(void *ignore,
				    struct pt_regs *regs,
				    void *pc)
{
	if (user_mode(regs)) {
		coresight_abort();
		pr_debug("coresight_event: pc: %p", pc);
	}
}

static void event_abort_unhandled_abort(void *ignore,
					struct pt_regs *regs,
					unsigned long addr,
					unsigned int fsr)
{
	if (user_mode(regs)) {
		coresight_abort();
		pr_debug("coresight_event: addr: %lu, fsr:%u", addr, fsr);
	}
}

static int event_abort_register(void)
{
	int ret;

	ret = register_trace_user_fault(event_abort_user_fault, NULL);
	if (ret)
		goto err_usr_fault;
	ret = register_trace_undef_instr(event_abort_undef_instr, NULL);
	if (ret)
		goto err_undef_instr;
	ret = register_trace_unhandled_abort(event_abort_unhandled_abort, NULL);
	if (ret)
		goto err_unhandled_abort;

	return 0;

err_unhandled_abort:
	unregister_trace_undef_instr(event_abort_undef_instr, NULL);
err_undef_instr:
	unregister_trace_user_fault(event_abort_user_fault, NULL);
err_usr_fault:
	return ret;
}

static void event_abort_unregister(void)
{
	unregister_trace_user_fault(event_abort_user_fault, NULL);
	unregister_trace_undef_instr(event_abort_undef_instr, NULL);
	unregister_trace_unhandled_abort(event_abort_unhandled_abort, NULL);
}

static int event_abort_set(const char *val, struct kernel_param *kp)
{
	int ret;

	ret = param_set_int(val, kp);
	if (ret) {
		pr_err("coresight_event: error setting value %d\n", ret);
		return ret;
	}

	if (event_abort_enable)
		ret = event_abort_register();
	else
		event_abort_unregister();

	return ret;
}

static int __init event_init(void)
{
	return 0;
}
module_init(event_init);

static void __exit event_exit(void)
{
}
module_exit(event_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Coresight Event driver to abort tracing");
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
