/**
 * @file pmta_recipient.c
 * @date 29.09.2010
 * @author Vladimir Kolesnikov <vladimir@extrememember.com>
 * @brief @c PmtaRecipient class implementation
 * @details
<PRE>
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

static zend_object_handlers pmtarcpt_object_handlers;

struct pmtarcpt_object {
	zend_object obj;
	PmtaRcpt rcpt;
	char* address;
	HashTable* vars;
	int notify;
};

static inline struct pmtarcpt_object* fetchPmtaRcptObject(zval* zobj TSRMLS_DC)
{
	return (struct pmtarcpt_object*)zend_objects_get_address(zobj TSRMLS_CC);
}

/**
 * @brief Retrieves @c PmtaRcpt resource (@c $recipient property)
 * @param object @c PmtaRecipient class
 * @return @c PmtaRcpt resource
 */
PmtaRcpt getRecipient(zval* object TSRMLS_DC)
{
	return fetchPmtaRcptObject(object TSRMLS_CC)->rcpt;
}

/**
 * @brief Locks PmtaRecipient class instance by setting its @c $locked property ro 1
 * @param object PmtaRecipient object
 */
void lock_recipient(zval* object TSRMLS_DC)
{
	struct pmtarcpt_object* obj = fetchPmtaRcptObject(object TSRMLS_CC);
	if (obj->rcpt) {
		PmtaRcptFree(obj->rcpt);
		obj->rcpt = NULL;
	}
}

static zval* pmtarcpt_read_property_internal(struct pmtarcpt_object* obj, zval* member, int type TSRMLS_DC)
{
	zval* ret;
	MAKE_STD_ZVAL(ret);

	if (ISSTR(member, "address")) {
		ZVAL_STRING(ret, obj->address, 1);
	}
	else if (ISSTR(member, "notify")) {
		ZVAL_LONG(ret, obj->notify);
	}
	else if (ISSTR(member, "variables")) {
		zval* tmp;
		array_init_size(ret, zend_hash_num_elements(obj->vars));
		zend_hash_copy(Z_ARRVAL_P(ret), obj->vars, (copy_ctor_func_t)zval_add_ref, (void*)&tmp, sizeof(zval*));
	}
	else {
		if (type != BP_VAR_IS) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Undefined property PmtaRecipient::%s", Z_STRVAL_P(member));
		}

		ZVAL_NULL(ret);
	}

	assert(Z_REFCOUNT_P(ret) == 1);
	Z_SET_REFCOUNT_P(ret, 0);
	return ret;
}

static zval* pmtarcpt_read_property(zval* object, zval* member, int type ZLK_DC TSRMLS_DC)
{
	zval tmp;
	zval* ret;
	struct pmtarcpt_object* obj = fetchPmtaRcptObject(object TSRMLS_CC);

	if (obj->obj.ce->type != ZEND_INTERNAL_CLASS) {
		return zend_get_std_object_handlers()->read_property(object, member, type ZLK_CC TSRMLS_CC);
	}

	if (UNEXPECTED(Z_TYPE_P(member) != IS_STRING)) {
		ZVAL_ZVAL(&tmp, member, 1, 0);
		convert_to_string(&tmp);
		member = &tmp;
	}

	ret = pmtarcpt_read_property_internal(obj, member, type TSRMLS_CC);

	if (UNEXPECTED(member == &tmp)) {
		zval_dtor(&tmp);
	}

	return ret;
}

