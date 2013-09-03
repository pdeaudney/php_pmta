<?php

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
