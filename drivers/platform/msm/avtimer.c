<<<<<<< HEAD

/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
=======
/* Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/avtimer.h>
<<<<<<< HEAD
#include <mach/qdsp6v2/apr.h>

#define DEVICE_NAME "avtimer"


#define ADSP_CMD_SET_POWER_COLLAPSE_STATE 0x0001115C

static int major;	/* Major number assigned to our device driver */
struct avtimer_t {
=======
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <mach/qdsp6v2/apr.h>
#include <sound/q6core.h>

#define DEVICE_NAME "avtimer"
#define TIMEOUT_MS 1000
#define CORE_CLIENT 1
#define TEMP_PORT ((CORE_CLIENT << 8) | 0x0001)
#define SSR_WAKETIME 1000
#define Q6_READY_RETRY 250
#define Q6_READY_MAX_RETRIES 40

#define AVCS_CMD_REMOTE_AVTIMER_VOTE_REQUEST 0x00012914
#define AVCS_CMD_RSP_REMOTE_AVTIMER_VOTE_REQUEST 0x00012915
#define AVCS_CMD_REMOTE_AVTIMER_RELEASE_REQUEST 0x00012916
#define AVTIMER_REG_CNT 2

struct adsp_avt_timer {
	struct apr_hdr hdr;
	union {
		char client_name[8];
		u32 avtimer_handle;
	};
} __packed;

static int major;

struct avtimer_t {
	struct apr_svc *core_handle_q;
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
	struct cdev myc;
	struct class *avtimer_class;
	struct mutex avtimer_lock;
	int avtimer_open_cnt;
<<<<<<< HEAD
	struct dev_avtimer_data *avtimer_pdata;
};
static struct avtimer_t avtimer;

static struct apr_svc *core_handle;

struct adsp_power_collapse {
	struct apr_hdr hdr;
	uint32_t power_collapse;
};

static int32_t avcs_core_callback(struct apr_client_data *data, void *priv)
{
	uint32_t *payload;

	pr_debug("core msg: payload len = %u, apr resp opcode = 0x%X\n",
		data->payload_size, data->opcode);
=======
	struct dev_avtimer_data avtimer_pdata;
	struct delayed_work ssr_dwork;
	wait_queue_head_t adsp_resp_wait;
	int enable_timer_resp_recieved;
	int timer_handle;
	void __iomem *p_avtimer_msw;
	void __iomem *p_avtimer_lsw;
	uint32_t clk_div;
	atomic_t adsp_ready;
	int num_retries;
};

static struct avtimer_t avtimer;

static int32_t aprv2_core_fn_q(struct apr_client_data *data, void *priv)
{
	uint32_t *payload1;

	pr_debug("%s: core msg: payload len = %u, apr resp opcode = 0x%X\n",
		__func__, data->payload_size, data->opcode);

	if (!data) {
		pr_err("%s: Invalid params\n", __func__);
		return -EINVAL;
	}
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0

	switch (data->opcode) {

	case APR_BASIC_RSP_RESULT:{

<<<<<<< HEAD
		if (data->payload_size == 0) {
=======
		if (!data->payload_size) {
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
			pr_err("%s: APR_BASIC_RSP_RESULT No Payload ",
					__func__);
			return 0;
		}

<<<<<<< HEAD
		payload = data->payload;

		switch (payload[0]) {

		case ADSP_CMD_SET_POWER_COLLAPSE_STATE:
			pr_debug("CMD_SET_POWER_COLLAPSE_STATE status[0x%x]\n",
					payload[1]);
			break;
		default:
			pr_err("Invalid cmd rsp[0x%x][0x%x]\n",
					payload[0], payload[1]);
=======
		payload1 = data->payload;
		switch (payload1[0]) {
		case AVCS_CMD_REMOTE_AVTIMER_RELEASE_REQUEST:
			pr_debug("%s: Cmd = TIMER RELEASE status[0x%x]\n",
			__func__, payload1[1]);
			break;
		default:
			pr_err("Invalid cmd rsp[0x%x][0x%x]\n",
					payload1[0], payload1[1]);
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
			break;
		}
		break;
	}
<<<<<<< HEAD
	case RESET_EVENTS:{
		pr_debug("Reset event received in Core service");
		apr_reset(core_handle);
		core_handle = NULL;
		break;
	}

	default:
		pr_err("Message id from adsp core svc: %d\n", data->opcode);
=======

	case RESET_EVENTS:{
		pr_debug("%s: Reset event received in AV timer\n", __func__);
		apr_reset(avtimer.core_handle_q);
		avtimer.core_handle_q = NULL;
		avtimer.avtimer_open_cnt = 0;
		atomic_set(&avtimer.adsp_ready, 0);
		schedule_delayed_work(&avtimer.ssr_dwork,
				  msecs_to_jiffies(SSR_WAKETIME));
		break;
	}

	case AVCS_CMD_RSP_REMOTE_AVTIMER_VOTE_REQUEST:
		payload1 = data->payload;
		pr_debug("%s: RSP_REMOTE_AVTIMER_VOTE_REQUEST handle %x\n",
			__func__, payload1[0]);
		avtimer.timer_handle = payload1[0];
		avtimer.enable_timer_resp_recieved = 1;
		wake_up(&avtimer.adsp_resp_wait);
		break;
	default:
		pr_err("%s: Message adspcore svc: %d\n",
				__func__, data->opcode);
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
		break;
	}

	return 0;
}

int avcs_core_open(void)
{
<<<<<<< HEAD
	if (core_handle == NULL)
		core_handle = apr_register("ADSP", "CORE",
					avcs_core_callback, 0xFFFFFFFF, NULL);

	pr_debug("Open_q %p\n", core_handle);
	if (core_handle == NULL) {
		pr_err("%s: Unable to register CORE\n", __func__);
		return -ENODEV;
	}
	return 0;
}

int avcs_core_disable_power_collapse(int disable)
{
	struct adsp_power_collapse pc;
	int rc = 0;

	if (core_handle) {
		pc.hdr.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_EVENT,
			APR_HDR_LEN(APR_HDR_SIZE), APR_PKT_VER);
		pc.hdr.pkt_size = APR_PKT_SIZE(APR_HDR_SIZE,
					sizeof(uint32_t));
		pc.hdr.src_port = 0;
		pc.hdr.dest_port = 0;
		pc.hdr.token = 0;
		pc.hdr.opcode = ADSP_CMD_SET_POWER_COLLAPSE_STATE;
		/*
		* When power_collapse set to 1 -- If the aDSP is in the power
		* collapsed state when this command is received, it is awakened
		* from this state. The aDSP does not power collapse again until
		* the client revokes this	command
		* When power_collapse set to 0 -- This indicates to the aDSP
		* that the remote client does not need it to be out of power
		* collapse any longer. This may not always put the aDSP into
		* power collapse; the aDSP must honor an internal client's
		* power requirements as well.
		*/
		pc.power_collapse = disable;
		rc = apr_send_pkt(core_handle, (uint32_t *)&pc);
		if (rc < 0) {
			pr_debug("disable power collapse = %d failed\n",
				disable);
			return rc;
		}
		pr_debug("disable power collapse = %d\n", disable);
	}
	return 0;
}

static int avtimer_open(struct inode *inode, struct file *file)
{
	int rc = 0;
	struct avtimer_t *pavtimer = &avtimer;

	pr_debug("avtimer_open\n");
	mutex_lock(&pavtimer->avtimer_lock);

	if (pavtimer->avtimer_open_cnt != 0) {
		pavtimer->avtimer_open_cnt++;
		pr_debug("%s: opened avtimer open count=%d\n",
			__func__, pavtimer->avtimer_open_cnt);
		mutex_unlock(&pavtimer->avtimer_lock);
		return 0;
	}
	try_module_get(THIS_MODULE);

	rc = avcs_core_open();
	if (core_handle)
		rc = avcs_core_disable_power_collapse(1);

	pavtimer->avtimer_open_cnt++;
	pr_debug("%s: opened avtimer open count=%d\n",
		__func__, pavtimer->avtimer_open_cnt);
	mutex_unlock(&pavtimer->avtimer_lock);
	pr_debug("avtimer_open leave rc=%d\n", rc);

	return rc;
}

static int avtimer_release(struct inode *inode, struct file *file)
{
	int rc = 0;
	struct avtimer_t *pavtimer = &avtimer;

	mutex_lock(&pavtimer->avtimer_lock);
	pavtimer->avtimer_open_cnt--;

	if (core_handle && pavtimer->avtimer_open_cnt == 0)
		rc = avcs_core_disable_power_collapse(0);

	pr_debug("device_release(%p,%p) open count=%d\n",
		inode, file, pavtimer->avtimer_open_cnt);

	module_put(THIS_MODULE);

	mutex_unlock(&pavtimer->avtimer_lock);

	return rc;
=======
	if (!avtimer.core_handle_q)
		avtimer.core_handle_q = apr_register("ADSP", "CORE",
					aprv2_core_fn_q, TEMP_PORT, NULL);
	pr_debug("%s: Open_q %p\n", __func__, avtimer.core_handle_q);
	if (!avtimer.core_handle_q) {
		pr_err("%s: Unable to register CORE\n", __func__);
		return -EINVAL;
	}
	return 0;
}
EXPORT_SYMBOL(avcs_core_open);

static int avcs_core_disable_avtimer(int timerhandle)
{
	int rc = -EINVAL;
	struct adsp_avt_timer payload;

	if (!timerhandle) {
		pr_err("%s: Invalid timer handle\n", __func__);
		return -EINVAL;
	}
	memset(&payload, 0, sizeof(payload));
	rc = avcs_core_open();
	if (!rc && avtimer.core_handle_q) {
		payload.hdr.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_SEQ_CMD,
			APR_HDR_LEN(APR_HDR_SIZE), APR_PKT_VER);
		payload.hdr.pkt_size =
			sizeof(struct adsp_avt_timer);
		payload.hdr.src_svc = avtimer.core_handle_q->id;
		payload.hdr.src_domain = APR_DOMAIN_APPS;
		payload.hdr.dest_domain = APR_DOMAIN_ADSP;
		payload.hdr.dest_svc = APR_SVC_ADSP_CORE;
		payload.hdr.src_port = TEMP_PORT;
		payload.hdr.dest_port = TEMP_PORT;
		payload.hdr.token = CORE_CLIENT;
		payload.hdr.opcode = AVCS_CMD_REMOTE_AVTIMER_RELEASE_REQUEST;
		payload.avtimer_handle = timerhandle;
		pr_debug("%s: disable avtimer opcode %x handle %x\n",
			__func__, payload.hdr.opcode, payload.avtimer_handle);
		rc = apr_send_pkt(avtimer.core_handle_q,
						(uint32_t *)&payload);
		if (rc < 0)
			pr_err("%s: Enable AVtimer failed op[0x%x]rc[%d]\n",
				__func__, payload.hdr.opcode, rc);
		else
			rc = 0;
	}
	return rc;
}

static int avcs_core_enable_avtimer(char *client_name)
{
	int rc = -EINVAL, ret = -EINVAL;
	struct adsp_avt_timer payload;

	if (!client_name) {
		pr_err("%s: Invalid params\n", __func__);
		return -EINVAL;
	}
	memset(&payload, 0, sizeof(payload));
	rc = avcs_core_open();
	if (!rc && avtimer.core_handle_q) {
		avtimer.enable_timer_resp_recieved = 0;
		payload.hdr.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_EVENT,
			APR_HDR_LEN(APR_HDR_SIZE), APR_PKT_VER);
		payload.hdr.pkt_size =
			sizeof(struct adsp_avt_timer);
		payload.hdr.src_svc = avtimer.core_handle_q->id;
		payload.hdr.src_domain = APR_DOMAIN_APPS;
		payload.hdr.dest_domain = APR_DOMAIN_ADSP;
		payload.hdr.dest_svc = APR_SVC_ADSP_CORE;
		payload.hdr.src_port = TEMP_PORT;
		payload.hdr.dest_port = TEMP_PORT;
		payload.hdr.token = CORE_CLIENT;
		payload.hdr.opcode = AVCS_CMD_REMOTE_AVTIMER_VOTE_REQUEST;
		strlcpy(payload.client_name, client_name,
			   sizeof(payload.client_name));
		pr_debug("%s: enable avtimer opcode %x client name %s\n",
			__func__, payload.hdr.opcode, payload.client_name);
		rc = apr_send_pkt(avtimer.core_handle_q,
						(uint32_t *)&payload);
		if (rc < 0) {
			pr_err("%s: Enable AVtimer failed op[0x%x]rc[%d]\n",
				__func__, payload.hdr.opcode, rc);
			goto bail;
		} else
			rc = 0;
		ret = wait_event_timeout(avtimer.adsp_resp_wait,
			(avtimer.enable_timer_resp_recieved == 1),
			msecs_to_jiffies(TIMEOUT_MS));
		if (!ret) {
			pr_err("%s: wait_event timeout for Enable timer\n",
					__func__);
			rc = -ETIMEDOUT;
		}
		if (rc)
			avtimer.timer_handle = 0;
	}
bail:
	return rc;
}

