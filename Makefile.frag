htmldocs: docs/html/index.html Doxyfile

docs/html/index.html: macros.h config.h extension.c php_pmta.h pmta_common.c pmta_common.h pmta_connection.c pmta_connection.h pmta_error.c pmta_error.h pmta_message.c pmta_message.h pmta_recipient.c pmta_recipient.h Doxyfile
	doxygen Doxyfile

macros.h: extension.c pmta_common.c pmta_connection.c pmta_error.c pmta_message.c pmta_recipient.c
	$(CPP) $(COMMON_FLAGS) -DDOXYGEN -DHAVE_CONFIG_H -dD $^ | $(CPP) $(DEFS) $(CPPFLAGS) -DDOXYGEN -DHAVE_CONFIG_H -dM - > $@
