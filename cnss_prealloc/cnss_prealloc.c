/* Copyright (c) 2012,2014-2017,2019-2020 The Linux Foundation. All rights reserved.
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
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/err.h>
#include <linux/stacktrace.h>
#include <linux/spinlock.h>
#include <linux/debugfs.h>
#ifdef CONFIG_WCNSS_SKB_PRE_ALLOC
#include <linux/skbuff.h>
#endif
#include "cnss_prealloc.h"
#include <linux/version.h>
#include "cnss_module.h"

static DEFINE_SPINLOCK(alloc_lock);

#ifdef CONFIG_SLUB_DEBUG
#define WCNSS_MAX_STACK_TRACE			64
#endif

#define PRE_ALLOC_DEBUGFS_DIR		PRE_ALLOC_DEBUGFS_DIR_NAME
#define PRE_ALLOC_DEBUGFS_FILE_OBJ	"status"

static struct dentry *debug_base;

struct wcnss_prealloc {
	int occupied;
	size_t size;
	void *ptr;
#ifdef CONFIG_SLUB_DEBUG
	unsigned long stack_trace[WCNSS_MAX_STACK_TRACE];
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
	unsigned int nr_entries;
#else
	struct stack_trace trace;
#endif
#endif
};

/* pre-alloced mem for WLAN driver */

#ifdef CONFIG_CNSS_QCA6390
/* This table should be paired with WLAN chip attached ideally.
 * For QCA6390, roughly 800KB pre-allocation required.
 *
 * Be noted, currently not all QCA6390 attachment defined CNSS_QCA6390,
 * like on which single-DT feature enabled.
 */
static struct wcnss_prealloc wcnss_allocs[] = {
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 64 * 1024, NULL},
	{0, 64 * 1024, NULL},
	{0, 64 * 1024, NULL},
	{0, 64 * 1024, NULL},
};
#else
static struct wcnss_prealloc wcnss_allocs[] = {
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 8  * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 16 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 32 * 1024, NULL},
	{0, 64 * 1024, NULL},
	{0, 64 * 1024, NULL},
	{0, 64 * 1024, NULL},
	{0, 64 * 1024, NULL},
	{0, 128 * 1024, NULL},
	{0, 128 * 1024, NULL},
};
#endif

#if defined(CONFIG_WCNSS_SKB_PRE_ALLOC)
static struct wcnss_prealloc wcnss_skb_allocs[] = {
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 20 * 1024, NULL},
	{0, 64 * 1024, NULL},
	{0, 64 * 1024, NULL},
	{0, 128 * 1024, NULL},
	{0, 128 * 1024, NULL},
};
#endif

int wcnss_prealloc_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(wcnss_allocs); i++) {
		wcnss_allocs[i].occupied = 0;
		wcnss_allocs[i].ptr = kmalloc(wcnss_allocs[i].size, GFP_KERNEL);
		if (!wcnss_allocs[i].ptr)
			return -ENOMEM;
	}

#if defined(CONFIG_WCNSS_SKB_PRE_ALLOC)
	for (i = 0; i < ARRAY_SIZE(wcnss_skb_allocs); i++) {
		wcnss_skb_allocs[i].occupied = 0;
		wcnss_skb_allocs[i].ptr =
				dev_alloc_skb(wcnss_skb_allocs[i].size);
		if (wcnss_skb_allocs[i].ptr == NULL)
			return -ENOMEM;
	}
#endif

	return 0;
}

void wcnss_prealloc_deinit(void)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(wcnss_allocs); i++) {
		kfree(wcnss_allocs[i].ptr);
		wcnss_allocs[i].ptr = NULL;
	}

#if defined(CONFIG_WCNSS_SKB_PRE_ALLOC)
	for (i = 0; i < ARRAY_SIZE(wcnss_skb_allocs); i++) {
		wcnss_skb_allocs[i].occupied = 0;
		if (wcnss_skb_allocs[i].ptr != NULL){
			dev_kfree_skb(wcnss_skb_allocs[i].ptr);
			wcnss_skb_allocs[i].ptr = NULL;
		}
	}
#endif
}

#ifdef CONFIG_SLUB_DEBUG
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
static void wcnss_prealloc_save_stack_trace(struct wcnss_prealloc *entry)
{
	memset(&entry->stack_trace, 0, sizeof(entry->stack_trace));

	entry->nr_entries = stack_trace_save(entry->stack_trace, WCNSS_MAX_STACK_TRACE, 2);
}
#else
static void wcnss_prealloc_save_stack_trace(struct wcnss_prealloc *entry)
{
	struct stack_trace *trace = &entry->trace;

	memset(&entry->stack_trace, 0, sizeof(entry->stack_trace));
	trace->nr_entries = 0;
	trace->max_entries = WCNSS_MAX_STACK_TRACE;
	trace->entries = entry->stack_trace;
	trace->skip = 2;

	save_stack_trace(trace);
}
#endif
#else
static inline
void wcnss_prealloc_save_stack_trace(struct wcnss_prealloc *entry) {}
#endif

