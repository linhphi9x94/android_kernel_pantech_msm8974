/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/slab.h>
#include <linux/list.h>
<<<<<<< HEAD
#include <linux/module.h>
#include <kgsl_device.h>

#include "kgsl_trace.h"

static inline struct list_head *_get_list_head(struct kgsl_device *device,
		struct kgsl_context *context)
{
	return (context) ? &context->events : &device->events;
}

static void _add_event_to_list(struct list_head *head, struct kgsl_event *event)
{
	struct list_head *n;

	for (n = head->next; n != head; n = n->next) {
		struct kgsl_event *e =
			list_entry(n, struct kgsl_event, list);

		if (timestamp_cmp(e->timestamp, event->timestamp) > 0) {
			list_add(&event->list, n->prev);
			break;
		}
	}

	if (n == head)
		list_add_tail(&event->list, head);
}

static inline void _do_signal_event(struct kgsl_device *device,
		struct kgsl_event *event, unsigned int timestamp,
		unsigned int type)
{
	int id = event->context ? event->context->id : KGSL_MEMSTORE_GLOBAL;

	trace_kgsl_fire_event(id, timestamp, type, jiffies - event->created,
		event->func);

	if (event->func)
		event->func(device, event->priv, id, timestamp, type);

	list_del(&event->list);
	kgsl_context_put(event->context);
	kfree(event);
}

static void _retire_events(struct kgsl_device *device,
		struct list_head *head, unsigned int timestamp)
{
	struct kgsl_event *event, *tmp;

	list_for_each_entry_safe(event, tmp, head, list) {
		if (timestamp_cmp(timestamp, event->timestamp) < 0)
			break;

		_do_signal_event(device, event, event->timestamp,
			KGSL_EVENT_TIMESTAMP_RETIRED);
	}
}

static struct kgsl_event *_find_event(struct kgsl_device *device,
		struct list_head *head, unsigned int timestamp,
		kgsl_event_func func, void *priv)
{
	struct kgsl_event *event, *tmp;

	list_for_each_entry_safe(event, tmp, head, list) {
		if (timestamp == event->timestamp && func == event->func &&
			event->priv == priv)
			return event;
	}

	return NULL;
}

/**
 * _signal_event() - send a signal to a specific event in the list
 * @device: Pointer to the KGSL device struct
 * @head: Pointer to the event list to process
 * @timestamp: timestamp of the event to signal
 * @cur: timestamp value to send to the callback
 * @type: Signal ID to send to the callback
 *
 * Send the specified signal to the events in the list with the specified
 * timestamp. The timestamp 'cur' is sent to the callback so it knows
 * when the signal was delivered
 */
static void _signal_event(struct kgsl_device *device,
		struct list_head *head, unsigned int timestamp,
		unsigned int cur, unsigned int type)
{
	struct kgsl_event *event, *tmp;

	list_for_each_entry_safe(event, tmp, head, list) {
		if (timestamp_cmp(timestamp, event->timestamp) == 0)
			_do_signal_event(device, event, cur, type);
	}
}

/**
 * _signal_events() - send a signal to all the events in a list
 * @device: Pointer to the KGSL device struct
 * @head: Pointer to the event list to process
 * @timestamp: Timestamp to pass to the events (this should be the current
 * timestamp when the signal is sent)
 * @type: Signal ID to send to the callback
 *
 * Send the specified signal to all the events in the list and destroy them
 */
static void _signal_events(struct kgsl_device *device,
		struct list_head *head, uint32_t timestamp,
		unsigned int type)
{
	struct kgsl_event *event, *tmp;

	list_for_each_entry_safe(event, tmp, head, list)
		_do_signal_event(device, event, timestamp, type);

}

/**
 * kgsl_signal_event() - send a signal to a specific event in the context
 * @device: Pointer to the KGSL device struct
 * @context: Pointer to the KGSL context
 * @timestamp: Timestamp of the event to signal
 * @type: Signal ID to send to the callback
 *
 * Send the specified signal to all the events in the context with the given
 * timestamp
 */
