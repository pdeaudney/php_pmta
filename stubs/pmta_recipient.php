<?php

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

	public function __construct($address);
	public function __destruct();
	public function __get($property);
	public function __isset($property);
	public function __set($name, $value);
	public function defineVariable($name, $value);
	public function getLastError();
	private function __clone();
}
