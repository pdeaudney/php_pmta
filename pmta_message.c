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

static zend_object_handlers pmtamsg_object_handlers;

struct pmtamsg_object {
	zend_object obj;
	PmtaMsg msg;
	char* originator;
	char* envid;
	char* vmta;
	char* jobid;
	HashTable* recipients;
	int rettype;
	int encoding;
	int verp;
};

static inline struct pmtamsg_object* fetchPmtaMsgObject(zval* zobj TSRMLS_DC)
{
	return (struct pmtamsg_object*)zend_objects_get_address(zobj TSRMLS_CC);
}

PmtaMsg getMessage(zval* object TSRMLS_DC)
{
	return fetchPmtaMsgObject(object TSRMLS_CC)->msg;
}

static zval* pmtamsg_read_property_internal(struct pmtamsg_object* obj, zval* member, int type TSRMLS_DC)
{
	zval* ret;
	MAKE_STD_ZVAL(ret);

	if (ISSTR(member, "originator")) {
		ZVAL_STRING(ret, obj->originator, 1);
	}
	else if (ISSTR(member, "envelope_id")) {
		ZVAL_STRING(ret, obj->envid, 1);
	}
	else if (ISSTR(member, "vmta")) {
		ZVAL_STRING(ret, obj->vmta, 1);
	}
	else if (ISSTR(member, "jobid")) {
		ZVAL_STRING(ret, obj->jobid, 1);
	}
	else if (ISSTR(member, "encoding")) {
		ZVAL_LONG(ret, obj->encoding);
	}
	else if (ISSTR(member, "return_type")) {
		ZVAL_LONG(ret, obj->rettype);
	}
	else if (ISSTR(member, "verp")) {
		ZVAL_LONG(ret, obj->verp);
	}
	else if (ISSTR(member, "recipients")) {
		zval* tmp;
		array_init_size(ret, zend_hash_num_elements(obj->recipients));
		zend_hash_copy(Z_ARRVAL_P(ret), obj->recipients, (copy_ctor_func_t)zval_add_ref, (void*)&tmp, sizeof(zval*));
	}
	else {
		if (type != BP_VAR_IS) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Undefined property PmtaMessage::%s", Z_STRVAL_P(member));
		}

		ZVAL_NULL(ret);
	}

	assert(Z_REFCOUNT_P(ret) == 1);
	Z_SET_REFCOUNT_P(ret, 0);
	return ret;
}

static zval* pmtamsg_read_property(zval* object, zval* member, int type ZLK_DC TSRMLS_DC)
{
	zval tmp;
	zval* ret;
	struct pmtamsg_object* obj = fetchPmtaMsgObject(object TSRMLS_CC);

	if (obj->obj.ce->type != ZEND_INTERNAL_CLASS) {
		return zend_get_std_object_handlers()->read_property(object, member, type ZLK_CC TSRMLS_CC);
	}

	if (UNEXPECTED(Z_TYPE_P(member) != IS_STRING)) {
		ZVAL_ZVAL(&tmp, member, 1, 0);
		convert_to_string(&tmp);
		member = &tmp;
	}

	ret = pmtamsg_read_property_internal(obj, member, type TSRMLS_CC);

	if (UNEXPECTED(member == &tmp)) {
		zval_dtor(&tmp);
	}

	return ret;
}

