/**
 * @file pmta_connection.c
 * @date 29.09.2010
 * @author Vladimir Kolesnikov <vladimir@extrememember.com>
 * @brief @c PmtaConnection class (implementation)
 * @details @c PmtaConnection class implementation
<PRE>
final class PmtaConnection
{
	const LOCAL_SERVER = "127.0.0.1";
	const DEFAULT_PORT = 25;

	private $connection;

	private $server;
	private $port;
	private $username;
	private $password;

	public function __construct($server = '127.0.0.1', $port = 25, $username = null, $password = null)
	{
		$this->connection = PmtaConnAlloc();

		if ($username && $password) {
			$res = PmtaConnConnectRemoteAuth($this->connection, $server, $port, $username, $password);
		}
		else {
			$res = PmtaConnConnectRemote($this->connection, $server, $port);
		}

		if (!$res) {
			throw new PmtaErrorConnection(PmtaConnGetLastError($this->connection), PmtaConnGetLastErrorType($this->connection));
		}

		$this->server   = $server;
		$this->port     = $port;
		$this->username = $username;
		$this->password = $password;
	}

	public function __destruct()
	{
		PmtaConnFree($this->connection);
	}

	public function getLastError()
	{
		return new PmtaErrorConnection(PmtaConnGetLastError($this->connection), PmtaConnGetLastErrorType($this->connection));
	}

	public function submitMessage(PmtaMessage $message)
	{
		return PmtaConnSubmit($this->connection, $message);
	}

	public function __get($property)
	{
		static $properties = array('server', 'port', 'username', 'password');
		if (isset($properties[$property])) {
			return $properties[$property];
		}

		trigger_error("Undefined property PmtaConnection::{$property}", E_WARNING);
	}

	public function __isset($property)
	{
		static $properties = array('server', 'port', 'username', 'password');
		return array_key_exists($properties, $property);
	}

	private function __clone() {}
}
</PRE>
*/

#include "pmta_connection.h"
#include "pmta_error.h"
#include "pmta_message.h"
#include "pmta_common.h"
#include <submitter/PmtaConn.h>

static zend_object_handlers pmtaconn_object_handlers;

struct pmtaconn_object {
	zend_object obj;
	PmtaConn conn;
	char* server;
	char* username;
	char* password;
	int port;
};

static inline struct pmtaconn_object* fetchPmtaConnObject(zval* zobj TSRMLS_DC)
{
	return (struct pmtaconn_object*)zend_objects_get_address(zobj TSRMLS_CC);
}

static zval* pmtaconn_read_property(zval* object, zval* member, int type ZLK TSRMLS_DC)
{
	zval tmp;
	zval* ret;
	struct pmtaconn_object* obj = fetchPmtaConnObject(object TSRMLS_CC);

	if (UNEXPECTED(Z_TYPE_P(member) != IS_STRING)) {
		ZVAL_ZVAL(&tmp, member, 1, 0);
		convert_to_string(&tmp);
		member = &tmp;
	}

	MAKE_STD_ZVAL(ret);

	if (ISSTR(member, "server")) {
		ZVAL_STRING(ret, obj->server, 1);
	}
	else if (ISSTR(member, "username")) {
		ZVAL_STRING(ret, obj->username, 1);
	}
	else if (ISSTR(member, "password")) {
		ZVAL_STRING(ret, obj->password, 1);
	}
	else if (ISSTR(member, "port")) {
		ZVAL_LONG(ret, obj->port);
	}
	else {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Undefined property %s", Z_STRVAL_P(member));
		ZVAL_NULL(ret);
	}

	Z_SET_REFCOUNT_P(ret, 0);

	if (UNEXPECTED(member == &tmp)) {
		zval_dtor(&tmp);
	}

	return ret;
}