int avcs_core_disable_power_collapse(int enable)
{
	int rc = 0;
	mutex_lock(&avtimer.avtimer_lock);
	if (enable) {
		if (avtimer.avtimer_open_cnt) {
			avtimer.avtimer_open_cnt++;
			pr_debug("%s: opened avtimer open count=%d\n",
				__func__, avtimer.avtimer_open_cnt);
			rc = 0;
			goto done;
		}
		rc = avcs_core_enable_avtimer("timer");
		if (!rc) {
			avtimer.avtimer_open_cnt++;
			atomic_set(&avtimer.adsp_ready, 1);
		}
	} else {
		if (avtimer.avtimer_open_cnt > 0) {
			avtimer.avtimer_open_cnt--;
			if (!avtimer.avtimer_open_cnt) {
				rc = avcs_core_disable_avtimer(
				avtimer.timer_handle);
				avtimer.timer_handle = 0;
			}
		}
	}
done:
	mutex_unlock(&avtimer.avtimer_lock);
	return rc;
}
EXPORT_SYMBOL(avcs_core_disable_power_collapse);

static void reset_work(struct work_struct *work)
{
	if (q6core_is_adsp_ready()) {
		avcs_core_disable_power_collapse(1);
		avtimer.num_retries = Q6_READY_MAX_RETRIES;
		return;
	}
	pr_debug("%s:Q6 not ready-retry after sometime\n", __func__);
	if (--avtimer.num_retries > 0) {
		schedule_delayed_work(&avtimer.ssr_dwork,
			  msecs_to_jiffies(Q6_READY_RETRY));
	} else {
		pr_err("%s: Q6 failed responding after multiple retries\n",
							__func__);
		avtimer.num_retries = Q6_READY_MAX_RETRIES;
	}
}

