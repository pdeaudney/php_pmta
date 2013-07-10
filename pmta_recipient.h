/**
 * @file pmta_recipient.h
 * @date Sep 29, 2010 v0.1
 * @date Jul 4, 2013 Major code refactoring, use object handlers instead of magic methods
 * @date Jul 11, 2013 v0.4
 * @author Vladimir Kolesnikov <vladimir@extrememember.com>
 * @brief Exposes PmtaRecipient class
 * @details
@code
class PmtaRecipient
{
	const NOTIFY_NEVER   = PmtaRcptNOTIFY_NEVER;
	const NOTIFY_SUCCESS = PmtaRcptNOTIFY_SUCCESS;
	const NOTIFY_FAILURE = PmtaRcptNOTIFY_FAILURE;
	const NOTIFY_DELAY   = PmtaRcptNOTIFY_DELAY;
	const NOTIFY_ALWAYS  = PmtaRcptNOTIFY_SUCCESS | PmtaRcptNOTIFY_FAILURE | PmtaRcptNOTIFY_DELAY;

	private $recipient;
	private $locked = false;

	private $address;
	private $notify    = self::NOTIFY_NEVER;
	private $variables = array();

	public function __construct($address);
	public function __destruct();
	public function __get($property);
	public function __isset($property);
	public function __set($name, $value);
	public function defineVariable($name, $value);
	public function getLastError();
	private function __clone();
}
@endcode
 */

#ifdef DOXYGEN
#	undef PMTA_RECIPIENT_H_
#endif

#ifndef PMTA_RECIPIENT_H_
#define PMTA_RECIPIENT_H_

#include "php_pmta.h"
#include <submitter/PmtaRcpt.h>

/**
 * @brief Extracts @c PmtaRcpt from @c PmtaRecipient object
 * @param object @c PmtaRecipient object
 * @param tsrm_ls Internally used by Zend
 * @return @c PmtaRcpt
 */
PHPPMTA_VISIBILITY_HIDDEN extern PmtaRcpt getRecipient(zval* object TSRMLS_DC);

/**
 * @brief Locks @c PmtaRecipient object (when Recipient is added to the Message, Recipient must not be modified)
 * @param object @c PmtaRecipient object
 * @param tsrm_ls Internally used by Zend
 */
PHPPMTA_VISIBILITY_HIDDEN extern void lock_recipient(zval* object TSRMLS_DC);

/**
 * @brief Registers @c PmtaRecipient class
 * @param tsrm_ls Internally used by Zend
 */
PHPPMTA_VISIBILITY_HIDDEN extern void pmtarcpt_register_class(TSRMLS_D);

#endif /* PMTA_RECIPIENT_H_ */
