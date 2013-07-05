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

#ifndef ZEND_FETCH_RESOURCE_NO_RETURN
#	define ZEND_FETCH_RESOURCE_NO_RETURN(rsrc, rsrc_type, passed_id, default_id, resource_type_name, resource_type) \
	(rsrc = (rsrc_type)zend_fetch_resource(passed_id TSRMLS_CC, default_id, resource_type_name, NULL, 1, resource_type))
#endif

/**
 * @brief Generic @c __get() implementation
 * @param properties Pointer to <code>struct props</code> containing the names of all private proiperties
 * @param n Number of elements in @c properties
 * @param scope Current class
 * @param class_name Class name (to be used in "Undefined property Class::property" message)
 * @param ht Internally used by Zend (number of parameters)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value of the function is used by the caller)
 * @param tsrm_ls Internally used by Zend
 * @note @c properties[0] is not used by the function as it represents an internally used resource about which the calling application should not know
 */
PHPPMTA_VISIBILITY_HIDDEN extern void generic_get(const struct props* properties, size_t n, zend_class_entry* scope, const char* class_name, INTERNAL_FUNCTION_PARAMETERS);

/**
 * @brief Generic @c __isset() implementation
 * @param properties Pointer to <code>struct props</code> containing the names of all private proiperties
 * @param n Number of elements in @c properties
 * @param ht Internally used by Zend (number of parameters)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value of the function is used by the caller)
 * @param tsrm_ls Internally used by Zend
 * @note @c properties[0] is not used by the function as it represents an internally used resource about which the calling application should not know
 */
PHPPMTA_VISIBILITY_HIDDEN extern void generic_isset(const struct props* properties, size_t n, INTERNAL_FUNCTION_PARAMETERS);

/**
 * @brief Empty arginfo — for @c __clone(), @c __destruct()
 */
PHPPMTA_VISIBILITY_HIDDEN extern
#if ZEND_MODULE_API_NO > 20060613
const
#endif
zend_arg_info arginfo_empty[1];

/**
 * @c arginfo for @c __get() and @c __isset()
 */
PHPPMTA_VISIBILITY_HIDDEN extern
#if ZEND_MODULE_API_NO > 20060613
const
#endif
zend_arg_info arginfo_get[2];

/**
 * @brief arginfo for @c __set()
 */
PHPPMTA_VISIBILITY_HIDDEN extern
#if ZEND_MODULE_API_NO > 20060613
const
#endif
zend_arg_info arginfo_set[3];

#endif /* PMTA_COMMON_H_ */
