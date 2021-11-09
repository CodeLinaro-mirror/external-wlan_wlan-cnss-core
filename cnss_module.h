/* Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _CNSS_MODULE_H_
#define _CNSS_MODULE_H_

#ifdef CNSS_DISABLE_EXPORT_SYMBOL
#define cnss_export_symbol(sym)
#else
#define cnss_export_symbol(sym) EXPORT_SYMBOL(sym)
#endif

#endif