static int pmtamsg_has_property_internal(struct pmtamsg_object* obj, zval* member, int has_set_exists TSRMLS_DC)
{
	int retval = 1;

	if (ISSTR(member, "originator")) {
		if (1 == has_set_exists) {
			retval = (obj->originator && obj->originator[0]);
		}
	}
	else if (ISSTR(member, "envelope_id")) {
		if (1 == has_set_exists) {
			retval = (obj->envid && obj->envid[0]);
		}
	}
	else if (ISSTR(member, "vmta")) {
		if (1 == has_set_exists) {
			retval = (obj->vmta && obj->vmta[0]);
		}
	}
	else if (ISSTR(member, "jobid")) {
		if (1 == has_set_exists) {
			retval = (obj->jobid && obj->jobid[0]);
		}
	}
	else if (ISSTR(member, "encoding")) {
		if (1 == has_set_exists) {
			retval = (obj->encoding != 0);
		}
	}
	else if (ISSTR(member, "return_type")) {
		if (1 == has_set_exists) {
			retval = (obj->rettype != 0);
		}
	}
	else if (ISSTR(member, "verp")) {
		if (1 == has_set_exists) {
			retval = (obj->verp != 0);
		}
	}
	else if (ISSTR(member, "recipients")) {
		if (1 == has_set_exists) {
			retval = (obj->recipients && zend_hash_num_elements(obj->recipients));
		}
	}
	else {
		retval = 0;
	}

	return retval;
}

/**
 * @property object
 * @property member
 * @property has_set_exists 0 = isset(), 1 = empty()
 */
static int pmtamsg_has_property(zval* object, zval* member, int has_set_exists ZLK_DC TSRMLS_DC)
{
	zval tmp;
	int retval = 1;
	struct pmtamsg_object* obj = fetchPmtaMsgObject(object TSRMLS_CC);

	if (obj->obj.ce->type != ZEND_INTERNAL_CLASS) {
		return zend_get_std_object_handlers()->has_property(object, member, has_set_exists ZLK_CC TSRMLS_CC);
	}

	if (UNEXPECTED(Z_TYPE_P(member) != IS_STRING)) {
		ZVAL_ZVAL(&tmp, member, 1, 0);
		convert_to_string(&tmp);
		member = &tmp;
	}

	retval = pmtamsg_has_property_internal(obj, member, has_set_exists TSRMLS_CC);

	if (UNEXPECTED(member == &tmp)) {
		zval_dtor(&tmp);
	}

	return retval;
}

static void pmtamsg_write_property_internal(struct pmtamsg_object* obj, zval* member, zval* value TSRMLS_DC)
{
	BOOL res;

	if (ISSTR(member, "encoding") || ISSTR(member, "return_type") || ISSTR(member, "verp")) {
		long int v;
		int* property;

		if (Z_TYPE_P(value) == IS_LONG) {
			v = Z_LVAL_P(value);
		}
		else {
			zval lval;
			ZVAL_ZVAL(&lval, value, 1, 0);
			convert_to_long(&lval);
			v = Z_LVAL(lval);
			zval_dtor(&lval);
		}

		switch (Z_STRVAL_P(member)[0]) {
			case 'e': res = PmtaMsgSetEncoding(obj->msg, (PmtaMsgENCODING)v); property = &obj->encoding; break;
			case 'r': res = PmtaMsgSetReturnType(obj->msg, (PmtaMsgRETURN)v); property = &obj->rettype;  break;
			case 'v': res = PmtaMsgSetVerp(obj->msg, v ? TRUE : FALSE);       property = &obj->verp;     break;
			default:  res = FALSE; property = NULL; break;
		}

		if (FALSE == res) {
			throw_pmta_error(pmta_error_message_class, PmtaMsgGetLastErrorType(obj->msg), PmtaMsgGetLastError(obj->msg), NULL TSRMLS_CC);
		}
		else {
			*property = v;
		}
	}
	else if (ISSTR(member, "envelope_id") || ISSTR(member, "vmta") || ISSTR(member, "jobid")) {
		char* v;
		char** property;

		if (Z_TYPE_P(value) == IS_STRING) {
			v = estrndup(Z_STRVAL_P(value), Z_STRLEN_P(value));
		}
		else {
			zval str;
			ZVAL_ZVAL(&str, value, 1, 0);
			convert_to_string(&str);
			v = Z_STRVAL(str);
			/* Skipping zval_dtor because we stole the underlying pointer */
		}

		switch (Z_STRVAL_P(member)[0]) {
			case 'e': res = PmtaMsgSetEnvelopeId(obj->msg, v); property = &obj->envid; break;
			case 'v': res = PmtaMsgSetVirtualMta(obj->msg, v); property = &obj->vmta;  break;
			case 'j': res = PmtaMsgSetJobId(obj->msg, v);      property = &obj->jobid; break;
			default:  res = FALSE; property = NULL;
		}

		if (FALSE == res) {
			throw_pmta_error(pmta_error_message_class, PmtaMsgGetLastErrorType(obj->msg), PmtaMsgGetLastError(obj->msg), NULL TSRMLS_CC);
		}
		else {
			if (*property) {
				efree(*property);
			}

			*property = v;
		}
	}
	else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot set property PmtaMessage::%s", Z_STRVAL_P(member));
	}
}