static void pmtaconn_write_property(zval* object, zval* member, zval* value ZLK TSRMLS_DC)
{
	zval tmp;
	struct pmtaconn_object* obj = fetchPmtaConnObject(object TSRMLS_CC);

	if (UNEXPECTED(Z_TYPE_P(member) != IS_STRING)) {
		ZVAL_ZVAL(&tmp, member, 1, 0);
		convert_to_string(&tmp);
		member = &tmp;
	}

	if (ISSTR(member, "server")) {
		if (obj->server) {
			efree(obj->server);
		}

		if (Z_TYPE_P(value) == IS_STRING) {
			obj->server = estrndup(Z_STRVAL_P(value), Z_STRLEN_P(value));
		}
		else {
			zval str;
			ZVAL_ZVAL(&str, value, 1, 0);
			convert_to_string(&str);
			obj->server = Z_STRVAL(str);
			/* do not call the destructor */
		}
	}
	else if (ISSTR(member, "username")) {
		if (obj->username) {
			efree(obj->username);
		}

		if (Z_TYPE_P(value) == IS_STRING) {
			obj->username = estrndup(Z_STRVAL_P(value), Z_STRLEN_P(value));
		}
		else {
			zval str;
			ZVAL_ZVAL(&str, value, 1, 0);
			convert_to_string(&str);
			obj->username = Z_STRVAL(str);
			/* do not call the destructor */
		}
	}
	else if (ISSTR(member, "password")) {
		if (obj->password) {
			efree(obj->password);
		}

		if (Z_TYPE_P(value) == IS_STRING) {
			obj->password = estrndup(Z_STRVAL_P(value), Z_STRLEN_P(value));
		}
		else {
			zval str;
			ZVAL_ZVAL(&str, value, 1, 0);
			convert_to_string(&str);
			obj->password = Z_STRVAL(str);
			/* do not call the destructor */
		}
	}
	else if (ISSTR(member, "port")) {
		if (Z_TYPE_P(value) == IS_LONG) {
			obj->port = Z_DVAL_P(value);
		}
		else {
			zval lval;
			ZVAL_ZVAL(&lval, value, 1, 0);
			convert_to_long(&lval);
			obj->port = Z_DVAL(lval);
			zval_dtor(&lval);
		}
	}
	else {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Undefined property %s", Z_STRVAL_P(member));
	}

	if (UNEXPECTED(member == &tmp)) {
		zval_dtor(&tmp);
	}
}

static int pmtaconn_has_property(zval* object, zval* member, int has_set_exists ZLK TSRMLS_DC)
{
	zval tmp;
	int retval = 1;
	struct pmtaconn_object* obj = fetchPmtaConnObject(object TSRMLS_CC);

	if (UNEXPECTED(Z_TYPE_P(member) != IS_STRING)) {
		ZVAL_ZVAL(&tmp, member, 1, 0);
		convert_to_string(&tmp);
		member = &tmp;
	}

	if (ISSTR(member, "server")) {
		if (1 == has_set_exists) { /* set */
			retval = (obj->server != NULL);
		}
	}
	else if (ISSTR(member, "username")) {
		if (1 == has_set_exists) { /* set */
			retval = (obj->username != NULL);
		}
	}
	else if (ISSTR(member, "password")) {
		if (1 == has_set_exists) { /* set */
			retval = (obj->password != NULL);
		}
	}
	else if (ISSTR(member, "port")) {
		if (1 == has_set_exists) { /* set */
			retval = (obj->port > 0);
		}
	}
	else {
		retval = 0;
	}

	if (UNEXPECTED(member == &tmp)) {
		zval_dtor(&tmp);
	}

	return retval;
}

static void pmtaconn_unset_property(zval* object, zval* member ZLK TSRMLS_DC)
{
	zval tmp;
	struct pmtaconn_object* obj = fetchPmtaConnObject(object TSRMLS_CC);

	if (UNEXPECTED(Z_TYPE_P(member) != IS_STRING)) {
		ZVAL_ZVAL(&tmp, member, 1, 0);
		convert_to_string(&tmp);
		member = &tmp;
	}

	if (ISSTR(member, "server")) {
		if (obj->server) {
			efree(obj->server);
			obj->server = NULL;
		}
	}
	else if (ISSTR(member, "username")) {
		if (obj->username) {
			efree(obj->username);
			obj->username = NULL;
		}
	}
	else if (ISSTR(member, "password")) {
		if (obj->password) {
			efree(obj->password);
			obj->password = NULL;
		}
	}
	else if (ISSTR(member, "port")) {
		obj->port = 0;
	}

	if (UNEXPECTED(member == &tmp)) {
		zval_dtor(&tmp);
	}
}