int avcs_core_query_timer(uint64_t *avtimer_tick)
{
	uint32_t avtimer_msw = 0, avtimer_lsw = 0;
	uint32_t res = 0;
	uint64_t avtimer_tick_temp;

	if (!atomic_read(&avtimer.adsp_ready)) {
		pr_debug("%s:In SSR, return\n", __func__);
		return -ENETRESET;
	}
	avtimer_lsw = ioread32(avtimer.p_avtimer_lsw);
	avtimer_msw = ioread32(avtimer.p_avtimer_msw);

	avtimer_tick_temp =
		(uint64_t)((uint64_t)avtimer_msw << 32)
			| avtimer_lsw;
	res = do_div(avtimer_tick_temp, avtimer.clk_div);
	*avtimer_tick = avtimer_tick_temp;
	pr_debug_ratelimited("%s:Avtimer: msw: %u, lsw: %u, tick: %llu\n",
			__func__,
			avtimer_msw, avtimer_lsw, *avtimer_tick);
	return 0;
}
EXPORT_SYMBOL(avcs_core_query_timer);

static int avtimer_open(struct inode *inode, struct file *file)
{
	return avcs_core_disable_power_collapse(1);
}

static int avtimer_release(struct inode *inode, struct file *file)
{
	return avcs_core_disable_power_collapse(0);
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
}