void kgsl_signal_event(struct kgsl_device *device,
		struct kgsl_context *context, unsigned int timestamp,
		unsigned int type)
{
	struct list_head *head = _get_list_head(device, context);
	uint32_t cur;

	BUG_ON(!mutex_is_locked(&device->mutex));

	cur = kgsl_readtimestamp(device, context, KGSL_TIMESTAMP_RETIRED);
	_signal_event(device, head, timestamp, cur, type);

	if (context && list_empty(&context->events))
		list_del_init(&context->events_list);
}
EXPORT_SYMBOL(kgsl_signal_event);

/**
 * kgsl_signal_events() - send a signal to all events in the context
 * @device: Pointer to the KGSL device struct
 * @context: Pointer to the KGSL context
 * @type: Signal ID to send to the callback function
 *
 * Send the specified signal to all the events in the context
 */
void kgsl_signal_events(struct kgsl_device *device,
		struct kgsl_context *context, unsigned int type)
{
	struct list_head *head = _get_list_head(device, context);
	uint32_t cur;

	BUG_ON(!mutex_is_locked(&device->mutex));

	/*
	 * Send the current timestamp to the callback so it knows when the
	 * signal occured
	 */

	cur = kgsl_readtimestamp(device, context, KGSL_TIMESTAMP_RETIRED);

	_signal_events(device, head, cur, type);

	/*
	 * Remove the context from the master list since we know everything on
	 * it has been removed
	 */

	if (context)
		list_del_init(&context->events_list);
}
EXPORT_SYMBOL(kgsl_signal_events);

/**
 * kgsl_add_event - Add a new timstamp event for the KGSL device
 * @device - KGSL device for the new event
 * @id - the context ID that the event should be added to
 * @ts - the timestamp to trigger the event on
 * @func - callback function to call when the timestamp expires
 * @priv - private data for the specific event type
 * @owner - driver instance that owns this event
 *
 * @returns - 0 on success or error code on failure
 */
