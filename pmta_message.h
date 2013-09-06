/**
 * @file pmta_message.h
 * @date Sep 29, 2010 v0.1
 * @date Jul 4, 2013 Major code refactoring, use object handlers instead of magic methods
 * @date Jul 11, 2013 v0.4
 * @author Vladimir Kolesnikov <vladimir@extrememember.com>
 * @brief Exposes @c PmtaMessage class
 * @details
@code{.php}
class PmtaMessage
{
	const RETURN_HEADERS = PmtaMsgRETURN_HEADERS;
	const RETURN_FULL    = PmtaMsgRETURN_FULL;

	const ENCODING_7BIT   = PmtaMsgENCODING_7BIT;
	const ENCODING_8BIT   = PmtaMsgENCODING_8BIT;
	const ENCODING_BASE64 = PmtaMsgENCODING_BASE64;

	private $message;

	private $originator;
	private $verp;
	private $return_type;
	private $envelope_id;
	private $vmta;
	private $jobid;
	private $encoding;
	private $recipients;

	public function __construct($originator);
	public function __destruct();
	public function __get($property);
	public function __isset($property);
	public function __set($property, $value);
	public function beginPart($number);
	public function addData($data);
	public function addMergeData($data);
	public function addDateHeader();
	public function addRecipient(PmtaRecipient $recipient);
	public function getLastError();
	private function __clone();
}
@endcode
 */

#ifdef DOXYGEN
#	undef PMTA_MESSAGE_H
#endif

#ifndef PMTA_MESSAGE_H
#define PMTA_MESSAGE_H

#include "php_pmta.h"
#include <submitter/PmtaMsg.h>

/**
 * @brief Extracts @c PmtaMsg from @c PmtaMessage object
 * @param object @c PmtaMessage object
 * @param tsrm_ls Internally used by Zend
 * @return @c PmtaMsg
 */
PHPPMTA_VISIBILITY_HIDDEN extern PmtaMsg getMessage(zval* object TSRMLS_DC);

/**
 * @brief Registers @c PmtaMessage class
 * @param tsrm_ls Internally used by Zend
 */
PHPPMTA_VISIBILITY_HIDDEN extern void pmtamsg_register_class(TSRMLS_D);

#endif /* PMTA_MESSAGE_H */