/*
 * ioctl call provides GET_AVTIMER
 */
static long avtimer_ioctl(struct file *file, unsigned int ioctl_num,
				unsigned long ioctl_param)
{
<<<<<<< HEAD
	struct avtimer_t *pavtimer = &avtimer;
	pr_debug("avtimer_ioctl: ioctlnum=%d,param=%lx\n",
				ioctl_num, ioctl_param);

	switch (ioctl_num) {
	case IOCTL_GET_AVTIMER_TICK:
	{
		void __iomem *p_avtimer_msw = NULL, *p_avtimer_lsw = NULL;
		uint32_t avtimer_msw_1st = 0, avtimer_lsw = 0;
		uint32_t avtimer_msw_2nd = 0;
		uint64_t avtimer_tick;

		if (pavtimer->avtimer_pdata) {
			p_avtimer_lsw = ioremap(
			pavtimer->avtimer_pdata->avtimer_lsw_phy_addr, 4);
			p_avtimer_msw = ioremap(
			pavtimer->avtimer_pdata->avtimer_msw_phy_addr, 4);
		}
		if (!p_avtimer_lsw || !p_avtimer_msw) {
			pr_err("ioremap failed\n");
			return -EIO;
		}
		do {
			avtimer_msw_1st = ioread32(p_avtimer_msw);
			avtimer_lsw = ioread32(p_avtimer_lsw);
			avtimer_msw_2nd = ioread32(p_avtimer_msw);
		} while (avtimer_msw_1st != avtimer_msw_2nd);

		avtimer_tick =
		((uint64_t) avtimer_msw_1st << 32) | avtimer_lsw;

		pr_debug("AV Timer tick: msw: %d, lsw: %d\n", avtimer_msw_1st,
				avtimer_lsw);
		if (copy_to_user((void *) ioctl_param, &avtimer_tick,
				sizeof(avtimer_tick))) {
					pr_err("copy_to_user failed\n");
					iounmap(p_avtimer_lsw);
					iounmap(p_avtimer_msw);
					return -EFAULT;
			}
		iounmap(p_avtimer_lsw);
		iounmap(p_avtimer_msw);
=======
	switch (ioctl_num) {
	case IOCTL_GET_AVTIMER_TICK:
	{
		uint64_t avtimer_tick;

		avcs_core_query_timer(&avtimer_tick);
		pr_debug_ratelimited("%s: AV Timer tick: time %llx\n",
		__func__, avtimer_tick);
		if (copy_to_user((void *) ioctl_param, &avtimer_tick,
				sizeof(avtimer_tick))) {
					pr_err("copy_to_user failed\n");
					return -EFAULT;
			}
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
		}
		break;

	default:
<<<<<<< HEAD
		pr_err("invalid cmd\n");
		break;
	}

=======
		pr_err("%s: invalid cmd\n", __func__);
		return -EINVAL;
	}
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
	return 0;
}

static const struct file_operations avtimer_fops = {
	.unlocked_ioctl = avtimer_ioctl,
	.open = avtimer_open,
	.release = avtimer_release
};

static int dev_avtimer_probe(struct platform_device *pdev)
{
<<<<<<< HEAD
	int result;
	dev_t dev = MKDEV(major, 0);
	struct device *device_handle;
	struct avtimer_t *pavtimer = &avtimer;

=======
	int result = 0;
	dev_t dev = MKDEV(major, 0);
	struct device *device_handle;
	struct resource *reg_lsb = NULL, *reg_msb = NULL;
	uint32_t clk_div_val;

	if (!pdev) {
		pr_err("%s: Invalid params\n", __func__);
		return -EINVAL;
	}
	reg_lsb = platform_get_resource_byname(pdev,
		IORESOURCE_MEM, "avtimer_lsb_addr");
	if (!reg_lsb) {
		dev_err(&pdev->dev, "%s: Looking up %s property",
			"avtimer_lsb_addr", __func__);
		return -EINVAL;
	}
	reg_msb = platform_get_resource_byname(pdev,
		IORESOURCE_MEM, "avtimer_msb_addr");
	if (!reg_msb) {
		dev_err(&pdev->dev, "%s: Looking up %s property",
			"avtimer_msb_addr", __func__);
		return -EINVAL;
	}
	INIT_DELAYED_WORK(&avtimer.ssr_dwork, reset_work);

	avtimer.p_avtimer_lsw = devm_ioremap_nocache(&pdev->dev,
				reg_lsb->start, resource_size(reg_lsb));
	if (!avtimer.p_avtimer_lsw) {
		dev_err(&pdev->dev, "%s: ioremap failed for lsb avtimer register",
			__func__);
		return -ENOMEM;
	}

	avtimer.p_avtimer_msw = devm_ioremap_nocache(&pdev->dev,
				reg_msb->start, resource_size(reg_msb));
	if (!avtimer.p_avtimer_msw) {
		dev_err(&pdev->dev, "%s: ioremap failed for msb avtimer register",
			__func__);
		goto unmap;
	}
	avtimer.num_retries = Q6_READY_MAX_RETRIES;
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
	/* get the device number */
	if (major)
		result = register_chrdev_region(dev, 1, DEVICE_NAME);
	else {
		result = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
		major = MAJOR(dev);
	}

	if (result < 0) {
<<<<<<< HEAD
		pr_err("Registering avtimer device failed\n");
		return result;
	}

	pavtimer->avtimer_class = class_create(THIS_MODULE, "avtimer");
	if (IS_ERR(pavtimer->avtimer_class)) {
		result = PTR_ERR(pavtimer->avtimer_class);
		pr_err("Error creating avtimer class: %d\n", result);
		goto unregister_chrdev_region;
	}
	pavtimer->avtimer_pdata = pdev->dev.platform_data;

	cdev_init(&pavtimer->myc, &avtimer_fops);
	result = cdev_add(&pavtimer->myc, dev, 1);

	if (result < 0) {
		pr_err("Registering file operations failed\n");
		goto class_destroy;
	}

	device_handle = device_create(pavtimer->avtimer_class,
			NULL, pavtimer->myc.dev, NULL, "avtimer");
	if (IS_ERR(device_handle)) {
		result = PTR_ERR(device_handle);
		pr_err("device_create failed: %d\n", result);
		goto class_destroy;
	}

	mutex_init(&pavtimer->avtimer_lock);
	core_handle = NULL;
	pavtimer->avtimer_open_cnt = 0;

	pr_debug("Device create done for avtimer major=%d\n", major);

	return 0;

class_destroy:
	class_destroy(pavtimer->avtimer_class);
unregister_chrdev_region:
	unregister_chrdev_region(MKDEV(major, 0), 1);
=======
		dev_err(&pdev->dev, "%s: Registering avtimer device failed\n",
			__func__);
		goto unmap;
	}

	avtimer.avtimer_class = class_create(THIS_MODULE, "avtimer");
	if (IS_ERR(avtimer.avtimer_class)) {
		result = PTR_ERR(avtimer.avtimer_class);
		dev_err(&pdev->dev, "%s: Error creating avtimer class: %d\n",
			__func__, result);
		goto unregister_chrdev_region;
	}

	cdev_init(&avtimer.myc, &avtimer_fops);
	result = cdev_add(&avtimer.myc, dev, 1);

	if (result < 0) {
		dev_err(&pdev->dev, "%s: Registering file operations failed\n",
			__func__);
		goto class_destroy;
	}

	device_handle = device_create(avtimer.avtimer_class,
			NULL, avtimer.myc.dev, NULL, "avtimer");
	if (IS_ERR(device_handle)) {
		result = PTR_ERR(device_handle);
		pr_err("%s: device_create failed: %d\n", __func__, result);
		goto class_destroy;
	}
	init_waitqueue_head(&avtimer.adsp_resp_wait);
	mutex_init(&avtimer.avtimer_lock);
	avtimer.avtimer_open_cnt = 0;

	pr_debug("%s: Device create done for avtimer major=%d\n",
			__func__, major);

	if (of_property_read_u32(pdev->dev.of_node,
			"qcom,clk_div", &clk_div_val))
		avtimer.clk_div = 1;
	else
		avtimer.clk_div = clk_div_val;

	pr_debug("avtimer.clk_div = %d\n", avtimer.clk_div);
	return 0;

class_destroy:
	class_destroy(avtimer.avtimer_class);
unregister_chrdev_region:
	unregister_chrdev_region(MKDEV(major, 0), 1);
unmap:
	if (avtimer.p_avtimer_lsw)
		devm_iounmap(&pdev->dev, avtimer.p_avtimer_lsw);
	if (avtimer.p_avtimer_msw)
		devm_iounmap(&pdev->dev, avtimer.p_avtimer_msw);
	avtimer.p_avtimer_lsw = NULL;
	avtimer.p_avtimer_msw = NULL;
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
	return result;

}

