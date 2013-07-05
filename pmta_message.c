/**
 * @file pmta_message.c
 * @date 29.09.2010
 * @author Vladimir Kolesnikov <vladimir@extrememember.com>
 * @brief @c PmtaMessage class implementation
 * @details
<PRE>
final class PmtaMessage
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

	public function __construct($originator)
	{
		$this->message = PmtaMsgAlloc();

		if (PmtaMsgInit($this->message, $originator)) {
			$this->originator = $originator;
		}
		else {
			throw new PmtaErrorMessage(PmtaMsgGetLastError($this->message), PmtaMsgGetLastErrorType($this->message));
		}
	}

	public function __destruct()
	{
		PmtaMsgFree($this->message);
	}

	public function __get($property)
	{
		$properties = array('originator', 'verp', 'return_type', 'envelope_id', 'vmta', 'jobid', 'encoding', 'recipients');
		for ($i=0; $i<count($properties); ++$i) {
			if ($property == $properties[$i]) {
				return $this->$property;
			}
		}
			trigger_error("Undefined property PmtaMessage::{$property}", E_WARNING);
	}

	public function __isset($property)
	{
		$properties = array('originator', 'verp', 'return_type', 'envelope_id', 'vmta', 'jobid', 'encoding', 'recipients');
		for ($i=0; $i<count($properties); ++$i) {
			if ($property == $properties[$i]) {
				return true;
			}
		}

		return false;
	}

	public function __set($property, $value)
	{
		switch ($property) {
			case 'verp':        $res = PmtaMsgSetVerp($this->message, (bool)$value); break;
			case 'return_type': $res = PmtaMsgSetReturnType($this->message, $value); break;
			case 'envelope_id': $res = PmtaMsgSetEnvelopeId($this->message, $value); break;
			case 'vmta':        $res = PmtaMsgSetVirtualMta($this->message, $value); break;
			case 'jobid':       $res = PmtaMsgSetJobId($this->message, $value); break;
			case 'encoding':    $res = PmtaMsgSetEncoding($this->message, $value); break;
			default:
				trigger_error("Cannot set property PmtaMessage::{$property}", E_USER_WARNING);
				return;
		}

		if (!$res) {
			throw new PmtaErrorMessage(PmtaMsgGetLastError($this->message), PmtaMsgGetLastErrorType($this->message));
		}

		$this->$property = $value;
	}

	public function beginPart($number)
	{
		return PmtaMsgBeginPart($this->message, $number);
	}

	public function addData($data)
	{
		return PmtaMsgAddData($this->message, $data, strlen($data));
	}

	public function addMergeData($data)
	{
		return PmtaMsgAddMergeData($this->message, $data, strlen($data));
	}

	public function addDateHeader()
	{
		return PmtaMsgAddDateHeader($this->message);
	}

	public function addRecipient(PmtaRecipient $recipient)
	{
		$res = PmtaMsgAddRecipient($this->message, $recipient);
		if ($res) {
			unset($recipient->recipient);
			$this->recipients[] = $recipient;
			return true;
		}

		return false;
	}

	public function getLastError()
	{
		return new PmtaErrorMessage(PmtaMsgGetLastError($this->message), PmtaMsgGetLastErrorType($this->message));
	}

	private function __clone() {}
}
</PRE>
 */

#include "pmta_message.h"
#include "pmta_recipient.h"
#include "pmta_error.h"
#include "pmta_common.h"

/**
 * @brief Return PmtaMsg resource
 * @param object PmtaMessage object
 * @return PmtaMsg resource (@c $message property)
 */
PmtaMsg getMessage(zval* object TSRMLS_DC)
{
	PmtaMsg result;
	zval* rv = zend_read_property(pmta_msg_class, object, PHPPMTA_SL("message"), 0 TSRMLS_CC);
	ZEND_FETCH_RESOURCE_NO_RETURN(result, PmtaMsg, &rv, -1, PMTA_MESSAGE_RES_NAME, le_pmta_message);
	return result;
}

/**
 * @brief @c PmtaMessage properties
 */