static void pmtamsg_write_property(zval* object, zval* member, zval* value ZLK_DC TSRMLS_DC)
{
	zval tmp;
	struct pmtamsg_object* obj = fetchPmtaMsgObject(object TSRMLS_CC);

	if (obj->obj.ce->type != ZEND_INTERNAL_CLASS) {
		return zend_get_std_object_handlers()->write_property(object, member, value ZLK_CC TSRMLS_CC);
	}

	if (UNEXPECTED(Z_TYPE_P(member) != IS_STRING)) {
		ZVAL_ZVAL(&tmp, member, 1, 0);
		convert_to_string(&tmp);
		member = &tmp;
	}

	pmtamsg_write_property_internal(obj, member, value TSRMLS_CC);

	if (UNEXPECTED(member == &tmp)) {
		zval_dtor(&tmp);
	}
}

static HashTable* pmtamsg_get_properties(zval* object TSRMLS_DC)
{
	struct pmtamsg_object* obj = fetchPmtaMsgObject(object TSRMLS_CC);
	HashTable* props           = zend_std_get_properties(object TSRMLS_CC);
	zval* zv;
	zval* tmp;

	if (!obj->msg || GC_G(gc_active)) {
		return props;
	}

	if (obj->originator) {
		ALLOC_ZVAL(zv);
		ZVAL_STRING(zv, obj->originator, 1);
		zend_hash_update(props, "originator", sizeof("originator"), &zv, sizeof(zval*), NULL);
	}

	if (obj->envid) {
		ALLOC_ZVAL(zv);
		ZVAL_STRING(zv, obj->envid, 1);
		zend_hash_update(props, "envelope_id", sizeof("envelope_id"), &zv, sizeof(zval*), NULL);
	}

	if (obj->vmta) {
		ALLOC_ZVAL(zv);
		ZVAL_STRING(zv, obj->vmta, 1);
		zend_hash_update(props, "vmta", sizeof("vmta"), &zv, sizeof(zval*), NULL);
	}

	if (obj->jobid) {
		ALLOC_ZVAL(zv);
		ZVAL_STRING(zv, obj->jobid, 1);
		zend_hash_update(props, "jobid", sizeof("jobid"), &zv, sizeof(zval*), NULL);
	}

	ALLOC_ZVAL(zv);
	ZVAL_LONG(zv, obj->rettype);
	zend_hash_update(props, "return_type", sizeof("return_type"), &zv, sizeof(zval*), NULL);

	ALLOC_ZVAL(zv);
	ZVAL_LONG(zv, obj->encoding);
	zend_hash_update(props, "encoding", sizeof("encoding"), &zv, sizeof(zval*), NULL);

	ALLOC_ZVAL(zv);
	ZVAL_LONG(zv, obj->verp);
	zend_hash_update(props, "verp", sizeof("verp"), &zv, sizeof(zval*), NULL);

	ALLOC_ZVAL(zv);
	array_init_size(zv, zend_hash_num_elements(obj->recipients));
	zend_hash_copy(Z_ARRVAL_P(zv), obj->recipients, (copy_ctor_func_t)zval_add_ref, (void*)&tmp, sizeof(zval*));
	zend_hash_update(props, "recipients", sizeof("recipients"), (void*)&zv, sizeof(zval*), NULL);

	return props;
}

