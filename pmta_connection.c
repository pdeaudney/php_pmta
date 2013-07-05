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

/**
 * @brief Returns connection resource
 * @param object PmtaConnection object
 * @return Connection resource or @c NULL on failure
 */
static PmtaConn getConnection(zval* object TSRMLS_DC)
{
	PmtaConn result;
	zval* rv = zend_read_property(pmta_conn_class, object, PHPPMTA_SL("connection"), 0 TSRMLS_CC);
	ZEND_FETCH_RESOURCE_NO_RETURN(result, PmtaConn, &rv, -1, PMTA_CONNECTION_RES_NAME, le_pmta_connection);
	return result;
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
	zval* c;
	PmtaConn conn;
	char* server     = NULL;
	int server_len   = 0;
	long int port    = 0;
	char* username   = NULL;
	int username_len = 0;
	char* password   = NULL;
	int password_len;
	BOOL result;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|slss", &server, &server_len, &port, &username, &username_len, &password, &password_len)) {
		RETURN_NULL();
	}

	conn = PmtaConnAlloc();
	if (!conn) {
		throw_pmta_error(pmta_error_connection_class, PmtaApiERROR_PHP_API, "PmtaConnAlloc() failed", NULL TSRMLS_CC);
		RETURN_NULL();
	}

	MAKE_STD_ZVAL(c);
	ZEND_REGISTER_RESOURCE(c, conn, le_pmta_connection);

	zend_update_property(pmta_conn_class, getThis(), PHPPMTA_SL("connection"), c TSRMLS_CC);
	zval_ptr_dtor(&c);

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
		result = PmtaConnConnectRemoteAuth(conn, server, port, username, password);
	}
	else {
		result = PmtaConnConnectRemote(conn, server, port);
	}

	if (FALSE == result) {
		throw_pmta_error(pmta_error_connection_class, PmtaConnGetLastErrorType(conn), PmtaConnGetLastError(conn), NULL TSRMLS_CC);
	}
	else {
		zend_update_property_string(pmta_conn_class, getThis(), PHPPMTA_SL("server"), (server ? server : "127.0.0.1") TSRMLS_CC);
		zend_update_property_long(pmta_conn_class, getThis(), PHPPMTA_SL("port"), port TSRMLS_CC);

		if (username && password) {
			zend_update_property_string(pmta_conn_class, getThis(), PHPPMTA_SL("username"), username TSRMLS_CC);
			zend_update_property_string(pmta_conn_class, getThis(), PHPPMTA_SL("password"), password TSRMLS_CC);
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
 * @brief private function __clone();
 * @param ht Internally used by Zend (number of arguments)
 * @param return_value Internally used by Zend (return value)
 * @param return_value_ptr Internally used by Zend
 * @param this_ptr Internally used by Zend (@c $this)
 * @param return_value_used Internally used by Zend (whether the return value is used)
 * @param tsrm_ls Internally used by Zend
 *
 * Private method to disable connection cloning
 */
static PHP_METHOD(PmtaConnection, __clone)
{
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
	PmtaConn conn = getConnection(getThis() TSRMLS_CC);
	PmtaMsg msg;
	zval* message;
	BOOL res;
	zend_bool exceptions = PMTA_G(use_exceptions);

	ZEND_VERIFY_RESOURCE(conn);

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &message, pmta_msg_class)) {
		RETURN_NULL();
	}

	msg = getMessage(message TSRMLS_CC);
	ZEND_VERIFY_RESOURCE(msg);

	res = PmtaConnSubmit(conn, msg);
	if (TRUE == res) {
		RETURN_TRUE;
	}

	if (exceptions) {
		throw_pmta_error(pmta_error_connection_class, PmtaConnGetLastErrorType(conn), PmtaConnGetLastError(conn), NULL TSRMLS_CC);
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
	if (return_value_used) {
		PmtaConn conn = getConnection(getThis() TSRMLS_CC);
		ZEND_VERIFY_RESOURCE(conn);

		throw_pmta_error(pmta_error_connection_class, PmtaConnGetLastErrorType(conn), PmtaConnGetLastError(conn), &return_value TSRMLS_CC);
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
	PHP_ME(PmtaConnection, __clone,          arginfo_empty,     ZEND_ACC_PUBLIC | ZEND_ACC_CLONE)
	PHP_ME(PmtaConnection, submitMessage,    arginfo_submit,    ZEND_ACC_PUBLIC)
	PHP_ME(PmtaConnection, getLastError,     arginfo_empty,     ZEND_ACC_PUBLIC)

	{ NULL, NULL, NULL, 0, 0 }
};

ZEND_RSRC_DTOR_FUNC(pmta_connection_dtor)
{
	PmtaConn connection = (PmtaConn)rsrc->ptr;
	if (connection) {
		PmtaConnFree(connection);
	}
}

void pmtaconn_register_class(TSRMLS_D)
{
	zend_class_entry e;
	size_t i;

	INIT_CLASS_ENTRY(e, "PmtaConnection", pmta_conn_class_methods);

	pmta_conn_class            = zend_register_internal_class(&e TSRMLS_CC);
	pmta_conn_class->ce_flags |= ZEND_ACC_FINAL_CLASS;

	for (i=0; i<PHPPMTA_NELEMS(pmtaconn_properties); ++i) {
		zval* property;
#ifdef ALLOC_PERMANENT_ZVAL
		ALLOC_PERMANENT_ZVAL(property);
#else
		property = malloc(sizeof(zval));
#endif
		INIT_ZVAL(*property);

		zend_declare_property_ex(pmta_conn_class, pmtaconn_properties[i].name, pmtaconn_properties[i].len, property, ZEND_ACC_PRIVATE, pmtaconn_properties[i].comment, pmtaconn_properties[i].comment_len TSRMLS_CC);
	}

	zend_declare_class_constant_stringl(pmta_conn_class, PHPPMTA_SL("LOCAL_SERVER"), PHPPMTA_SL("127.0.0.1") TSRMLS_CC);
	zend_declare_class_constant_long(pmta_conn_class, PHPPMTA_SL("DEFAULT_PORT"), 25 TSRMLS_CC);
}
