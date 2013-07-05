/**
 * @file pmta_common.h
 * @brief Common functions and data structures — declarations
 * @date 29.09.2010
 * @author Vladimir Kolesnikov <vladimir@extrememember.com>
 */

#ifdef DOXYGEN
#	undef PMTA_COMMON_H_
#endif

#ifndef PMTA_COMMON_H_
#define PMTA_COMMON_H_

#include "php_pmta.h"

/**
 * @brief Empty arginfo — for @c __clone(), @c __destruct()
 */
ZEND_BEGIN_ARG_INFO_EX(arginfo_empty, 0, 0, 0)
ZEND_END_ARG_INFO()

/**
 * @c arginfo for @c __get() and @c __isset()
 */
ZEND_BEGIN_ARG_INFO_EX(arginfo_get, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

/**
 * @brief arginfo for @c __set()
 */
ZEND_BEGIN_ARG_INFO_EX(arginfo_set, 0, 0, 2)
	ZEND_ARG_INFO(0, property)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

#endif /* PMTA_COMMON_H_ */
