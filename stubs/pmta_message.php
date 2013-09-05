<?php

class PmtaMessage
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

	public function __construct($originator);
	public function __destruct();
	public function __get($property);
	public function __isset($property);
	public function __set($property, $value);
	public function beginPart($number);
	public function addData($data);
	public function addMergeData($data);
	public function addDateHeader();
	public function addRecipient(PmtaRecipient $recipient);
	public function getLastError();
	private function __clone();
}