static const struct props pmtamsg_properties[] = {
	{ PHPPMTA_SL("message"),     PHPPMTA_SL("/**\n * @property resource\n */") },
	{ PHPPMTA_SL("originator"),  PHPPMTA_SL("/**\n * @property string\n */") },
	{ PHPPMTA_SL("verp"),        PHPPMTA_SL("/**\n * @property bool\n */") },
	{ PHPPMTA_SL("return_type"), PHPPMTA_SL("/**\n * @property int\n */") },
	{ PHPPMTA_SL("envelope_id"), PHPPMTA_SL("/**\n * @property string\n */") },
	{ PHPPMTA_SL("vmta"),        PHPPMTA_SL("/**\n * @property string\n */") },
	{ PHPPMTA_SL("jobid"),       PHPPMTA_SL("/**\n * @property string\n */") },
	{ PHPPMTA_SL("encoding"),    PHPPMTA_SL("/**\n * @property int\n */") },
	{ PHPPMTA_SL("recipients"),  PHPPMTA_SL("/**\n * @property array\n */") }
};

/**
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 */
static PHP_METHOD(PmtaMessage, __construct)
{
	zval* c;
	PmtaMsg msg;
	char* originator;
	int originator_len;
	BOOL result;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &originator, &originator_len)) {
		RETURN_NULL();
	}

	msg = PmtaMsgAlloc();
	if (!msg) {
		throw_pmta_error(pmta_error_recipient_class, PmtaApiERROR_PHP_API, "PmtaMsgAlloc() failed", NULL TSRMLS_CC);
		RETURN_NULL();
	}

	MAKE_STD_ZVAL(c);
	ZEND_REGISTER_RESOURCE(c, msg, le_pmta_message);

	zend_update_property(pmta_msg_class, getThis(), PHPPMTA_SL("message"), c TSRMLS_CC);
	zval_ptr_dtor(&c);

	result = PmtaMsgInit(msg, originator);
	if (FALSE == result) {
		throw_pmta_error(pmta_error_message_class, PmtaMsgGetLastErrorType(msg), PmtaMsgGetLastError(msg), NULL TSRMLS_CC);
	}

	zend_update_property_string(pmta_msg_class, getThis(), PHPPMTA_SL("originator"), originator TSRMLS_CC);

	MAKE_STD_ZVAL(c);
	array_init(c);
	zend_update_property(pmta_msg_class, getThis(), PHPPMTA_SL("recipients"), c TSRMLS_CC);
	zval_ptr_dtor(&c);
}

/**
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 */
static PHP_METHOD(PmtaMessage, __get)
{
	generic_get(pmtamsg_properties, PHPPMTA_NELEMS(pmtamsg_properties), pmta_msg_class, "PmtaMessage", INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

/**
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 */
static PHP_METHOD(PmtaMessage, __isset)
{
	generic_isset(pmtamsg_properties, PHPPMTA_NELEMS(pmtamsg_properties), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

/**
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 */
static PHP_METHOD(PmtaMessage, __set)
{
	PmtaMsg msg = getMessage(getThis() TSRMLS_CC);
	char* property;
	int property_len;
	zval* val;
	BOOL res;

	ZEND_VERIFY_RESOURCE(msg);

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &property, &property_len, &val)) {
		RETURN_NULL();
	}

	if (!strcmp(property, "verp")) {
		convert_to_boolean(val);
		res = PmtaMsgSetVerp(msg, Z_BVAL_P(val) ? TRUE : FALSE);
	}
	else if (!strcmp(property, "return_type")) {
		convert_to_long(val);
		res = PmtaMsgSetReturnType(msg, (PmtaMsgRETURN)(Z_LVAL_P(val)));
	}
	else if (!strcmp(property, "envelope_id")) {
		convert_to_string(val);
		res = PmtaMsgSetEnvelopeId(msg, Z_STRVAL_P(val));
	}
	else if (!strcmp(property, "vmta")) {
		convert_to_string(val);
		res = PmtaMsgSetVirtualMta(msg, Z_STRVAL_P(val));
	}
	else if (!strcmp(property, "jobid")) {
		convert_to_string(val);
		res = PmtaMsgSetJobId(msg, Z_STRVAL_P(val));
	}
	else if (!strcmp(property, "encoding")) {
		convert_to_long(val);
		res = PmtaMsgSetEncoding(msg, (PmtaMsgENCODING)(Z_LVAL_P(val)));
	}
	else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot set property PmtaMessage::%s", property);
		RETURN_NULL();
	}

	if (FALSE == res) {
		throw_pmta_error(pmta_error_message_class, PmtaMsgGetLastErrorType(msg), PmtaMsgGetLastError(msg), NULL TSRMLS_CC);
	}
	else {
		zend_update_property(pmta_msg_class, getThis(), property, property_len, val TSRMLS_CC);
	}
}

/**
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 */
static PHP_METHOD(PmtaMessage, __clone)
{
}

/**
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 */
static PHP_METHOD(PmtaMessage, beginPart)
{
	PmtaMsg msg = getMessage(getThis() TSRMLS_CC);
	long int part;
	BOOL res;

	ZEND_VERIFY_RESOURCE(msg);

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &part)) {
		RETURN_NULL();
	}

	res = PmtaMsgBeginPart(msg, part);
	if (TRUE == res) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 */
