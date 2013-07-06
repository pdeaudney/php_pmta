/**
 * @file php_pmta.h
 * @brief Common header file
 * @date 28.09.2010
 * @author Vladimir Kolesnikov <vladimir@extrememember.com>
 */

#ifdef DOXYGEN
#	include "macros.h"
#	undef ZEND_API
#	undef ZEND_DLEXPORT
#	define ZEND_API
#	define ZEND_DLEXPORT
#	undef PHP_PMTA_H_
#endif

#ifndef PHP_PMTA_H_
#define PHP_PMTA_H_

/**
 * @headerfile php_pmta.h
 * @brief Internal extension name
 */
#define PHP_PMTA_EXTNAME "PMTA API Wrapper"

/**
 * @headerfile php_pmta.h
 * @brief Extension version
 */
#define PHP_PMTA_EXTVER  "0.4"

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include <main/php.h>
#include <main/php_ini.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_interfaces.h>

/**
 * @headerfile php_pmta.h
 * @def PMTA_G(v)
 * @brief Provides thread safe acccess to the global @c v (stored in @c pmta_globals)
 */
#ifdef ZTS
#	include "TSRM.h"
#	define PMTA_G(v) TSRMG(pmta_globals_id, zend_pmta_globals*, v)
#else
#	define PMTA_G(v) (pmta_globals.v)
#endif

/**
 * @headerfile php_pmta.h
 * @def PHPPMTA_VISIBILITY_HIDDEN
 * @brief Prevents the name from being exported outside the shared module
 */
#ifdef DOXYGEN
#	undef PHPPMTA_VISIBILITY_HIDDEN
#	define PHPPMTA_VISIBILITY_HIDDEN
#elif __GNUC__ >= 4
#	define PHPPMTA_VISIBILITY_HIDDEN __attribute__((visibility("hidden")))
#else
#	define PHPPMTA_VISIBILITY_HIDDEN
#endif

/**
 * @headerfile php_pmta.h
 * @def PHPPMTA_STATIC
 *
 * Old versions of PHP declared @c ZEND_BEGIN_ARG_INFO_EX without @c static keyword.
 * New versions of PHP include @c static. This macro ensures that we won't have too many @c static in @c ZEND_BEGIN_ARG_INFO_EX
 */
#if ZEND_MODULE_API_NO <= 20060613
#	define PHPPMTA_STATIC static
#else
#	define PHPPMTA_STATIC
#endif

#if PHP_VERSION_ID > 50399
#	define ZLK_DC , const struct _zend_literal* key
#	define ZLK_CC , key
#else
#	define ZLK_DC
#	define ZLK_CC
#endif

#define ISSTR(pzv, str) ((Z_STRLEN_P(pzv) == strlen(str)) && !strcmp(Z_STRVAL_P(pzv), str))

/**
 * This structure is used internally to declare private class properties
 */
struct props {
	char* name;       /**< Property name */
	int len;          /**< Property name length */
	char* comment;    /**< PHPDoc comment */
	int comment_len;  /**< PHPDoc comment length */
};

PHPPMTA_VISIBILITY_HIDDEN extern zend_class_entry* pmta_conn_class;             /**< PmtaConnection class */
PHPPMTA_VISIBILITY_HIDDEN extern zend_class_entry* pmta_error_connection_class; /**< PmtaErrorCoonection class */
PHPPMTA_VISIBILITY_HIDDEN extern zend_class_entry* pmta_error_recipient_class;  /**< PmtaErrorMessage class */
PHPPMTA_VISIBILITY_HIDDEN extern zend_class_entry* pmta_error_message_class;    /**< PmtaErrorMessage class */
PHPPMTA_VISIBILITY_HIDDEN extern zend_class_entry* pmta_rcpt_class;             /**< PmtaRecipient class */
PHPPMTA_VISIBILITY_HIDDEN extern zend_class_entry* pmta_msg_class;              /**< PmtaMessage class */

/**
 * @headerfile php_pmta.h
 * @var pmta_module_entry
 * @brief Module entry
 */
#ifdef COMPILE_DL_PMTA
PHPPMTA_VISIBILITY_HIDDEN extern zend_module_entry pmta_module_entry;
#else
extern zend_module_entry pmta_module_entry;
#endif

/**
 * This one is required by php/main/internal_functions.c when php_pmta is built statically
 *
 * @headerfile php_pmta.h
 * @def phpext_pmta_ptr
 */
#define phpext_pmta_ptr &pmta_module_entry

/**
 * @headerfile php_pmta.h
 * @def PHPPMTA_SL
 * @warning @c x can be evaluated several times
 * @warning @c x is supposed to be a zero-terminated string
 * @brief @c x and its size minus one byte
 */
#define PHPPMTA_SL(x) x, sizeof(x)-1

/**
 * @brief This structure declares module global variables
 */
ZEND_BEGIN_MODULE_GLOBALS(pmta)
	zend_bool use_exceptions; /**< Whether to throw exceptions instead of returning error */
	char* server;             /**< Default server to use in PmtaConnection::__construct() */
	int port;                 /**< Default port to use in PmtaConnection::__construct() */
	char* username;           /**< Default username to use in PmtaConnection::__construct() */
	char* password;           /**< Default password to use in PmtaConnection::__construct() */
ZEND_END_MODULE_GLOBALS(pmta);

/**
 * @brief Module globals
 */
PHPPMTA_VISIBILITY_HIDDEN extern ZEND_DECLARE_MODULE_GLOBALS(pmta);

#endif /* PHP_PMTA_H_ */
