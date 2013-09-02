<?php

class PmtaError extends Exception
{
	const OUT_OF_MEMORY    = PmtaApiERROR_OutOfMemory;
	const ILLEGAL_STATE    = PmtaApiERROR_IllegalState;
	const ILLEGAL_ARGUMENT = PmtaApiERROR_IllegalArgument;
	const SECURITY         = PmtaApiERROR_Security;
	const IO               = PmtaApiERROR_IO;
	const SERVICE          = PmtaApiERROR_Service;
	const EMAIL_ADDRESS    = PmtaApiERROR_EmailAddress;
}

final class PmtaErrorConnection extends PmtaError {}
final class PmtaErrorRecipient  extends PmtaError {}
final class PmtaErrorMessage    extends PmtaError {}
