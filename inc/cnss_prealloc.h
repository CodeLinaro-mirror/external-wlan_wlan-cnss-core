/* Copyright (c) 2015-2016,2020 The Linux Foundation. All rights reserved.
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

#ifndef _NET_CNSS_PREALLOC_H_
#define _NET_CNSS_PREALLOC_H_

#define WCNSS_PRE_ALLOC_GET_THRESHOLD (4*1024)

extern void *wcnss_prealloc_get(size_t size);
extern int wcnss_prealloc_put(void *ptr);
extern int wcnss_pre_alloc_reset(void);
void wcnss_prealloc_check_memory_leak(void);

#ifdef FEATURE_SKB_PRE_ALLOC
#define WCNSS_PRE_SKB_ALLOC_GET_THRESHOLD (16*1024)
extern struct sk_buff *wcnss_skb_prealloc_get(unsigned int size);
extern int wcnss_skb_prealloc_put(struct sk_buff *skb);
#endif

#ifdef CONFIG_WCNSS_DMA_PRE_ALLOC
int wcnss_dma_prealloc_put(size_t size,
			   void *vaddr, dma_addr_t dma_handle);
void *wcnss_dma_prealloc_get(size_t size, dma_addr_t *dma_handle);
void wcnss_dma_prealloc_save(struct device *dev, size_t size,
			   void *vaddr, dma_addr_t dma_handle);
#endif

#endif /* _NET_CNSS__PREALLOC_H_ */
