/**
 * @file pmta_connection.c
 * @date 29.09.2010
 * @author Vladimir Kolesnikov <vladimir@extrememember.com>
 * @brief @c PmtaConnection class (implementation)
 * @details @c PmtaConnection class implementation
@code{.php}
class PmtaConnection
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
@endcode
*/

#include "pmta_connection.h"
#include "pmta_error.h"
#include "pmta_message.h"
#include "pmta_common.h"
#include <submitter/PmtaConn.h>

/**
 * @brief PmtaConnection object handlers
 */
static zend_object_handlers pmtaconn_object_handlers;

typedef struct _pmtaconn_object {
	zend_object obj; /**< zend object data */
	PmtaConn conn;   /**< PMTA Connection handle */
	char* server;    /**< PowerMTA server */
	char* username;  /**< Username to authenticate with */
	char* password;  /**< Password to authenticate with */
	int port;        /**< Server port */
} pmtaconn_object;

/**
 * @brief Fetches @c pmtaconn_object
 * @see pmtaconn_object
 * @param zobj @c PmtaConnection instance
 * @return pmtaconn_object associated with @a zobj
 * @pre <tt>Z_TYPE_P(zobj) == IS_OBJECT && instanceof_function(Z_OBJCE_P(zobj), pmta_conn_class TSRMLS_CC)</tt>
 */
static inline pmtaconn_object* fetchPmtaConnObject(zval* zobj TSRMLS_DC)
{
	return (pmtaconn_object*)zend_objects_get_address(zobj TSRMLS_CC);
}

/**
 * @brief Internal implementation of @c __get() method
 * @see pmtaconn_object
 * @param obj @c pmtaconn_object
 * @param member Property to read
 * @param type If @c BP_VAR_IS, error messages will be suppressed
 * @return Property value
 * @exception @c E_WARNING if @c member is not a valid property and @a type != @c BP_VAR_IS
 * @pre <tt>Z_TYPE_P(member) == IS_STRING</tt>
 * @note Reference count of the result value will be 0
 */
static zval* pmtaconn_read_property_internal(pmtaconn_object* obj, zval* member, int type)
{
	zval* ret;
	ALLOC_INIT_ZVAL(ret);

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
		if (type != BP_VAR_IS) {
			zend_error(E_WARNING, "Undefined property PmtaConnection::%s", Z_STRVAL_P(member));
		}

		ZVAL_NULL(ret);
	}

	Z_SET_REFCOUNT_P(ret, 0);
	return ret;
}

/**
 * @brief @c read_property handler
 * @param object @c PmtaConnection instance
 * @param member Property to read
 * @param type Read type (@c BP_VAR_R, @c BP_VAR_IS)
 * @param key Zend literal associated with @a member
 * @return Property value
 * @note Reference count of the result is not incremented
 * @pre <tt>Z_TYPE_P(object) == IS_OBJECT && instanceof_function(Z_OBJCE_P(object), pmta_conn_class TSRMLS_CC)</tt>
 */
static zval* pmtaconn_read_property(zval* object, zval* member, int type ZLK_DC TSRMLS_DC)
{
	zval tmp;
	zval* ret;
	pmtaconn_object* obj = fetchPmtaConnObject(object TSRMLS_CC);

	if (obj->obj.ce->type != ZEND_INTERNAL_CLASS) {
		return zend_get_std_object_handlers()->read_property(object, member, type ZLK_CC TSRMLS_CC);
	}

	if (UNEXPECTED(Z_TYPE_P(member) != IS_STRING)) {
		ZVAL_ZVAL(&tmp, member, 1, 0);
		convert_to_string(&tmp);
		member = &tmp;
	}

	ret = pmtaconn_read_property_internal(obj, member, type);

	if (UNEXPECTED(member == &tmp)) {
		zval_dtor(&tmp);
	}

	return ret;
}