void *wcnss_prealloc_get(size_t size)
{
	int i = 0;
	unsigned long flags;

	spin_lock_irqsave(&alloc_lock, flags);
	for (i = 0; i < ARRAY_SIZE(wcnss_allocs); i++) {
		if (wcnss_allocs[i].occupied)
			continue;

		if (wcnss_allocs[i].size >= size) {
			/* we found the slot */
			wcnss_allocs[i].occupied = 1;
			spin_unlock_irqrestore(&alloc_lock, flags);
			wcnss_prealloc_save_stack_trace(&wcnss_allocs[i]);
			return wcnss_allocs[i].ptr;
		}
	}
	spin_unlock_irqrestore(&alloc_lock, flags);

	return NULL;
}
cnss_export_symbol(wcnss_prealloc_get);

int wcnss_prealloc_put(void *ptr)
{
	int i = 0;
	unsigned long flags;

	spin_lock_irqsave(&alloc_lock, flags);
	for (i = 0; i < ARRAY_SIZE(wcnss_allocs); i++) {
		if (wcnss_allocs[i].ptr == ptr) {
			wcnss_allocs[i].occupied = 0;
			spin_unlock_irqrestore(&alloc_lock, flags);
			return 1;
		}
	}
	spin_unlock_irqrestore(&alloc_lock, flags);

	return 0;
}
cnss_export_symbol(wcnss_prealloc_put);

#ifdef CONFIG_SLUB_DEBUG
void wcnss_prealloc_check_memory_leak(void)
{
	int i, j = 0;

	for (i = 0; i < ARRAY_SIZE(wcnss_allocs); i++) {
		if (!wcnss_allocs[i].occupied)
			continue;

		if (j == 0) {
			pr_err("wcnss_prealloc: Memory leak detected\n");
			j++;
		}

		pr_err("Size: %zu, addr: %pK, backtrace:\n",
		       wcnss_allocs[i].size, wcnss_allocs[i].ptr);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
		stack_trace_print(wcnss_allocs[i].stack_trace, wcnss_allocs[i].nr_entries, 1);
#else
		print_stack_trace(&wcnss_allocs[i].trace, 1);
#endif
	}
}
#else
void wcnss_prealloc_check_memory_leak(void) {}
#endif
cnss_export_symbol(wcnss_prealloc_check_memory_leak);

int wcnss_pre_alloc_reset(void)
{
	int i, n = 0;

	for (i = 0; i < ARRAY_SIZE(wcnss_allocs); i++) {
		if (!wcnss_allocs[i].occupied)
			continue;

		wcnss_allocs[i].occupied = 0;
		n++;
	}

#if defined(CONFIG_WCNSS_SKB_PRE_ALLOC)
	for (i = 0; i < ARRAY_SIZE(wcnss_skb_allocs); i++) {
		wcnss_skb_allocs[i].occupied = 0;
	}
#endif

	return n;
}
cnss_export_symbol(wcnss_pre_alloc_reset);

#if defined(CONFIG_WCNSS_SKB_PRE_ALLOC)
struct sk_buff *wcnss_skb_prealloc_get(unsigned int size)
{
	int i = 0;
	unsigned long flags;

	spin_lock_irqsave(&alloc_lock, flags);
	for (i = 0; i < ARRAY_SIZE(wcnss_skb_allocs); i++) {
		if (wcnss_skb_allocs[i].occupied)
			continue;

		if (wcnss_skb_allocs[i].size > size) {
			/* we found the slot */
			wcnss_skb_allocs[i].occupied = 1;
			spin_unlock_irqrestore(&alloc_lock, flags);
			return wcnss_skb_allocs[i].ptr;
		}
	}
	spin_unlock_irqrestore(&alloc_lock, flags);

	pr_err("wcnss_pre: %s: prealloc not available for size: %d\n",
	       __func__, size);

	return NULL;
}
cnss_export_symbol(wcnss_skb_prealloc_get);

int wcnss_skb_prealloc_put(struct sk_buff *skb)
{
	int i = 0;
	unsigned long flags;

	spin_lock_irqsave(&alloc_lock, flags);
	for (i = 0; i < ARRAY_SIZE(wcnss_skb_allocs); i++) {
		if (wcnss_skb_allocs[i].ptr == skb) {
			wcnss_skb_allocs[i].occupied = 0;
			spin_unlock_irqrestore(&alloc_lock, flags);
			return 1;
		}
	}
	spin_unlock_irqrestore(&alloc_lock, flags);

	return 0;
}
cnss_export_symbol(wcnss_skb_prealloc_put);