static void pmtamsg_dtor(void* v TSRMLS_DC)
{
	struct pmtamsg_object* obj = v;

	if (obj->originator) { efree(obj->originator);          }
	if (obj->envid)      { efree(obj->envid);               }
	if (obj->jobid)      { efree(obj->jobid);               }
	if (obj->vmta)       { efree(obj->vmta);                }
	if (obj->recipients) { FREE_HASHTABLE(obj->recipients); }
	if (obj->msg)        { PmtaMsgFree(obj->msg);           }

	zend_object_std_dtor(&(obj->obj) TSRMLS_CC);
	efree(obj);
}

static zend_object_value pmtamsg_ctor(zend_class_entry* ce TSRMLS_DC)
{
	struct pmtamsg_object* obj = ecalloc(1, sizeof(struct pmtamsg_object));
	zend_object_value retval;

	zend_object_std_init(&(obj->obj), ce TSRMLS_CC);
	retval.handle = zend_objects_store_put(
		obj,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		pmtamsg_dtor,
		NULL TSRMLS_CC
	);

	retval.handlers = &pmtamsg_object_handlers;

	return retval;
}

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
	struct pmtamsg_object* obj;
	char* originator;
	int originator_len;
	BOOL result;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &originator, &originator_len)) {
		RETURN_NULL();
	}

	obj = fetchPmtaMsgObject(getThis() TSRMLS_CC);

	obj->msg = PmtaMsgAlloc();
	if (!obj->msg) {
		throw_pmta_error(pmta_error_recipient_class, PmtaApiERROR_PHP_API, "PmtaMsgAlloc() failed", NULL TSRMLS_CC);
		RETURN_NULL();
	}

	result = PmtaMsgInit(obj->msg, originator);
	if (FALSE == result) {
		throw_pmta_error(pmta_error_message_class, PmtaMsgGetLastErrorType(obj->msg), PmtaMsgGetLastError(obj->msg), NULL TSRMLS_CC);
	}

	obj->originator = estrndup(originator, originator_len);

	ALLOC_HASHTABLE(obj->recipients);
	zend_hash_init(obj->recipients, 32, NULL, ZVAL_PTR_DTOR, 0);
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
	zval* property;
	zval* retval;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &property)) {
		RETURN_NULL();
	}

	retval = pmtamsg_read_property_internal(fetchPmtaMsgObject(getThis() TSRMLS_CC), property, BP_VAR_R TSRMLS_CC);
	Z_ADDREF_P(retval);
	RETURN_ZVAL(retval, 1, 1);
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
	zval* property;
	int retval;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &property)) {
		RETURN_NULL();
	}

	retval = pmtamsg_has_property_internal(fetchPmtaMsgObject(getThis() TSRMLS_CC), property, 1 TSRMLS_CC);
	RETURN_BOOL(retval);
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
	zval* property;
	zval* value;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &property, &value)) {
		RETURN_NULL();
	}

	pmtamsg_write_property_internal(fetchPmtaMsgObject(getThis() TSRMLS_CC), property, value TSRMLS_CC);
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
	struct pmtamsg_object* obj;
	long int part;
	BOOL res;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &part)) {
		RETURN_NULL();
	}

	obj = fetchPmtaMsgObject(getThis() TSRMLS_CC);
	res = PmtaMsgBeginPart(obj->msg, part);
	RETURN_BOOL(TRUE == res ? 1 : 0);
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
	struct pmtamsg_object* obj;
	char* data;
	int data_len;
	BOOL res;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &data_len)) {
		RETURN_NULL();
	}

	obj = fetchPmtaMsgObject(getThis() TSRMLS_CC);
	res = PmtaMsgAddData(obj->msg, data, data_len);
	RETURN_BOOL(TRUE == res ? 1 : 0);
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
	struct pmtamsg_object* obj;
	char* data;
	int data_len;
	BOOL res;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &data_len)) {
		RETURN_NULL();
	}

	obj = fetchPmtaMsgObject(getThis() TSRMLS_CC);
	res = PmtaMsgAddMergeData(obj->msg, data, data_len);
	RETURN_BOOL(TRUE == res ? 1 : 0);
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
	struct pmtamsg_object* obj;
	BOOL res;

	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_NULL();
	}

	obj = fetchPmtaMsgObject(getThis() TSRMLS_CC);
	res = PmtaMsgAddDateHeader(obj->msg);
	RETURN_BOOL(TRUE == res ? 1 : 0);
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
	struct pmtamsg_object* obj;
	PmtaRcpt rcpt;
	zval* recipient;
	BOOL res;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &recipient, pmta_rcpt_class)) {
		RETURN_NULL();
	}

	obj  = fetchPmtaMsgObject(getThis() TSRMLS_CC);
	rcpt = getRecipient(recipient TSRMLS_CC);
	res  = PmtaMsgAddRecipient(obj->msg, rcpt);
	if (TRUE == res) {
		Z_ADDREF_P(recipient);
		zend_hash_next_index_insert(obj->recipients, (void*)&recipient, sizeof(zval*), NULL);
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
	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_NULL();
	}

	if (return_value_used) {
		struct pmtamsg_object* obj = fetchPmtaMsgObject(getThis() TSRMLS_CC);
		throw_pmta_error(pmta_error_message_class, PmtaMsgGetLastErrorType(obj->msg), PmtaMsgGetLastError(obj->msg), &return_value TSRMLS_CC);
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
	PHP_ME(PmtaMessage, beginPart,        arginfo_beginpart,    ZEND_ACC_PUBLIC)
	PHP_ME(PmtaMessage, addData,          arginfo_adddata,      ZEND_ACC_PUBLIC)
	PHP_ME(PmtaMessage, addMergeData,     arginfo_adddata,      ZEND_ACC_PUBLIC)
	PHP_ME(PmtaMessage, addDateHeader,    arginfo_empty,        ZEND_ACC_PUBLIC)
	PHP_ME(PmtaMessage, addRecipient,     arginfo_addrecipient, ZEND_ACC_PUBLIC)
	PHP_ME(PmtaMessage, getLastError,     arginfo_empty,        ZEND_ACC_PUBLIC)
	PHP_FE_END
};

