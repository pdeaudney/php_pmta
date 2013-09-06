/**
 * @file pmta_connection.h
 * @date Sep 29, 2010 v0.1
 * @date Jul 4, 2013 Major code refactoring, use object handlers instead of magic methods
 * @date Jul 11, 2013 v0.4
 * @author Vladimir Kolesnikov <vladimir@extrememember.com>
 * @brief Exposes @c PmtaConnection class
 * @details
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

	public function __construct($server = '127.0.0.1', $port = 25, $username = null, $password = null);
	public function __destruct();
	public function getLastError();
	public function submitMessage(PmtaMessage $message);
	public function __get($property);
	public function __isset($property);
	private function __clone();
}
@endcode
 */

#ifdef DOXYGEN
#	undef PMTA_CONNECTION_H
#endif

#ifndef PMTA_CONNECTION_H
#define PMTA_CONNECTION_H

#include "php_pmta.h"

/**
 * @brief Registers @c PmtaConnection class
 * @param tsrm_ls Internally used by Zend
 */
PHPPMTA_VISIBILITY_HIDDEN extern void pmtaconn_register_class(TSRMLS_D);

#endif /* PMTA_CONNECTION_H */
