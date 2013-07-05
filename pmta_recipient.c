/**
 * @file pmta_recipient.c
 * @date 29.09.2010
 * @author Vladimir Kolesnikov <vladimir@extrememember.com>
 * @brief @c PmtaRecipient class implementation
 * @details
<PRE>
final class PmtaRecipient
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

	public function __construct($address)
	{
		$this->recipient = PmtaRcptAlloc();

		if (!PmtaRcptInit($this->recipient, $address)) {
			throw new PmtaErrorRecipient(PmtaRcptGetLastError($this->recipient), PmtaRcptGetLastErrorType($this->recipient));
		}

		$this->address = $address;
	}

	public function __destruct()
	{
		PmtaRcptFree($this->recipient);
	}

	public function __get($property)
	{
		static $names = array('address', 'notify', 'variables');
		for ($i=0; $i<count($names); ++$i) {
			if ($property == $names[$i]) {
				return $this->$property;
			}
		}

		trigger_error("Property PmtaRecipient::{$property} does not exist", E_WARNING);
	}

	public function __isset($property)
	{
		static $names = array('address', 'notify', 'variables');
		for ($i=0; $i<count($names); ++$i) {
			if ($property == $names[$i]) {
				return true;
			}
		}

		return false;
	}

	public function __set($name, $value)
	{
		if ('notify' == $name) {
			$res = PmtaRcptSetNotify($this->recipient, $value);
			if ($res) {
				$this->notify = $value;
			}
			else {
				throw new PmtaErrorRecipient(PmtaRcptGetLastError($this->recipient), PmtaRcptGetLastErrorType($this->recipient));
			}
		}
		else {
			trigger_error("Cannot set property PmtaRecipient::{$property}", E_WARNING);
		}
	}

	public function defineVariable($name, $value)
	{
		return PmtaRcptDefineVariable($this->recipient, $name, $value);
	}

	public function getLastError()
	{
		return new PmtaErrorRecipient(PmtaRcptGetLastError($this->recipient), PmtaRcptGetLastErrorType($this->recipient));
	}

	private function __clone() {}
}
</PRE>
 */

#include "pmta_recipient.h"
#include "pmta_error.h"
#include "pmta_common.h"

/**
 * @brief Retrieves @c PmtaRcpt resource (@c $recipient property)
 * @param object @c PmtaRecipient class
 * @return @c PmtaRcpt resource
 */
PmtaRcpt getRecipient(zval* object TSRMLS_DC)
{
	PmtaRcpt result;
	zval* rv = zend_read_property(pmta_rcpt_class, object, PHPPMTA_SL("recipient"), 0 TSRMLS_CC);
	ZEND_FETCH_RESOURCE_NO_RETURN(result, PmtaRcpt, &rv, -1, PMTA_RECIPIENT_RES_NAME, le_pmta_recipient);
	return result;
}

/**
 * @brief Locks PmtaRecipient class instance by setting its @c $locked property ro 1
 * @param object PmtaRecipient object
 */
void lock_recipient(zval* object TSRMLS_DC)
{
	zend_update_property_bool(pmta_rcpt_class, object, PHPPMTA_SL("locked"), 1 TSRMLS_CC);
	zend_update_property_null(pmta_rcpt_class, object, PHPPMTA_SL("recipient") TSRMLS_CC);
}

/**
 * @brief Checks whether the object is locked
 * @param object @c PmtaRecipient object
 * @return Whether the object is locked
 * @retval 0 Not locked
 * @retval 1 Locked
 */
static int is_locked(zval* object TSRMLS_DC)
{
	zval* locked = zend_read_property(pmta_rcpt_class, object, PHPPMTA_SL("locked"), 0 TSRMLS_CC);
	return Z_BVAL_P(locked);
}

/**
 * @brief @c PmtaRecipient properties
 */
static const struct props pmtarcpt_properties[] = {
	{ PHPPMTA_SL("recipient"), PHPPMTA_SL("/**\n * @property resource\n */") },
	{ PHPPMTA_SL("address"),   PHPPMTA_SL("/**\n * @property string\n */") },
	{ PHPPMTA_SL("notify"),    PHPPMTA_SL("/**\n * @property int\n */") },
	{ PHPPMTA_SL("variables"), PHPPMTA_SL("/**\n * @property array\n */") }
};

