/* libcry - A C testing framework that doesn' make you want to cry
 * Copyright (C) 2024  babalark <babalark@proton.me>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#pragma once

#ifndef __LIB_CRY__
#define __LIB_CRY__
#include <stdlib.h>
// For communicating with runner
#include <mqueue.h>

#warning "Make macros work with c++"

/* A test function with a description */
#define test(name, description) test_desc(name, description) test_impl(name)
/* Just the description of a test */
#define test_desc(name, desc) char* _cry_test_$##name##$_desc() {return desc;}
/* Just the implementation of a test */
#define test_impl(name) void _cry_test_$##name##$_impl(mqd_t _cry__mqd)

/* Aborts if false */
#define cry_assert(expr) ({if(!(expr)) abort();})

#endif /* __LIB_CRY__ */