static PHP_METHOD(PmtaMessage, addData)
{
	PmtaMsg msg = getMessage(getThis() TSRMLS_CC);
	char* data;
	int data_len;
	BOOL res;

	ZEND_VERIFY_RESOURCE(msg);

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &data_len)) {
		RETURN_NULL();
	}

	res = PmtaMsgAddData(msg, data, data_len);
	if (TRUE == res) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 */
static PHP_METHOD(PmtaMessage, addMergeData)
{
	PmtaMsg msg = getMessage(getThis() TSRMLS_CC);
	char* data;
	int data_len;
	BOOL res;

	ZEND_VERIFY_RESOURCE(msg);

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &data_len)) {
		RETURN_NULL();
	}

	res = PmtaMsgAddMergeData(msg, data, data_len);
	if (TRUE == res) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 */
static PHP_METHOD(PmtaMessage, addDateHeader)
{
	PmtaMsg msg = getMessage(getThis() TSRMLS_CC);
	BOOL res;

	ZEND_VERIFY_RESOURCE(msg);

	res = PmtaMsgAddDateHeader(msg);
	if (TRUE == res) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 */
static PHP_METHOD(PmtaMessage, addRecipient)
{
	PmtaMsg msg = getMessage(getThis() TSRMLS_CC);
	PmtaRcpt rcpt;
	zval* recipient;
	BOOL res;

	ZEND_VERIFY_RESOURCE(msg);

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &recipient, pmta_rcpt_class)) {
		RETURN_NULL();
	}

	rcpt = getRecipient(recipient TSRMLS_CC);
	ZEND_VERIFY_RESOURCE(rcpt);

	res = PmtaMsgAddRecipient(msg, rcpt);
	if (TRUE == res) {
		zval* c = zend_read_property(pmta_msg_class, getThis(), PHPPMTA_SL("recipients"), 0 TSRMLS_CC);
#ifdef Z_ADDREF_P
		Z_ADDREF_P(recipient);
#else
		ZVAL_ADDREF(recipient);
#endif
		add_next_index_zval(c, recipient);
		zend_update_property(pmta_msg_class, getThis(), PHPPMTA_SL("recipients"), c TSRMLS_CC);
		lock_recipient(recipient TSRMLS_CC);
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 */
static PHP_METHOD(PmtaMessage, getLastError)
{
	if (return_value_used) {
		PmtaMsg msg = getMessage(getThis() TSRMLS_CC);
		ZEND_VERIFY_RESOURCE(msg);

		throw_pmta_error(pmta_error_message_class, PmtaMsgGetLastErrorType(msg), PmtaMsgGetLastError(msg), &return_value TSRMLS_CC);
	}
}

/**
 * @brief arginfo for @c __construct()
 */
PHPPMTA_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_construct, 0, 0, 1)
	ZEND_ARG_INFO(0, originator)
ZEND_END_ARG_INFO()

/**
 * @brief arginfo for @c beginPart()
 */
