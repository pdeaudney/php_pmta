/**
 * @file pmta_error.h
 * @date 29.09.2010
 * @author Vladimir Kolesnikov <vladimir@extrememember.com>
 * @brief Exposes @c PmtaError, @c PmtaErrorConnection, @c PmtaErrorRecipient and @c PmtaErrorMessage classes
 * @note If @c HAVE_SPL macro evaluates to non-zero, @c PmtaError class is derived from @c RuntimeException, not from @c Exception
 * @details
@code{.php}
class PmtaError extends Exception
{
	const OUT_OF_MEMORY    = PmtaApiERROR_OutOfMemory;
	const ILLEGAL_STATE    = PmtaApiERROR_IllegalState;
	const ILLEGAL_ARGUMENT = PmtaApiERROR_IllegalArgument;
	const SECURITY         = PmtaApiERROR_Security;
	const IO               = PmtaApiERROR_IO;
	const SERVICE          = PmtaApiERROR_Service;
	const EMAIL_ADDRESS    = PmtaApiERROR_EmailAddress;
	const PHP_API          = PmtaApiERROR_PHP_API;
}

final class PmtaErrorConnection extends PmtaError { }
final class PmtaErrorRecipient  extends PmtaError { }
final class PmtaErrorMessage    extends PmtaError { }
@endcode
 */

#ifdef DOXYGEN
#	undef PMTA_ERROR_H_
#endif

#ifndef PMTA_ERROR_H_
#define PMTA_ERROR_H_

#include "php_pmta.h"

/**
 * @brief PHP API error
 */
#define PmtaApiERROR_PHP_API 255

/**
 * @brief registers PmtaError and derived classes
 * @param tsrm_ls Internally used by Zend
 */
PHPPMTA_VISIBILITY_HIDDEN extern void pmtaerror_register_class(TSRMLS_D);

/**
 * @brief Instantiates PmtaError or derived from it class
 * @param error_class Class to instantiate
 * @param code Error code (typically the return value of @c PmtaXXXGetLastErrorType())
 * @param message Error message (typically the return value of @c PmtaXXXGetLastError())
 * @param result If @c NULL, throws the exception with the instantiated class; otherwise, this is where the result is placed
 * @param tsrm_ls Internally used by Zend
 */
PHPPMTA_VISIBILITY_HIDDEN extern void throw_pmta_error(zend_class_entry* error_class, int code, const char* message, zval** result TSRMLS_DC);

#endif /* PMTA_ERROR_H_ */