static int pmtarcpt_has_property_internal(struct pmtarcpt_object* obj, zval* member, int has_set_exists TSRMLS_DC)
{
	int retval = 1;

	if (ISSTR(member, "address")) {
		if (1 == has_set_exists) {
			retval = (obj->address && obj->address[0]);
		}
	}
	else if (ISSTR(member, "notify")) {
		if (1 == has_set_exists) {
			retval = (obj->notify != 0);
		}
	}
	else if (ISSTR(member, "variables")) {
		if (1 == has_set_exists) {
			retval = (obj->vars && zend_hash_num_elements(obj->vars));
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
static int pmtarcpt_has_property(zval* object, zval* member, int has_set_exists ZLK_DC TSRMLS_DC)
{
	zval tmp;
	int retval = 1;
	struct pmtarcpt_object* obj = fetchPmtaRcptObject(object TSRMLS_CC);

	if (obj->obj.ce->type != ZEND_INTERNAL_CLASS) {
		return zend_get_std_object_handlers()->has_property(object, member, has_set_exists ZLK_CC TSRMLS_CC);
	}

	if (UNEXPECTED(Z_TYPE_P(member) != IS_STRING)) {
		ZVAL_ZVAL(&tmp, member, 1, 0);
		convert_to_string(&tmp);
		member = &tmp;
	}

	retval = pmtarcpt_has_property_internal(obj, member, has_set_exists TSRMLS_CC);

	if (UNEXPECTED(member == &tmp)) {
		zval_dtor(&tmp);
	}

	return retval;
}

static void pmtarcpt_write_property_internal(struct pmtarcpt_object* obj, zval* member, zval* value TSRMLS_DC)
{
	if (!obj->rcpt) {
		throw_pmta_error(pmta_error_recipient_class, PmtaApiERROR_PHP_API, "Cannot modify locked object", NULL TSRMLS_CC);
		return;
	}

	if (ISSTR(member, "notify")) {
		BOOL res;
		long int v;

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

		res = PmtaRcptSetNotify(obj->rcpt, v);
		if (TRUE == res) {
			obj->notify = v;
		}
		else {
			throw_pmta_error(pmta_error_recipient_class, PmtaRcptGetLastErrorType(obj->rcpt), PmtaRcptGetLastError(obj->rcpt), NULL TSRMLS_CC);
			return;
		}
	}
	else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot set property PmtaRecipient::%s", Z_STRVAL_P(member));
	}
}

static void pmtarcpt_write_property(zval* object, zval* member, zval* value ZLK_DC TSRMLS_DC)
{
	zval tmp;
	struct pmtarcpt_object* obj = fetchPmtaRcptObject(object TSRMLS_CC);

	if (obj->obj.ce->type != ZEND_INTERNAL_CLASS) {
		return zend_get_std_object_handlers()->write_property(object, member, value ZLK_CC TSRMLS_CC);
	}

	if (UNEXPECTED(Z_TYPE_P(member) != IS_STRING)) {
		ZVAL_ZVAL(&tmp, member, 1, 0);
		convert_to_string(&tmp);
		member = &tmp;
	}

	pmtarcpt_write_property_internal(obj, member, value TSRMLS_CC);

	if (UNEXPECTED(member == &tmp)) {
		zval_dtor(&tmp);
	}
}

static HashTable* pmtarcpt_get_properties(zval* object TSRMLS_DC)
{
	struct pmtarcpt_object* obj = fetchPmtaRcptObject(object TSRMLS_CC);
	HashTable* props            = zend_std_get_properties(object TSRMLS_CC);
	zval* zv;
	zval* tmp;

	if (GC_G(gc_active)) {
		return props;
	}

	if (obj->address) {
		ALLOC_ZVAL(zv);
		ZVAL_STRING(zv, obj->address, 1);
		zend_hash_update(props, "address", sizeof("address"), &zv, sizeof(zval*), NULL);
	}

	ALLOC_ZVAL(zv);
	ZVAL_LONG(zv, obj->notify);
	zend_hash_update(props, "notify", sizeof("notify"), &zv, sizeof(zval*), NULL);

	ALLOC_ZVAL(zv);
	array_init_size(zv, zend_hash_num_elements(obj->vars));
	zend_hash_copy(Z_ARRVAL_P(zv), obj->vars, (copy_ctor_func_t)zval_add_ref, (void*)&tmp, sizeof(zval*));
	zend_hash_update(props, "variables", sizeof("variables"), (void*)&zv, sizeof(zval*), NULL);

	return props;
}

static void pmtarcpt_dtor(void* v TSRMLS_DC)
{
	struct pmtarcpt_object* obj = v;

	if (obj->address)  { efree(obj->address);       }
	if (obj->vars)     { FREE_HASHTABLE(obj->vars); }
	if (obj->rcpt)     { PmtaRcptFree(obj->rcpt);   }

	zend_object_std_dtor(&(obj->obj) TSRMLS_CC);
	efree(obj);
}

static zend_object_value pmtarcpt_ctor(zend_class_entry* ce TSRMLS_DC)
{
	struct pmtarcpt_object* obj = ecalloc(1, sizeof(struct pmtarcpt_object));
	zend_object_value retval;

	zend_object_std_init(&(obj->obj), ce TSRMLS_CC);
	retval.handle = zend_objects_store_put(
		obj,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		pmtarcpt_dtor,
		NULL TSRMLS_CC
	);

	retval.handlers = &pmtarcpt_object_handlers;

	return retval;
}

static PHP_METHOD(PmtaRecipient, __construct)
{
	char* address;
	int address_len;
	BOOL result;
	struct pmtarcpt_object* obj;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &address, &address_len)) {
		RETURN_NULL();
	}

	obj = fetchPmtaRcptObject(getThis() TSRMLS_CC);

	obj->rcpt = PmtaRcptAlloc();
	if (!obj->rcpt) {
		throw_pmta_error(pmta_error_recipient_class, PmtaApiERROR_PHP_API, "PmtaRcptAlloc() failed", NULL TSRMLS_CC);
		RETURN_NULL();
	}

	result = PmtaRcptInit(obj->rcpt, address);
	if (FALSE == result) {
		throw_pmta_error(pmta_error_recipient_class, PmtaRcptGetLastErrorType(obj->rcpt), PmtaRcptGetLastError(obj->rcpt), NULL TSRMLS_CC);
	}

	obj->address = estrndup(address, address_len);
	obj->notify  = PmtaRcptNOTIFY_NEVER;

	ALLOC_HASHTABLE(obj->vars);
	zend_hash_init(obj->vars, 8, NULL, ZVAL_PTR_DTOR, 0);
}