/**
 * @brief Internal implementation of @c __isset() method
 * @see pmtaconn_object
 * @see pmtaconn_has_property
 * @param obj @c pmtaconn_object
 * @param member Property to read
 * @param has_set_exists Additional checks
 * @return Whether property @a member exists and satisfies @a has_set_exists criterion
 * @retval 1 Yes
 * @retval 0 No
 * @pre <tt>Z_TYPE_P(member) == IS_STRING</tt>
 */
static int pmtaconn_has_property_internal(pmtaconn_object* obj, zval* member, int has_set_exists)
{
	int retval = 1;

	if (ISSTR(member, "server")) {
		if (0 == has_set_exists) {
			retval = (obj->server != NULL);
		}
		else if (1 == has_set_exists) {
			retval = (obj->server && obj->server[0]);
		}
	}
	else if (ISSTR(member, "username")) {
		if (0 == has_set_exists) {
			retval = (obj->username != NULL);
		}
		else if (1 == has_set_exists) {
			retval = (obj->username && obj->username[0]);
		}
	}
	else if (ISSTR(member, "password")) {
		if (0 == has_set_exists) {
			retval = (obj->password != NULL);
		}
		else if (1 == has_set_exists) {
			retval = (obj->password && obj->password[0]);
		}
	}
	else if (ISSTR(member, "port")) {
		if (1 == has_set_exists) {
			retval = (obj->port > 0);
		}
	}
	else {
		retval = 0;
	}

	return retval;
}

/**
 * @param object @c PmtaConnection instance
 * @param member Property
 * @param has_set_exists Existence criterion
 * @param tsrm_ls Internally used by Zend
 * @return Whether property @a member exists and satisfies @a has_set_exists criterion
 * @retval 1 Yes
 * @retval 0 No
 * @pre <tt>Z_TYPE_P(object) == IS_OBJECT && instanceof_function(Z_OBJCE_P(object), pmta_conn_class TSRMLS_CC)</tt>
 *
 * Used to check if a property @a member of the object @a object exists.
 * @c has_set_exists can be one of the following:
 * @arg 0 (has) whether property exists and is not NULL
 * @arg 1 (set) whether property exists and is true
 * @arg 2 (exists) whether property exists
 */
static int pmtaconn_has_property(zval* object, zval* member, int has_set_exists ZLK_DC TSRMLS_DC)
{
	zval tmp;
	int retval = 1;
	pmtaconn_object* obj = fetchPmtaConnObject(object TSRMLS_CC);

	if (obj->obj.ce->type != ZEND_INTERNAL_CLASS) {
		return zend_get_std_object_handlers()->has_property(object, member, has_set_exists ZLK_CC TSRMLS_CC);
	}

	if (UNEXPECTED(Z_TYPE_P(member) != IS_STRING)) {
		ZVAL_ZVAL(&tmp, member, 1, 0);
		convert_to_string(&tmp);
		member = &tmp;
	}

	retval = pmtaconn_has_property_internal(obj, member, has_set_exists);

	if (UNEXPECTED(member == &tmp)) {
		zval_dtor(&tmp);
	}

	return retval;
}

/**
 * @brief @c get_properties handler
 * @param object @c PmtaConnection instance
 * @param tsrm_ls Internally used by Zend
 * @return Hash table with properties of @a object
 * @pre <tt>Z_TYPE_P(object) == IS_OBJECT && instanceof_function(Z_OBJCE_P(object), pmta_conn_class TSRMLS_CC)</tt>
 */
