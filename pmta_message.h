/**
 * @file pmta_message.h
 * @date 29.09.2010
 * @author Vladimir Kolesnikov <vladimir@extrememember.com>
 * @brief Exposes @c PmtaMessage class
 * @details
 * <PRE>
 * final class PmtaMessage
 * {
 * 	const RETURN_HEADERS = PmtaMsgRETURN_HEADERS;
 * 	const RETURN_FULL    = PmtaMsgRETURN_FULL;
 *
 * 	const ENCODING_7BIT   = PmtaMsgENCODING_7BIT;
 * 	const ENCODING_8BIT   = PmtaMsgENCODING_8BIT;
 * 	const ENCODING_BASE64 = PmtaMsgENCODING_BASE64;
 *
 * 	private $message;
 *
 * 	private $originator;
 * 	private $verp;
 * 	private $return_type;
 * 	private $envelope_id;
 * 	private $vmta;
 * 	private $jobid;
 * 	private $encoding;
 * 	private $recipients;
 *
 * 	public function __construct($originator);
 * 	public function __destruct();
 * 	public function __get($property);
 * 	public function __isset($property);
 * 	public function __set($property, $value);
 * 	public function beginPart($number);
 * 	public function addData($data);
 * 	public function addMergeData($data);
 * 	public function addDateHeader();
 * 	public function addRecipient(PmtaRecipient $recipient);
 * 	public function getLastError();
 * 	private function __clone();
 * }
 * </PRE>
 */

#ifdef DOXYGEN
#	undef PMTA_MESSAGE_H_
#endif

#ifndef PMTA_MESSAGE_H_
#define PMTA_MESSAGE_H_

#include "php_pmta.h"
#include <submitter/PmtaMsg.h>

/**
 * @brief Extracts @c PmtaMsg from @c PmtaMessage object
 * @param object PmtaMessage object
 * @param tsrm_ls Internally used by Zend
 * @return @c PmtaMsg
 */
PHPPMTA_VISIBILITY_HIDDEN extern PmtaMsg getMessage(zval* object TSRMLS_DC);

/**
 * @brief PMTA Message (PmtaMsg) resource destructor
 * @param res Resource
 * @param tsrm_ls Internally used by Zend
 */
PHPPMTA_VISIBILITY_HIDDEN extern void pmta_message_dtor(zend_rsrc_list_entry* res TSRMLS_DC);

/**
 * @brief Registers PmtaMessage class
 * @param tsrm_ls Internally used by Zend
 */
PHPPMTA_VISIBILITY_HIDDEN extern void pmtamsg_register_class(TSRMLS_D);

#endif /* PMTA_MESSAGE_H_ */
