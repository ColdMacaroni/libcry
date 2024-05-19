#pragma once

#ifndef __LIB_CRY__
#define __LIB_CRY__
#include <stdlib.h>
// For communicating with runner
#include <mqueue.h>

/* A test function with a description */
#define test(name, description) test_desc(name, description) test_impl(name)
/* Just the description of a test */
#define test_desc(name, desc) char* _cry_test_$##name##$_desc() {return desc;}
/* Just the implementation of a test */
#define test_impl(name) void _cry_test_$##name##$_impl(mqd_t _cry__mqd)

/* Aborts if false */
#define cry_assert(expr) ({if(!(expr)) abort();})



#endif /* __LIB_CRY__ */