static PHP_METHOD(PmtaRecipient, __get)
{
	zval* property;
	zval* retval;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &property)) {
		RETURN_NULL();
	}

	retval = pmtarcpt_read_property_internal(fetchPmtaRcptObject(getThis() TSRMLS_CC), property, BP_VAR_R TSRMLS_CC);
	Z_ADDREF_P(retval);
	RETURN_ZVAL(retval, 1, 1);
}

static PHP_METHOD(PmtaRecipient, __isset)
{
	zval* property;
	int retval;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &property)) {
		RETURN_NULL();
	}

	retval = pmtarcpt_has_property_internal(fetchPmtaRcptObject(getThis() TSRMLS_CC), property, 1 TSRMLS_CC);
	RETURN_BOOL(retval);
}

static PHP_METHOD(PmtaRecipient, __set)
{
	zval* property;
	zval* value;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &property, &value)) {
		RETURN_NULL();
	}

	pmtarcpt_write_property_internal(fetchPmtaRcptObject(getThis() TSRMLS_CC), property, value TSRMLS_CC);
}

static PHP_METHOD(PmtaRecipient, defineVariable)
{
	struct pmtarcpt_object* obj;
	char* name;
	int name_len;
	char* value;
	int value_len;
	BOOL res;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &name, &name_len, &value, &value_len)) {
		RETURN_NULL();
	}

	obj = fetchPmtaRcptObject(getThis() TSRMLS_CC);

	if (!obj->rcpt) {
		throw_pmta_error(pmta_error_recipient_class, PmtaApiERROR_PHP_API, "Cannot modify locked object", NULL TSRMLS_CC);
	}

	res = PmtaRcptDefineVariable(obj->rcpt, name, value);
	if (TRUE == res) {
		zval* tmp;
		ALLOC_ZVAL(tmp);
		ZVAL_STRINGL(tmp, value, value_len, 1);
		zend_symtable_update(obj->vars, name, name_len + 1, (void*)tmp, sizeof(zval*), NULL);
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static PHP_METHOD(PmtaRecipient, getLastError)
{
	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_NULL();
	}

	if (return_value_used) {
		struct pmtarcpt_object* obj = fetchPmtaRcptObject(getThis() TSRMLS_CC);
		throw_pmta_error(pmta_error_recipient_class, PmtaRcptGetLastErrorType(obj->rcpt), PmtaRcptGetLastError(obj->rcpt), &return_value TSRMLS_CC);
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
	PHP_ME(PmtaRecipient, defineVariable,   arginfo_defvar,    ZEND_ACC_PUBLIC)
	PHP_ME(PmtaRecipient, getLastError,     arginfo_empty,     ZEND_ACC_PUBLIC)
	PHP_FE_END
};

void pmtarcpt_register_class(TSRMLS_D)
{
	zend_class_entry e;

	INIT_CLASS_ENTRY(e, "PmtaRecipient", pmta_rcpt_class_methods);

	pmta_rcpt_class = zend_register_internal_class(&e TSRMLS_CC);

	pmta_rcpt_class->create_object = pmtarcpt_ctor;
	pmta_rcpt_class->serialize     = zend_class_serialize_deny;
	pmta_rcpt_class->unserialize   = zend_class_unserialize_deny;

	pmtarcpt_object_handlers = *zend_get_std_object_handlers();
	pmtarcpt_object_handlers.clone_obj            = NULL;
	pmtarcpt_object_handlers.read_property        = pmtarcpt_read_property;
	pmtarcpt_object_handlers.has_property         = pmtarcpt_has_property;
	pmtarcpt_object_handlers.write_property       = pmtarcpt_write_property;
	pmtarcpt_object_handlers.get_property_ptr_ptr = NULL;
	pmtarcpt_object_handlers.get_properties       = pmtarcpt_get_properties;

	zend_declare_class_constant_long(pmta_rcpt_class, ZEND_STRL("NOTIFY_NEVER"),   PmtaRcptNOTIFY_NEVER   TSRMLS_CC);
	zend_declare_class_constant_long(pmta_rcpt_class, ZEND_STRL("NOTIFY_SUCCESS"), PmtaRcptNOTIFY_SUCCESS TSRMLS_CC);
	zend_declare_class_constant_long(pmta_rcpt_class, ZEND_STRL("NOTIFY_FAILURE"), PmtaRcptNOTIFY_FAILURE TSRMLS_CC);
	zend_declare_class_constant_long(pmta_rcpt_class, ZEND_STRL("NOTIFY_DELAY"),   PmtaRcptNOTIFY_DELAY   TSRMLS_CC);
	zend_declare_class_constant_long(pmta_rcpt_class, ZEND_STRL("NOTIFY_ALWAYS"),  PmtaRcptNOTIFY_SUCCESS | PmtaRcptNOTIFY_FAILURE | PmtaRcptNOTIFY_DELAY TSRMLS_CC);
}