int kgsl_add_event(struct kgsl_device *device, u32 id, u32 ts,
	kgsl_event_func func, void *priv, void *owner)
{
	struct kgsl_event *event;
	unsigned int queued, cur_ts;
	struct kgsl_context *context = NULL;

	BUG_ON(!mutex_is_locked(&device->mutex));

	if (func == NULL)
		return -EINVAL;

	if (id != KGSL_MEMSTORE_GLOBAL) {
		context = kgsl_context_get(device, id);
		if (context == NULL)
			return -EINVAL;
	}

	queued = kgsl_readtimestamp(device, context, KGSL_TIMESTAMP_QUEUED);

	if (timestamp_cmp(ts, queued) > 0) {
		kgsl_context_put(context);
		return -EINVAL;
	}

	cur_ts = kgsl_readtimestamp(device, context, KGSL_TIMESTAMP_RETIRED);

	/*
	 * Check to see if the requested timestamp has already fired.  If it
	 * did do the callback right away.  Make sure to send the timestamp that
	 * the event expected instead of the current timestamp because sometimes
	 * the event handlers can get confused.
	 */

	if (timestamp_cmp(cur_ts, ts) >= 0) {
		trace_kgsl_fire_event(id, cur_ts, ts, 0, func);

		func(device, priv, id, ts, KGSL_EVENT_TIMESTAMP_RETIRED);
		kgsl_context_put(context);
		return 0;
	}

	event = kzalloc(sizeof(*event), GFP_KERNEL);
	if (event == NULL) {
		kgsl_context_put(context);
		return -ENOMEM;
	}

	event->context = context;
	event->timestamp = ts;
	event->priv = priv;
	event->func = func;
	event->owner = owner;
	event->created = jiffies;

	trace_kgsl_register_event(id, ts, func);

	/* Add the event to either the owning context or the global list */

	if (context) {
		_add_event_to_list(&context->events, event);

		/*
		 * Add it to the master list of contexts with pending events if
		 * it isn't already there
		 */

		if (list_empty(&context->events_list))
			list_add_tail(&context->events_list,
				&device->events_pending_list);

	} else
		_add_event_to_list(&device->events, event);

	queue_work(device->work_queue, &device->ts_expired_ws);
=======
#include <linux/workqueue.h>
#include <kgsl_device.h>

#include "kgsl_trace.h"
#include "adreno.h"

/*
 * Define an kmem cache for the event structures since we allocate and free them
 * so frequently
 */
static struct kmem_cache *events_cache;

static inline void signal_event(struct kgsl_device *device,
		struct kgsl_event *event, int result)
{
	list_del(&event->node);
	event->result = result;
	queue_work(device->events_wq, &event->work);
}

/**
 * _kgsl_event_worker() - Work handler for processing GPU event callbacks
 * @work: Pointer to the work_struct for the event
 *
 * Each event callback has its own work struct and is run on a event specific
 * workqeuue.  This is the worker that queues up the event callback function.
 */
static void _kgsl_event_worker(struct work_struct *work)
{
	struct kgsl_event *event = container_of(work, struct kgsl_event, work);
	int id = KGSL_CONTEXT_ID(event->context);

	trace_kgsl_fire_event(id, event->timestamp, event->result, jiffies - event->created,
		event->func);

	if (event->func)
		event->func(event->device, event->context, event->priv, event->result);

	kgsl_context_put(event->context);
	kmem_cache_free(events_cache, event);
}

/**
 * kgsl_process_event_group() - Handle all the retired events in a group
 * @device: Pointer to a KGSL device
 * @group: Pointer to a GPU events group to process
 */
void kgsl_process_event_group(struct kgsl_device *device,
		struct kgsl_event_group *group)
{
	struct kgsl_event *event, *tmp;
	unsigned int timestamp;
	struct kgsl_context *context;

	if (group == NULL)
		return;

	context = group->context;

	_kgsl_context_get(context);

	spin_lock(&group->lock);

	timestamp = kgsl_readtimestamp(device, context, KGSL_TIMESTAMP_RETIRED);

	/*
	 * If no timestamps have been retired since the last time we were here
	 * then we can avoid going through this loop
	 */
	if (timestamp_cmp(timestamp, group->processed) <= 0)
		goto out;

	list_for_each_entry_safe(event, tmp, &group->events, node) {
		if (timestamp_cmp(event->timestamp, timestamp) <= 0)
			signal_event(device, event, KGSL_EVENT_RETIRED);
	}

	group->processed = timestamp;

out:
	spin_unlock(&group->lock);
	kgsl_context_put(context);
}
EXPORT_SYMBOL(kgsl_process_event_group);

/**
 * kgsl_cancel_events_timestamp() - Cancel pending events for a given timestamp
 * @device: Pointer to a KGSL device
 * @group: Ponter to the GPU event group that owns the event
 * @timestamp: Registered expiry timestamp for the event
 */
void kgsl_cancel_events_timestamp(struct kgsl_device *device,
		struct kgsl_event_group *group, unsigned int timestamp)
{
	struct kgsl_event *event, *tmp;

	spin_lock(&group->lock);

	list_for_each_entry_safe(event, tmp, &group->events, node) {
		if (timestamp_cmp(timestamp, event->timestamp) == 0)
			signal_event(device, event, KGSL_EVENT_CANCELLED);
	}

	spin_unlock(&group->lock);
}
EXPORT_SYMBOL(kgsl_cancel_events_timestamp);

/**
 * kgsl_cancel_events() - Cancel all pending events in the group
 * @device: Pointer to a KGSL device
 * @group: Pointer to a kgsl_events_group
 */
void kgsl_cancel_events(struct kgsl_device *device,
		struct kgsl_event_group *group)
{
	struct kgsl_event *event, *tmp;

	spin_lock(&group->lock);

	list_for_each_entry_safe(event, tmp, &group->events, node)
		signal_event(device, event, KGSL_EVENT_CANCELLED);

	spin_unlock(&group->lock);
}
EXPORT_SYMBOL(kgsl_cancel_events);

/**
 * kgsl_cancel_event() - Cancel a specific event from a group
 * @device: Pointer to a KGSL device
 * @group: Pointer to the group that contains the events
 * @timestamp: Registered expiry timestamp for the event
 * @func: Registered callback for the function
 * @priv: Registered priv data for the function
 */
void kgsl_cancel_event(struct kgsl_device *device,
		struct kgsl_event_group *group, unsigned int timestamp,
		kgsl_event_func func, void *priv)
{
	struct kgsl_event *event, *tmp;
	spin_lock(&group->lock);

	list_for_each_entry_safe(event, tmp, &group->events, node) {
		if (timestamp == event->timestamp && func == event->func &&
			event->priv == priv)
			signal_event(device, event, KGSL_EVENT_CANCELLED);
	}

	spin_unlock(&group->lock);
}
EXPORT_SYMBOL(kgsl_cancel_event);

/**
 * kgsl_add_event() - Add a new GPU event to a group
 * @device: Pointer to a KGSL device
 * @group: Pointer to the group to add the event to
 * @timestamp: Timestamp that the event will expire on
 * @func: Callback function for the event
 * @priv: Private data to send to the callback function
 */
int kgsl_add_event(struct kgsl_device *device, struct kgsl_event_group *group,
		unsigned int timestamp, kgsl_event_func func, void *priv)
{
	unsigned int queued, retired;
	struct kgsl_context *context = group->context;
	struct kgsl_event *event;

	if (!func)
		return -EINVAL;

	/*
	 * If the caller is creating their own timestamps, let them schedule
	 * events in the future. Otherwise only allow timestamps that have been
	 * queued.
	 */
	if (!context || !(context->flags & KGSL_CONTEXT_USER_GENERATED_TS)) {
		queued = kgsl_readtimestamp(device, context, KGSL_TIMESTAMP_QUEUED);
		if (timestamp_cmp(timestamp, queued) > 0)
			return -EINVAL;
	}

	event = kmem_cache_alloc(events_cache, GFP_KERNEL);
	if (event == NULL)
		return -ENOMEM;

	/* Get a reference to the context while the event is active */
	_kgsl_context_get(context);

	event->device = device;
	event->context = context;
	event->timestamp = timestamp;
	event->priv = priv;
	event->func = func;
	event->created = jiffies;

	INIT_WORK(&event->work, _kgsl_event_worker);

	trace_kgsl_register_event(KGSL_CONTEXT_ID(context), timestamp, func);

	spin_lock(&group->lock);

	/*
	 * Check to see if the requested timestamp has already retired.  If so,
	 * schedule the callback right away
	 */
	retired = kgsl_readtimestamp(device, context, KGSL_TIMESTAMP_RETIRED);

	if (timestamp_cmp(retired, timestamp) >= 0) {
		event->result = KGSL_EVENT_RETIRED;
		queue_work(device->events_wq, &event->work);
		spin_unlock(&group->lock);
		return 0;
	}

	/* Add the event to the group list */
	list_add_tail(&event->node, &group->events);

	spin_unlock(&group->lock);

>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
	return 0;
}
EXPORT_SYMBOL(kgsl_add_event);

<<<<<<< HEAD
/**
 * kgsl_cancel_events() - Cancel all global events owned by a process
 * @device: Pointer to the KGSL device struct
 * @owner: driver instance that owns the events to cancel
 *
 * Cancel all global events that match the owner pointer
 */
void kgsl_cancel_events(struct kgsl_device *device, void *owner)
{
	struct kgsl_event *event, *event_tmp;
	unsigned int cur;

	BUG_ON(!mutex_is_locked(&device->mutex));

	cur = kgsl_readtimestamp(device, NULL, KGSL_TIMESTAMP_RETIRED);

	list_for_each_entry_safe(event, event_tmp, &device->events, list) {
		if (event->owner != owner)
			continue;

		_do_signal_event(device, event, cur, KGSL_EVENT_CANCELLED);
	}
}
EXPORT_SYMBOL(kgsl_cancel_events);

/**
 * kgsl_cancel_event() - send a cancel signal to a specific event
 * @device: Pointer to the KGSL device struct
 * @context: Pointer to the KGSL context
 * @timestamp: Timestamp of the event to cancel
 * @func: Callback function of the event - this is used to match the actual
 * event
 * @priv: Private data for the callback function - this is used to match to the
 * actual event
 *
 * Send the a cancel signal to a specific event that matches all the parameters
 */

void kgsl_cancel_event(struct kgsl_device *device, struct kgsl_context *context,
		unsigned int timestamp, kgsl_event_func func,
		void *priv)
{
	struct kgsl_event *event;
	struct list_head *head;

	BUG_ON(!mutex_is_locked(&device->mutex));

	head = _get_list_head(device, context);

	event = _find_event(device, head, timestamp, func, priv);

	if (event) {
		unsigned int cur = kgsl_readtimestamp(device, context,
			KGSL_TIMESTAMP_RETIRED);

		_do_signal_event(device, event, cur, KGSL_EVENT_CANCELLED);
	}
}
EXPORT_SYMBOL(kgsl_cancel_event);

static inline int _mark_next_event(struct kgsl_device *device,
		struct list_head *head)
{
	struct kgsl_event *event;

	if (!list_empty(head)) {
		event = list_first_entry(head, struct kgsl_event, list);

		/*
		 * Next event will return 0 if the event was marked or 1 if the
		 * timestamp on the event has passed - return that up a layer
		 */

		if (device->ftbl->next_event)
			return device->ftbl->next_event(device, event);
	}

	return 0;
}

static int kgsl_process_context_events(struct kgsl_device *device,
		struct kgsl_context *context)
{
	while (1) {
		unsigned int timestamp = kgsl_readtimestamp(device, context,
			KGSL_TIMESTAMP_RETIRED);

		_retire_events(device, &context->events, timestamp);

		/*
		 * _mark_next event will return 1 as long as the next event
		 * timestamp has expired - this is to cope with an unavoidable
		 * race condition with the GPU that is still processing events.
		 */

		if (!_mark_next_event(device, &context->events))
			break;
	}

	/*
	 * Return 0 if the list is empty so the calling function can remove the
	 * context from the pending list
	 */

	return list_empty(&context->events) ? 0 : 1;
}

void kgsl_process_events(struct work_struct *work)
{
	struct kgsl_device *device = container_of(work, struct kgsl_device,
		ts_expired_ws);
	struct kgsl_context *context, *tmp;
	uint32_t timestamp;

	mutex_lock(&device->mutex);

	timestamp = kgsl_readtimestamp(device, NULL, KGSL_TIMESTAMP_RETIRED);
	_retire_events(device, &device->events, timestamp);
	_mark_next_event(device, &device->events);

	/* Now process all of the pending contexts */
	list_for_each_entry_safe(context, tmp, &device->events_pending_list,
		events_list) {

		/*
		 * Increment the refcount to make sure that the list_del_init
		 * is called with a valid context's list
		 */
		if (_kgsl_context_get(context)) {
			/*
			 * If kgsl_timestamp_expired_context returns 0 then it
			 * no longer has any pending events and can be removed
			 * from the list
			 */

			if (kgsl_process_context_events(device, context) == 0)
				list_del_init(&context->events_list);
			kgsl_context_put(context);
		}
	}

	mutex_unlock(&device->mutex);
}
EXPORT_SYMBOL(kgsl_process_events);
=======
static DEFINE_RWLOCK(group_lock);
static LIST_HEAD(group_list);

/**
 * kgsl_process_events() - Work queue for processing new timestamp events
 * @work: Pointer to a work_struct
 */
void kgsl_process_events(struct work_struct *work)
{
	struct kgsl_event_group *group;
	struct kgsl_device *device = container_of(work, struct kgsl_device,
		event_work);

	read_lock(&group_lock);
	list_for_each_entry(group, &group_list, group)
		kgsl_process_event_group(device, group);
	read_unlock(&group_lock);
}
EXPORT_SYMBOL(kgsl_process_events);

/**
 * kgsl_del_event_group() - Remove a GPU event group
 * @group: GPU event group to remove
 */
void kgsl_del_event_group(struct kgsl_event_group *group)
{
	/* Make sure that all the events have been deleted from the list */
	BUG_ON(!list_empty(&group->events));

	write_lock(&group_lock);
	list_del(&group->group);
	write_unlock(&group_lock);
}
EXPORT_SYMBOL(kgsl_del_event_group);

/**
 * kgsl_add_event_group() - Add a new GPU event group
 * group: Pointer to the new group to add to the list
 */
void kgsl_add_event_group(struct kgsl_event_group *group,
		struct kgsl_context *context)
{
	spin_lock_init(&group->lock);
	INIT_LIST_HEAD(&group->events);

	group->context = context;

	write_lock(&group_lock);
	list_add_tail(&group->group, &group_list);
	write_unlock(&group_lock);
}
EXPORT_SYMBOL(kgsl_add_event_group);

/**
 * kgsl_events_exit() - Destroy the event kmem cache on module exit
 */
void kgsl_events_exit(void)
{
	if (events_cache)
		kmem_cache_destroy(events_cache);
}

/**
 * kgsl_events_init() - Create the event kmem cache on module start
 */
void __init kgsl_events_init(void)
{
	events_cache = KMEM_CACHE(kgsl_event, 0);
}
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
