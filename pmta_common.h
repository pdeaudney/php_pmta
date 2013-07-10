/**
 * @file pmta_common.h
 * @brief Common functions and data structures — declarations
 * @date Sep 29, 2010 v0.1
 * @date Jul 4, 2013 Major refactoring
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

/**
 * @brief A do-nothing destructor
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 */
PHPPMTA_VISIBILITY_HIDDEN PHP_FUNCTION(empty_destructor);

#endif /* PMTA_COMMON_H_ */