static PHP_METHOD(PmtaRecipient, __construct)
{
	zval* c;
	PmtaRcpt rcpt;
	char* address;
	int address_len;
	BOOL result;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &address, &address_len)) {
		RETURN_NULL();
	}

	rcpt = PmtaRcptAlloc();
	if (!rcpt) {
		throw_pmta_error(pmta_error_recipient_class, PmtaApiERROR_PHP_API, "PmtaRcptAlloc() failed", NULL TSRMLS_CC);
		RETURN_NULL();
	}

	MAKE_STD_ZVAL(c);
	ZEND_REGISTER_RESOURCE(c, rcpt, le_pmta_recipient);

	zend_update_property(pmta_rcpt_class, getThis(), PHPPMTA_SL("recipient"), c TSRMLS_CC);
	zval_ptr_dtor(&c);

	result = PmtaRcptInit(rcpt, address);
	if (FALSE == result) {
		throw_pmta_error(pmta_error_recipient_class, PmtaRcptGetLastErrorType(rcpt), PmtaRcptGetLastError(rcpt), NULL TSRMLS_CC);
	}

	zend_update_property_string(pmta_rcpt_class, getThis(), PHPPMTA_SL("address"), address TSRMLS_CC);

	MAKE_STD_ZVAL(c);
	array_init(c);
	zend_update_property(pmta_rcpt_class, getThis(), PHPPMTA_SL("variables"), c TSRMLS_CC);
	zend_update_property_long(pmta_rcpt_class, getThis(), PHPPMTA_SL("notify"), PmtaRcptNOTIFY_NEVER TSRMLS_CC);
	zend_update_property_bool(pmta_rcpt_class, getThis(), PHPPMTA_SL("locked"), 0 TSRMLS_CC);
	zval_ptr_dtor(&c);
}