void pmtamsg_register_class(TSRMLS_D)
{
	zend_class_entry e;

	INIT_CLASS_ENTRY(e, "PmtaMessage", pmta_msg_class_methods);

	pmta_msg_class = zend_register_internal_class(&e TSRMLS_CC);

	pmta_msg_class->create_object = pmtamsg_ctor;
	pmta_msg_class->serialize     = zend_class_serialize_deny;
	pmta_msg_class->unserialize   = zend_class_unserialize_deny;

	pmtamsg_object_handlers = *zend_get_std_object_handlers();
	pmtamsg_object_handlers.clone_obj            = NULL;
	pmtamsg_object_handlers.read_property        = pmtamsg_read_property;
	pmtamsg_object_handlers.has_property         = pmtamsg_has_property;
	pmtamsg_object_handlers.write_property       = pmtamsg_write_property;
	pmtamsg_object_handlers.get_property_ptr_ptr = NULL;
	pmtamsg_object_handlers.get_properties       = pmtamsg_get_properties;

	zend_declare_class_constant_long(pmta_msg_class, ZEND_STRL("RETURN_HEADERS"),  PmtaMsgRETURN_HEADERS TSRMLS_CC);
	zend_declare_class_constant_long(pmta_msg_class, ZEND_STRL("RETURN_FULL"),     PmtaMsgRETURN_FULL TSRMLS_CC);

	zend_declare_class_constant_long(pmta_msg_class, ZEND_STRL("ENCODING_7BIT"),   PmtaMsgENCODING_7BIT TSRMLS_CC);
	zend_declare_class_constant_long(pmta_msg_class, ZEND_STRL("ENCODING_8BIT"),   PmtaMsgENCODING_8BIT TSRMLS_CC);
	zend_declare_class_constant_long(pmta_msg_class, ZEND_STRL("ENCODING_BASE64"), PmtaMsgENCODING_BASE64 TSRMLS_CC);
}