static HashTable* pmtaconn_get_properties(zval* object TSRMLS_DC)
{
	pmtaconn_object* obj = fetchPmtaConnObject(object TSRMLS_CC);
	HashTable* props     = zend_std_get_properties(object TSRMLS_CC);
	zval* zv;

	if (obj->server) {
		ALLOC_INIT_ZVAL(zv);
		ZVAL_STRING(zv, obj->server, 1);
		zend_hash_update(props, "server", sizeof("server"), &zv, sizeof(zval*), NULL);
	}

	if (obj->username) {
		ALLOC_INIT_ZVAL(zv);
		ZVAL_STRING(zv, obj->username, 1);
		zend_hash_update(props, "username", sizeof("username"), &zv, sizeof(zval*), NULL);
	}

	if (obj->password) {
		ALLOC_INIT_ZVAL(zv);
		ZVAL_STRING(zv, obj->password, 1);
		zend_hash_update(props, "password", sizeof("password"), &zv, sizeof(zval*), NULL);
	}

	ALLOC_INIT_ZVAL(zv);
	ZVAL_LONG(zv, obj->port);
	zend_hash_update(props, "port", sizeof("port"), &zv, sizeof(zval*), NULL);

	return props;
}

/**
 * @brief @c PmtaConnection destructor
 * @param v @c pmtaconn_object
 * @param tsrm_ls Internally used by Zend
 * @details Frees all memory allocated for @c pmtaconn_object and its members
 */
static void pmtaconn_dtor(void* v TSRMLS_DC)
{
	pmtaconn_object* obj = v;

	if (obj->server)   { efree(obj->server);      }
	if (obj->username) { efree(obj->username);    }
	if (obj->password) { efree(obj->password);    }
	if (obj->conn)     { PmtaConnFree(obj->conn); }

	zend_object_std_dtor(&(obj->obj) TSRMLS_CC);
	efree(obj);
}

/**
 * @brief @c PmtaConnection constructor
 * @param ce Class Entry for @c PmtaConnection
 * @param tsrm_ls Internally used by Zend
 * @return Zend Object Value
 * @details Allocates memory for @c pmtaconn_object and registers the destructor
 */
static zend_object_value pmtaconn_ctor(zend_class_entry* ce TSRMLS_DC)
{
	pmtaconn_object* obj = ecalloc(1, sizeof(pmtaconn_object));
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
	int server_len;
	long int port    = 0;
	char* username   = NULL;
	int username_len;
	char* password   = NULL;
	int password_len;
	BOOL result;
	pmtaconn_object* obj;

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
	zval* property;
	zval* retval;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &property)) {
		RETURN_NULL();
	}

	retval = pmtaconn_read_property_internal(fetchPmtaConnObject(getThis() TSRMLS_CC), property, BP_VAR_R TSRMLS_CC);
	Z_ADDREF_P(retval);
	RETURN_ZVAL(retval, 1, 1);
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
	zval* property;
	int retval;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &property)) {
		RETURN_NULL();
	}

	retval = pmtaconn_has_property_internal(fetchPmtaConnObject(getThis() TSRMLS_CC), property, 1 TSRMLS_CC);
	RETURN_BOOL(retval);
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
	pmtaconn_object* obj;
	PmtaMsg msg;
	zval* message;
	BOOL res;
	zend_bool exceptions = PMTA_G(use_exceptions);

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &message, pmta_msg_class)) {
		RETURN_NULL();
	}

	obj = fetchPmtaConnObject(getThis() TSRMLS_CC);
	msg = getMessage(message TSRMLS_CC);
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
		pmtaconn_object* obj = fetchPmtaConnObject(getThis() TSRMLS_CC);
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

/**
 * @brief Registers @c PmtaConnection class with Zend
 * @param tsrm_ls Internally used by Zend
 */
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
	pmtaconn_object_handlers.has_property         = pmtaconn_has_property;
	pmtaconn_object_handlers.get_property_ptr_ptr = NULL;
	pmtaconn_object_handlers.get_properties       = pmtaconn_get_properties;

	zend_declare_class_constant_stringl(pmta_conn_class, ZEND_STRL("LOCAL_SERVER"), ZEND_STRL("127.0.0.1") TSRMLS_CC);
	zend_declare_class_constant_long(pmta_conn_class, ZEND_STRL("DEFAULT_PORT"), 25 TSRMLS_CC);
}
