PHP_ARG_WITH(pmta, [Whether to enable PowerMTA PHP Submission API], [ --with-pmta[=DIR]     Enable PowerMTA Submission API])

if test "$PHP_PMTA" != "no"; then
	for i in "$PHP_PMTA" /opt/pmta/api; do
		test -f $i/include/PmtaApi.h -a -f $i/include/submitter/PmtaConn.h -a -f $i/include/submitter/PmtaMsg.h -a -f $i/include/submitter/PmtaRcpt.h && \
		PMTA_INCLUDE_DIR=$i/include && \
		break
	done

	if test -z "$PMTA_INCLUDE_DIR"; then
		AC_MSG_ERROR([Cannot find PMTA header files])
	fi

	PHP_ADD_INCLUDE($PMTA_INCLUDE_DIR)

	PHP_CHECK_LIBRARY(
		[pmta],
		[PmtaConnAlloc],
		[
			PHP_ADD_LIBRARY_WITH_PATH([pmta],, PMTA_SHARED_LIBADD)
		],
		[AC_MSG_ERROR([Invalid PMTA library, PmtaConnAlloc() not found])]
	)

	PHP_SUBST(PMTA_SHARED_LIBADD)
	PHP_NEW_EXTENSION(pmta, [extension.c pmta_common.c pmta_error.c pmta_connection.c pmta_recipient.c pmta_message.c], $ext_shared,, [-Wall])

	PHP_ADD_MAKEFILE_FRAGMENT
fi