static int prealloc_skb_memory_stats_show(struct seq_file *fp, void *data)
{
	int i = 0;
	int used_slots = 0, free_slots = 0;
	unsigned int tsize = 0, tused = 0, size = 0;

	seq_puts(fp, "\nSKB_Slot_Size(Kb)\t[Used : Free]\n");
	for (i = 0; i < ARRAY_SIZE(wcnss_skb_allocs); i++) {
		tsize += wcnss_skb_allocs[i].size;
		if (size != wcnss_skb_allocs[i].size) {
			if (size) {
				seq_printf(
					fp, "[%d : %d]\n",
					used_slots, free_slots);
			}

			size = wcnss_skb_allocs[i].size;
			used_slots = 0;
			free_slots = 0;
			seq_printf(fp, "%d Kb\t\t\t", size / 1024);
		}

		if (wcnss_skb_allocs[i].occupied) {
			tused += wcnss_skb_allocs[i].size;
			++used_slots;
		} else {
			++free_slots;
		}
	}
	seq_printf(fp, "[%d : %d]\n", used_slots, free_slots);

	/* Convert byte to Kb */
	if (tsize)
		tsize = tsize / 1024;
	if (tused)
		tused = tused / 1024;
	seq_printf(fp, "\nSKB Memory Status:\nTotal Memory: %dKb\n", tsize);
	seq_printf(fp, "Used: %dKb\nFree: %dKb\n", tused, tsize - tused);

	return 0;
}
#endif

static int prealloc_memory_stats_show(struct seq_file *fp, void *data)
{
	int i = 0;
	int used_slots = 0, free_slots = 0;
	unsigned int tsize = 0, tused = 0, size = 0;

	seq_puts(fp, "\nSlot_Size(Kb)\t\t[Used : Free]\n");
	for (i = 0; i < ARRAY_SIZE(wcnss_allocs); i++) {
		tsize += wcnss_allocs[i].size;
		if (size != wcnss_allocs[i].size) {
			if (size) {
				seq_printf(
					fp, "[%d : %d]\n",
					used_slots, free_slots);
			}

			size = wcnss_allocs[i].size;
			used_slots = 0;
			free_slots = 0;
			seq_printf(fp, "%d Kb\t\t\t", size / 1024);
		}

		if (wcnss_allocs[i].occupied) {
			tused += wcnss_allocs[i].size;
			++used_slots;
		} else {
			++free_slots;
		}
	}
	seq_printf(fp, "[%d : %d]\n", used_slots, free_slots);

	/* Convert byte to Kb */
	if (tsize)
		tsize = tsize / 1024;
	if (tused)
		tused = tused / 1024;
	seq_printf(fp, "\nMemory Status:\nTotal Memory: %dKb\n", tsize);
	seq_printf(fp, "Used: %dKb\nFree: %dKb\n", tused, tsize - tused);

#ifdef CONFIG_WCNSS_SKB_PRE_ALLOC
	prealloc_skb_memory_stats_show(fp, data);
#endif
	return 0;
}

static int prealloc_memory_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, prealloc_memory_stats_show, NULL);
}

static const struct file_operations prealloc_memory_stats_fops = {
	.owner = THIS_MODULE,
	.open = prealloc_memory_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

#ifdef CONFIG_WLAN_CNSS_CORE
int wcnss_pre_alloc_init(void)
#else
static int __init wcnss_pre_alloc_init(void)
#endif
{
	int ret;

	ret = wcnss_prealloc_init();
	if (ret) {
		pr_err("%s: Failed to init the prealloc pool\n", __func__);
		return ret;
	}

	debug_base = debugfs_create_dir(PRE_ALLOC_DEBUGFS_DIR, NULL);
	if (IS_ERR_OR_NULL(debug_base)) {
		pr_err("%s: Failed to create debugfs dir\n", __func__);
	} else if (IS_ERR_OR_NULL(debugfs_create_file(
			PRE_ALLOC_DEBUGFS_FILE_OBJ,
			0644, debug_base, NULL,
			&prealloc_memory_stats_fops))) {
		pr_err("%s: Failed to create debugfs file\n", __func__);
		debugfs_remove_recursive(debug_base);
	}

	return ret;
}

#ifdef CONFIG_WLAN_CNSS_CORE
void wcnss_pre_alloc_exit(void)
#else
static void __exit wcnss_pre_alloc_exit(void)
#endif
{
	wcnss_prealloc_deinit();
	debugfs_remove_recursive(debug_base);
}

#ifndef CONFIG_WLAN_CNSS_CORE
module_init(wcnss_pre_alloc_init);
module_exit(wcnss_pre_alloc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("WCNSS Prealloc Driver");
#endif