static PHP_METHOD(PmtaRecipient, __get)
{
	generic_get(pmtarcpt_properties, PHPPMTA_NELEMS(pmtarcpt_properties), pmta_rcpt_class, "PmtaRecipient", INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static PHP_METHOD(PmtaRecipient, __isset)
{
	generic_isset(pmtarcpt_properties, PHPPMTA_NELEMS(pmtarcpt_properties), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static PHP_METHOD(PmtaRecipient, __set)
{
	char* property;
	int property_len;
	long int val;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &property, &property_len, &val)) {
		RETURN_NULL();
	}

	if (is_locked(getThis() TSRMLS_CC)) {
		throw_pmta_error(pmta_error_recipient_class, PmtaApiERROR_PHP_API, "Cannot modify locked object", NULL TSRMLS_CC);
	}

	if (!strcmp("notify", property)) {
		BOOL res;
		PmtaRcpt rcpt = getRecipient(getThis() TSRMLS_CC);

		ZEND_VERIFY_RESOURCE(rcpt);

		res = PmtaRcptSetNotify(rcpt, val);
		if (TRUE == res) {
			zend_update_property_long(pmta_rcpt_class, getThis(), PHPPMTA_SL("notify"), val TSRMLS_CC);
		}
		else {
			throw_pmta_error(pmta_error_recipient_class, PmtaRcptGetLastErrorType(rcpt), PmtaRcptGetLastError(rcpt), NULL TSRMLS_CC);
		}
	}

	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot set property PmtaRecipient::%s", property);
}

static PHP_METHOD(PmtaRecipient, __clone)
{
}

static PHP_METHOD(PmtaRecipient, defineVariable)
{
	PmtaRcpt rcpt;
	char* name;
	int name_len;
	char* value;
	int value_len;
	BOOL res;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &name, &name_len, &value, &value_len)) {
		RETURN_NULL();
	}

	if (is_locked(getThis() TSRMLS_CC)) {
		throw_pmta_error(pmta_error_recipient_class, PmtaApiERROR_PHP_API, "Cannot modify locked object", NULL TSRMLS_CC);
	}

	rcpt = getRecipient(getThis() TSRMLS_CC);
	ZEND_VERIFY_RESOURCE(rcpt);

	res = PmtaRcptDefineVariable(rcpt, name, value);
	if (TRUE == res) {
		zval* c = zend_read_property(pmta_rcpt_class, getThis(), PHPPMTA_SL("variables"), 0 TSRMLS_CC);
		add_assoc_string(c, name, value, 1);
		zend_update_property(pmta_rcpt_class, getThis(), PHPPMTA_SL("variables"), c TSRMLS_CC);
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static PHP_METHOD(PmtaRecipient, getLastError)
{
	if (return_value_used) {
		PmtaRcpt rcpt = getRecipient(getThis() TSRMLS_CC);
		ZEND_VERIFY_RESOURCE(rcpt);

		throw_pmta_error(pmta_error_recipient_class, PmtaRcptGetLastErrorType(rcpt), PmtaRcptGetLastError(rcpt), &return_value TSRMLS_CC);
	}
}

/**
 * @brief arginfo for @c __construct()
 */
PHPPMTA_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_construct, 0, 0, 1)
	ZEND_ARG_INFO(0, address)
ZEND_END_ARG_INFO()

/**
 * @brief arginfo for @c defineVariable()
 */
PHPPMTA_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_defvar, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

/**
 * @brief @c PmtaRecipient class methods
 */
static
#if ZEND_MODULE_API_NO > 20060613
const
#endif
zend_function_entry pmta_rcpt_class_methods[] = {
	PHP_ME(PmtaRecipient, __construct,      arginfo_construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(PmtaRecipient, __get,            arginfo_get,       ZEND_ACC_PUBLIC)
	PHP_ME(PmtaRecipient, __set,            arginfo_set,       ZEND_ACC_PUBLIC)
	PHP_ME(PmtaRecipient, __isset,          arginfo_get,       ZEND_ACC_PUBLIC)
	PHP_ME(PmtaRecipient, __clone,          arginfo_empty,     ZEND_ACC_PRIVATE | ZEND_ACC_CLONE)
	PHP_ME(PmtaRecipient, defineVariable,   arginfo_defvar,    ZEND_ACC_PUBLIC)
	PHP_ME(PmtaRecipient, getLastError,     arginfo_empty,     ZEND_ACC_PUBLIC)

	{ NULL, NULL, NULL, 0, 0 }
};

ZEND_RSRC_DTOR_FUNC(pmta_recipient_dtor)
{
	PmtaRcpt recipient = (PmtaRcpt)rsrc->ptr;
	if (recipient) {
		PmtaRcptFree(recipient);
	}
}

void pmtarcpt_register_class(TSRMLS_D)
{
	zend_class_entry e;
	size_t i;

	INIT_CLASS_ENTRY(e, "PmtaRecipient", pmta_rcpt_class_methods);

	pmta_rcpt_class            = zend_register_internal_class(&e TSRMLS_CC);
	pmta_rcpt_class->ce_flags |= ZEND_ACC_FINAL_CLASS;

	zend_declare_property_bool(pmta_rcpt_class, PHPPMTA_SL("locked"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);

	for (i=0; i<PHPPMTA_NELEMS(pmtarcpt_properties); ++i) {
		zval* property;
#ifdef ALLOC_PERMANENT_ZVAL
		ALLOC_PERMANENT_ZVAL(property);
#else
		property = malloc(sizeof(zval));
#endif
		INIT_ZVAL(*property);

		zend_declare_property_ex(pmta_rcpt_class, pmtarcpt_properties[i].name, pmtarcpt_properties[i].len, property, ZEND_ACC_PRIVATE, pmtarcpt_properties[i].comment, pmtarcpt_properties[i].comment_len TSRMLS_CC);
	}

	zend_declare_class_constant_long(pmta_rcpt_class, PHPPMTA_SL("NOTIFY_NEVER"),   PmtaRcptNOTIFY_NEVER   TSRMLS_CC);
	zend_declare_class_constant_long(pmta_rcpt_class, PHPPMTA_SL("NOTIFY_SUCCESS"), PmtaRcptNOTIFY_SUCCESS TSRMLS_CC);
	zend_declare_class_constant_long(pmta_rcpt_class, PHPPMTA_SL("NOTIFY_FAILURE"), PmtaRcptNOTIFY_FAILURE TSRMLS_CC);
	zend_declare_class_constant_long(pmta_rcpt_class, PHPPMTA_SL("NOTIFY_DELAY"),   PmtaRcptNOTIFY_DELAY   TSRMLS_CC);
	zend_declare_class_constant_long(pmta_rcpt_class, PHPPMTA_SL("NOTIFY_ALWAYS"),  PmtaRcptNOTIFY_SUCCESS | PmtaRcptNOTIFY_FAILURE | PmtaRcptNOTIFY_DELAY TSRMLS_CC);
}