PHPPMTA_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_beginpart, 0, 0, 1)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

/**
 * @brief arginfo for @c addData() and @c addMergeData()
 */
PHPPMTA_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_adddata, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

/**
 * @brief arginfo for @c addRecipient()
 */
PHPPMTA_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_addrecipient, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, recipient, PmtaRecipient, 0)
ZEND_END_ARG_INFO()

/**
 * @brief @c PmtaMessage class methods
 */
static
#if ZEND_MODULE_API_NO > 20060613
const
#endif
zend_function_entry pmta_msg_class_methods[] = {
	PHP_ME(PmtaMessage, __construct,      arginfo_construct,    ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(PmtaMessage, __get,            arginfo_get,          ZEND_ACC_PUBLIC)
	PHP_ME(PmtaMessage, __set,            arginfo_set,          ZEND_ACC_PUBLIC)
	PHP_ME(PmtaMessage, __isset,          arginfo_get,          ZEND_ACC_PUBLIC)
	PHP_ME(PmtaMessage, __clone,          arginfo_empty,        ZEND_ACC_PRIVATE | ZEND_ACC_CLONE)
	PHP_ME(PmtaMessage, beginPart,        arginfo_beginpart,    ZEND_ACC_PUBLIC)
	PHP_ME(PmtaMessage, addData,          arginfo_adddata,      ZEND_ACC_PUBLIC)
	PHP_ME(PmtaMessage, addMergeData,     arginfo_adddata,      ZEND_ACC_PUBLIC)
	PHP_ME(PmtaMessage, addDateHeader,    arginfo_empty,        ZEND_ACC_PUBLIC)
	PHP_ME(PmtaMessage, addRecipient,     arginfo_addrecipient, ZEND_ACC_PUBLIC)
	PHP_ME(PmtaMessage, getLastError,     arginfo_empty,        ZEND_ACC_PUBLIC)

	{ NULL, NULL, NULL, 0, 0 }
};

ZEND_RSRC_DTOR_FUNC(pmta_message_dtor)
{
	PmtaMsg message = (PmtaMsg)rsrc->ptr;
	if (message) {
		PmtaMsgFree(message);
	}
}

void pmtamsg_register_class(TSRMLS_D)
{
	zend_class_entry e;
	size_t i;

	INIT_CLASS_ENTRY(e, "PmtaMessage", pmta_msg_class_methods);

	pmta_msg_class            = zend_register_internal_class(&e TSRMLS_CC);
	pmta_msg_class->ce_flags |= ZEND_ACC_FINAL_CLASS;

	for (i=0; i<PHPPMTA_NELEMS(pmtamsg_properties); ++i) {
		zval* property;
#ifdef ALLOC_PERMANENT_ZVAL
		ALLOC_PERMANENT_ZVAL(property);
#else
		property = malloc(sizeof(zval));
#endif
		INIT_ZVAL(*property);

		zend_declare_property_ex(pmta_msg_class, pmtamsg_properties[i].name, pmtamsg_properties[i].len, property, ZEND_ACC_PRIVATE, pmtamsg_properties[i].comment, pmtamsg_properties[i].comment_len TSRMLS_CC);
	}

	zend_declare_class_constant_long(pmta_msg_class, PHPPMTA_SL("RETURN_HEADERS"),  PmtaMsgRETURN_HEADERS TSRMLS_CC);
	zend_declare_class_constant_long(pmta_msg_class, PHPPMTA_SL("RETURN_FULL"),     PmtaMsgRETURN_FULL TSRMLS_CC);

	zend_declare_class_constant_long(pmta_msg_class, PHPPMTA_SL("ENCODING_7BIT"),   PmtaMsgENCODING_7BIT TSRMLS_CC);
	zend_declare_class_constant_long(pmta_msg_class, PHPPMTA_SL("ENCODING_8BIT"),   PmtaMsgENCODING_8BIT TSRMLS_CC);
	zend_declare_class_constant_long(pmta_msg_class, PHPPMTA_SL("ENCODING_BASE64"), PmtaMsgENCODING_BASE64 TSRMLS_CC);
}