static void pmtaconn_dtor(void* v TSRMLS_DC)
{
	struct pmtaconn_object* obj = v;

	if (obj->server)   { efree(obj->server);      }
	if (obj->username) { efree(obj->username);    }
	if (obj->password) { efree(obj->password);    }
	if (obj->conn)     { PmtaConnFree(obj->conn); }

	zend_object_std_dtor(&(obj->obj) TSRMLS_CC);
	efree(obj);
}

static zend_object_value pmtaconn_ctor(zend_class_entry* ce TSRMLS_DC)
{
	struct pmtaconn_object* obj = ecalloc(1, sizeof(struct pmtaconn_object));
	zend_object_value retval;

	zend_object_std_init(&(obj->obj), ce TSRMLS_CC);
	retval.handle = zend_objects_store_put(
		obj,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		pmtaconn_dtor,
		NULL TSRMLS_CC
	);

	retval.handlers = &pmtaconn_object_handlers;

	return retval;
}

/**
 * @brief Class properties
 */
static const struct props pmtaconn_properties[] = {
	{ PHPPMTA_SL("connection"), PHPPMTA_SL("/**\n * PmtaConn — connection resource\n *\n * @var resource\n */") },
	{ PHPPMTA_SL("server"),     PHPPMTA_SL("/**\n * Server we are connected to\n *\n  * @var string\n */") },
	{ PHPPMTA_SL("port"),       PHPPMTA_SL("/**\n * Server port we are connected to\n *\n * @var int\n */") },
	{ PHPPMTA_SL("username"),   PHPPMTA_SL("/**\n * Username to log in with\n *\n * @var string\n */") },
	{ PHPPMTA_SL("password"),   PHPPMTA_SL("/**\n * Password to log in with\n *\n * @var string\n */") }
};

/**
 * @brief public function __construct($server = '127.0.0.1', $port = 25, $username = null, $password = null);
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 *
 * Class constructor. Allocates a PmtaConn object and connects to the server. Throws PmtaErrorConnection on failure
 */
static PHP_METHOD(PmtaConnection, __construct)
{
	char* server     = NULL;
	int server_len   = 0;
	long int port    = 0;
	char* username   = NULL;
	int username_len = 0;
	char* password   = NULL;
	int password_len;
	BOOL result;
	struct pmtaconn_object* obj;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|slss", &server, &server_len, &port, &username, &username_len, &password, &password_len)) {
		RETURN_NULL();
	}

	obj = fetchPmtaConnObject(getThis() TSRMLS_CC);

	obj->conn = PmtaConnAlloc();
	if (!obj->conn) {
		throw_pmta_error(pmta_error_connection_class, PmtaApiERROR_PHP_API, "PmtaConnAlloc() failed", NULL TSRMLS_CC);
		RETURN_NULL();
	}

	if (!server) {
		server = PMTA_G(server);
		if (!server || !*server) {
			server = "127.0.0.1";
		}
	}

	if (!port) {
		port = PMTA_G(port);
		if (!port) {
			port = 25;
		}
	}

	if (!username) {
		username = PMTA_G(username);
		if (username && !*username) {
			username = NULL;
		}
	}

	if (!password) {
		password = PMTA_G(password);
		if (password && !*password) {
			password = NULL;
		}
	}

	if (username && password) {
		result = PmtaConnConnectRemoteAuth(obj->conn, server, port, username, password);
	}
	else {
		result = PmtaConnConnectRemote(obj->conn, server, port);
	}

	if (FALSE == result) {
		throw_pmta_error(pmta_error_connection_class, PmtaConnGetLastErrorType(obj->conn), PmtaConnGetLastError(obj->conn), NULL TSRMLS_CC);
	}
	else {
		obj->server = estrdup(server);
		obj->port   = port;
		if (username && password) {
			obj->username = estrdup(username);
			obj->password = estrdup(password);
		}
	}
}

