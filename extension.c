/**
 * @file extension.c
 * @brief Extension related functions
 * @date 28.09.2010
 * @author Vladimir Kolesnikov <vladimir@extrememember.com>
 */

#include "php_pmta.h"
#include <ext/standard/info.h>

#include "pmta_connection.h"
#include "pmta_error.h"
#include "pmta_message.h"
#include "pmta_recipient.h"

#if !defined(ZEND_ENGINE_2) && !defined(OnUpdateLong)
	/**
	 * @note Zend Engine 1 had OnUpdateInt instead of OnUpdateLong
	 */
#	define OnUpdateLong OnUpdateInt
#endif

ZEND_DECLARE_MODULE_GLOBALS(pmta);

/**
 * @brief INI Entries
 *
 * <TABLE>
 * <TR><TH>Name</TH><TH>Default</TH><TH>Changeable</TH><TH>Description</TH></TR>
 * <TR><TH>@c pmta.always_throw_exceptions</TH><TD>@c 0</TD><TD>@c PHP_INI_ALL</TD><TD>Throw an exception when the operation resulted in failure</TD></TR>
 * <TR><TH>@c pmta.server</TH><TD>@c null</TD><TD>@c PHP_INI_ALL</TD><TD>Default server to use in @c PmtaConnection::__construct()</TD></TR>
 * <TR><TH>@c pmta.port</TH><TD>@c 25</TD><TD>@c PHP_INI_ALL</TD><TD>Default port to use in @c PmtaConnection::__construct()</TD></TR>
 * <TR><TH>@c pmta.username</TH><TD>@c null</TD><TD>@c PHP_INI_ALL</TD><TD>Default username to use in @c PmtaConnection::__construct()</TD></TR>
 * <TR><TH>@c pmta.password</TH><TD>@c null</TD><TD>@c PHP_INI_ALL</TD><TD>Default password to use in @c PmtaConnection::__construct()</TD></TR>
 * </TABLE>
 */
PHP_INI_BEGIN()
	STD_PHP_INI_BOOLEAN("pmta.always_throw_exceptions", "0", PHP_INI_ALL, OnUpdateBool, use_exceptions, zend_pmta_globals, pmta_globals)
	STD_PHP_INI_ENTRY("pmta.server",   NULL, PHP_INI_ALL, OnUpdateString, server,   zend_pmta_globals, pmta_globals)
	STD_PHP_INI_ENTRY("pmta.port",     "25", PHP_INI_ALL, OnUpdateLong,   port,     zend_pmta_globals, pmta_globals)
	STD_PHP_INI_ENTRY("pmta.username", NULL, PHP_INI_ALL, OnUpdateString, username, zend_pmta_globals, pmta_globals)
	STD_PHP_INI_ENTRY("pmta.password", NULL, PHP_INI_ALL, OnUpdateString, password, zend_pmta_globals, pmta_globals)
PHP_INI_END()

int le_pmta_connection;
int le_pmta_message;
int le_pmta_recipient;

zend_class_entry* pmta_error_connection_class;
zend_class_entry* pmta_error_recipient_class;
zend_class_entry* pmta_error_message_class;
zend_class_entry* pmta_conn_class;
zend_class_entry* pmta_rcpt_class;
zend_class_entry* pmta_msg_class;

#ifndef PHP_GINIT
/**
 * @brief Globals constructor
 * @param pmta_globals Pointer to the PMTA globals
 * @param tsrm_ls
 */
static void pmta_globals_ctor(zend_pmta_globals* pmta_globals TSRMLS_DC)
#else
/**
 * @brief Globals constructor
 * @param pmta_globals Pointer to the PMTA globals
 * @param tsrm_ls
 */
static PHP_GINIT_FUNCTION(pmta)
#endif
{
	pmta_globals->use_exceptions = 0;
	pmta_globals->server         = NULL;
	pmta_globals->username       = NULL;
	pmta_globals->password       = NULL;
}

/**
 * @brief Module initialization function
 * @param type Module type
 * @param module_number Module number
 * @param tsrm_ls
 * @return Whether initialization succeeded
 * @retval SUCCESS Yes
 * @retval FAILURE No
 */
static PHP_MINIT_FUNCTION(pmta)
{
#ifndef PHP_GINIT
#ifdef ZTS
	ts_allocate_id(&pmta_globals_id, sizeof(zend_pmta_globals), (ts_allocate_ctor)pmta_globals_ctor, NULL);
#else
	pmta_globals_ctor(&pmta_globals TSRMLS_CC);
#endif /* ZTS */
#endif /* PHP_GINIT */

	REGISTER_INI_ENTRIES();

	le_pmta_connection = zend_register_list_destructors_ex(pmta_connection_dtor, NULL, PMTA_CONNECTION_RES_NAME, module_number);
	le_pmta_message    = zend_register_list_destructors_ex(pmta_message_dtor,    NULL, PMTA_MESSAGE_RES_NAME,    module_number);
	le_pmta_recipient  = zend_register_list_destructors_ex(pmta_recipient_dtor,  NULL, PMTA_RECIPIENT_RES_NAME,  module_number);

	pmtaconn_register_class(TSRMLS_C);
	pmtaerror_register_class(TSRMLS_C);
	pmtarcpt_register_class(TSRMLS_C);
	pmtamsg_register_class(TSRMLS_C);

	return SUCCESS;
}

/**
 * @brief Module shutdown function
 * @param type Module type
 * @param module_number Module number
 * @param tsrm_ls
 */
static PHP_MSHUTDOWN_FUNCTION(pmta)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

/**
 * @brief Module initialization function
 * @param zend_module Pointer to the module entry
 * @param tsrm_ls
 */
static PHP_MINFO_FUNCTION(pmta)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "PHP Submission API for PowerMTA", "enabled");
	php_info_print_table_row(2, "Version", PHP_PMTA_EXTVER);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}

/**
 * @brief Module dependencies
 */
static
#if ZEND_MODULE_API_NO > 20060613
const
#endif
zend_module_dep pmta_deps[] = {
#if defined(HAVE_SPL) && ((PHP_MAJOR_VERSION > 5) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1))
	ZEND_MOD_REQUIRED("spl")
#endif
	{ NULL, NULL, NULL, 0 }
};

zend_module_entry pmta_module_entry = {
#if ZEND_MODULE_API_NO >= 20050922
	STANDARD_MODULE_HEADER_EX,
	ini_entries,
	pmta_deps,
#elif ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	PHP_PMTA_EXTNAME,
	NULL,
	PHP_MINIT(pmta),
	PHP_MSHUTDOWN(pmta),
	NULL,
	NULL,
	PHP_MINFO(pmta),
#if ZEND_MODULE_API_NO > 20010901
	PHP_PMTA_EXTVER,
#endif
#if ZEND_MODULE_API_NO > 20050922
	PHP_MODULE_GLOBALS(pmta),
#	ifdef PHP_GINIT
	PHP_GINIT(pmta),
	NULL,
#	endif
#endif
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_PMTA
/**
 * @brief Return a pointer to the module entry
 * @return Pointer to @c pmta_module_entry
 * @see pmta_module_entry
 */
ZEND_GET_MODULE(pmta)
#endif
