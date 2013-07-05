/**
 * @file pmta_common.c
 * @brief Common functions and data structures — implementation
 * @date 29.09.2010
 * @author Vladimir Kolesnikov <vladimir@extrememember.com>
 */

#include "pmta_common.h"

#if ZEND_MODULE_API_NO > 20060613
const
#endif
zend_arg_info arginfo_empty[1] = {
	{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 }
};

#if ZEND_MODULE_API_NO > 20060613
const
#endif
zend_arg_info arginfo_get[2] = {
	{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
	ZEND_ARG_INFO(0, property)
};

#if ZEND_MODULE_API_NO > 20060613
const
#endif
zend_arg_info arginfo_set[3] = {
	{ NULL, 0, NULL, 0, 0, 0, 0, 0, 2 },
	ZEND_ARG_INFO(0, property)
	ZEND_ARG_INFO(0, value)
};

void generic_get(const struct props* properties, size_t n, zend_class_entry* scope, const char* class_name, INTERNAL_FUNCTION_PARAMETERS)
{
	char* property;
	int property_len;
	size_t i;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &property, &property_len)) {
		RETURN_NULL();
	}

	for (i=1; i<n; ++i) {
		if (property_len == properties[i].len && !strcmp(property, properties[i].name)) {
			zval* retval = zend_read_property(scope, getThis(), property, property_len, 0 TSRMLS_CC);
			RETURN_ZVAL(retval, 1, 0);
		}
	}

	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Undefined property %s::%s", class_name, property);
}

void generic_isset(const struct props* properties, size_t n, INTERNAL_FUNCTION_PARAMETERS)
{
	char* property;
	int property_len;
	size_t i;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &property, &property_len)) {
		RETURN_NULL();
	}

	for (i=1; i<n; ++i) {
		if (property_len == properties[i].len && !strcmp(property, properties[i].name)) {
			RETURN_TRUE;
		}
	}

	RETURN_FALSE;
}