/**
 * @brief public function __get($property);
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 */
static PHP_METHOD(PmtaConnection, __get)
{
	generic_get(pmtaconn_properties, PHPPMTA_NELEMS(pmtaconn_properties), pmta_conn_class, "PmtaConnection", INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

/**
 * @brief public function __isset($property);
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 */
static PHP_METHOD(PmtaConnection, __isset)
{
	generic_isset(pmtaconn_properties, PHPPMTA_NELEMS(pmtaconn_properties), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}


/**
 * @brief public function submitMessage(PmtaMessage $message);
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 *
 * Submits the message to PowerMTA
 */
static PHP_METHOD(PmtaConnection, submitMessage)
{
	struct pmtaconn_object* obj;
	PmtaMsg msg;
	zval* message;
	BOOL res;
	zend_bool exceptions = PMTA_G(use_exceptions);

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &message, pmta_msg_class)) {
		RETURN_NULL();
	}

	obj = fetchPmtaConnObject(getThis() TSRMLS_CC);

	msg = getMessage(message TSRMLS_CC);
	ZEND_VERIFY_RESOURCE(msg);

	res = PmtaConnSubmit(obj->conn, msg);
	if (TRUE == res) {
		RETURN_TRUE;
	}

	if (exceptions) {
		throw_pmta_error(pmta_error_connection_class, PmtaConnGetLastErrorType(obj->conn), PmtaConnGetLastError(obj->conn), NULL TSRMLS_CC);
		RETURN_NULL();
	}

	RETURN_FALSE;
}

/**
 * @brief public function getLastError();
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 *
 * Returns the last connection error
 */
static PHP_METHOD(PmtaConnection, getLastError)
{
	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_NULL();
	}

	if (return_value_used) {
		struct pmtaconn_object* obj = fetchPmtaConnObject(getThis() TSRMLS_CC);
		throw_pmta_error(pmta_error_connection_class, PmtaConnGetLastErrorType(obj->conn), PmtaConnGetLastError(obj->conn), &return_value TSRMLS_CC);
	}
}

/**
 * @brief arginfo for @c __construct()
 */
PHPPMTA_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_construct, 0, 0, 0)
	ZEND_ARG_INFO(0, server)
	ZEND_ARG_INFO(0, port)
	ZEND_ARG_INFO(0, username)
	ZEND_ARG_INFO(0, password)
ZEND_END_ARG_INFO()

/**
 * @brief arginfo for @c submitMessage()
 */
PHPPMTA_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_submit, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, message, PmtaMessage, 0)
ZEND_END_ARG_INFO()

/**
 * @brief @c PmtaConnection class methods
 */
static
#if ZEND_MODULE_API_NO > 20060613
const
#endif
zend_function_entry pmta_conn_class_methods[] = {
	PHP_ME(PmtaConnection, __construct,      arginfo_construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(PmtaConnection, __get,            arginfo_get,       ZEND_ACC_PUBLIC)
	PHP_ME(PmtaConnection, __isset,          arginfo_get,       ZEND_ACC_PUBLIC)
	PHP_ME(PmtaConnection, submitMessage,    arginfo_submit,    ZEND_ACC_PUBLIC)
	PHP_ME(PmtaConnection, getLastError,     arginfo_empty,     ZEND_ACC_PUBLIC)
	PHP_FE_END
};

void pmtaconn_register_class(TSRMLS_D)
{
	zend_class_entry e;

	INIT_CLASS_ENTRY(e, "PmtaConnection", pmta_conn_class_methods);

	pmta_conn_class = zend_register_internal_class(&e TSRMLS_CC);

	pmta_conn_class->create_object = pmtaconn_ctor;
	pmta_conn_class->serialize     = zend_class_serialize_deny;
	pmta_conn_class->unserialize   = zend_class_unserialize_deny;

	pmtaconn_object_handlers = *zend_get_std_object_handlers();
	pmtaconn_object_handlers.clone_obj            = NULL;
	pmtaconn_object_handlers.read_property        = pmtaconn_read_property;
	pmtaconn_object_handlers.write_property       = pmtaconn_write_property;
	pmtaconn_object_handlers.has_property         = pmtaconn_has_property;
	pmtaconn_object_handlers.unset_property       = pmtaconn_unset_property;
	pmtaconn_object_handlers.get_property_ptr_ptr = NULL;

	zend_declare_class_constant_stringl(pmta_conn_class, PHPPMTA_SL("LOCAL_SERVER"), PHPPMTA_SL("127.0.0.1") TSRMLS_CC);
	zend_declare_class_constant_long(pmta_conn_class, PHPPMTA_SL("DEFAULT_PORT"), 25 TSRMLS_CC);
}
