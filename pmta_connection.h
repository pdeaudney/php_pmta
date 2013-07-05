/**
 * @file pmta_connection.h
 * @date 29.09.2010
 * @author Vladimir Kolesnikov <vladimir@extrememember.com>
 * @brief Exposes @c PmtaConnection class
 * @details
 * <PRE>
 * final class PmtaConnection
 * {
 * 	const LOCAL_SERVER = "127.0.0.1";
 * 	const DEFAULT_PORT = 25;
 *
 * 	private $connection;
 *
 * 	private $server;
 * 	private $port;
 * 	private $username;
 * 	private $password;
 *
 * 	public function __construct($server = '127.0.0.1', $port = 25, $username = null, $password = null);
 * 	public function __destruct();
 * 	public function getLastError();
 * 	public function submitMessage(PmtaMessage $message);
 * 	public function __get($property);
 * 	public function __isset($property);
 * 	private function __clone();
 * }
 * </PRE>
 */

#ifdef DOXYGEN
#	undef PMTA_CONNECTION_H_
#endif

#ifndef PMTA_CONNECTION_H_
#define PMTA_CONNECTION_H_

#include "php_pmta.h"

/**
 * @brief PMTA Connection (PmtaConn) resource destructor
 * @param res Resource
 * @param tsrm_ls Internally used by Zend
 */
PHPPMTA_VISIBILITY_HIDDEN extern void pmta_connection_dtor(zend_rsrc_list_entry* res TSRMLS_DC);

/**
 * @brief Registers PmtaConnection class
 * @param tsrm_ls Internally used by Zend
 */
PHPPMTA_VISIBILITY_HIDDEN extern void pmtaconn_register_class(TSRMLS_D);

#endif /* PMTA_CONNECTION_H_ */