static int __devexit dev_avtimer_remove(struct platform_device *pdev)
{
<<<<<<< HEAD
	struct avtimer_t *pavtimer = &avtimer;

	pr_debug("dev_avtimer_remove\n");

	device_destroy(pavtimer->avtimer_class, pavtimer->myc.dev);
	cdev_del(&pavtimer->myc);
	class_destroy(pavtimer->avtimer_class);
=======
	pr_debug("%s: dev_avtimer_remove\n", __func__);

	if (avtimer.p_avtimer_lsw)
		devm_iounmap(&pdev->dev, avtimer.p_avtimer_lsw);
	if (avtimer.p_avtimer_msw)
		devm_iounmap(&pdev->dev, avtimer.p_avtimer_msw);
	device_destroy(avtimer.avtimer_class, avtimer.myc.dev);
	cdev_del(&avtimer.myc);
	class_destroy(avtimer.avtimer_class);
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
	unregister_chrdev_region(MKDEV(major, 0), 1);

	return 0;
}

<<<<<<< HEAD
static struct platform_driver dev_avtimer_driver = {
	.probe = dev_avtimer_probe,
	.remove = __exit_p(dev_avtimer_remove),
	.driver = {.name = "dev_avtimer"}
=======
static const struct of_device_id avtimer_machine_of_match[]  = {
	{ .compatible = "qcom,avtimer", },
	{},
};
static struct platform_driver dev_avtimer_driver = {
	.probe = dev_avtimer_probe,
	.remove = dev_avtimer_remove,
	.driver = {
		.name = "dev_avtimer",
		.of_match_table = avtimer_machine_of_match,
	},
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
};

static int  __init avtimer_init(void)
{
	s32 rc;
	rc = platform_driver_register(&dev_avtimer_driver);
	if (IS_ERR_VALUE(rc)) {
<<<<<<< HEAD
		pr_err("platform_driver_register failed.\n");
		goto error_platform_driver;
	}
	pr_debug("dev_avtimer_init : done\n");
=======
		pr_err("%s: platform_driver_register failed\n", __func__);
		goto error_platform_driver;
	}
	pr_debug("%s: dev_avtimer_init : done\n", __func__);
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0

	return 0;
error_platform_driver:

<<<<<<< HEAD
	pr_err("encounterd error\n");
	return -ENODEV;
=======
	pr_err("%s: encounterd error\n", __func__);
	return rc;
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
}

static void __exit avtimer_exit(void)
{
<<<<<<< HEAD
	pr_debug("avtimer_exit\n");
=======
	pr_debug("%s: avtimer_exit\n", __func__);
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
	platform_driver_unregister(&dev_avtimer_driver);
}

module_init(avtimer_init);
module_exit(avtimer_exit);

MODULE_DESCRIPTION("avtimer driver");
MODULE_LICENSE("GPL v2");
