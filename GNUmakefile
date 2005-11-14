include $(GNUSTEP_MAKEFILES)/common.make

-include config.make

PACKAGE_NAME = Performance
PACKAGE_VERSION = 0.1.0
CVS_MODULE_NAME = gnustep/dev-libs/Performance
CVS_TAG_NAME = Performance

TEST_TOOL_NAME=

LIBRARY_NAME=Performance
DOCUMENT_NAME=Performance

Performance_INTERFACE_VERSION=0.1

Performance_OBJC_FILES += \
	GSCache.m \
	GSThroughput.m \
	GSTicker.m \

Performance_HEADER_FILES += \
	Performance.h \
	GSCache.h \
	GSThroughput.h \
	GSTicker.h \

Performance_AGSDOC_FILES += \
	GSCache.h \
	GSThroughput.h \
	GSTicker.h

# Optional Java wrappers for the library
JAVA_WRAPPER_NAME = Performance

#
# Assume that the use of the gnu runtime means we have the gnustep
# base library and can use its extensions to build Performance stuff.
#
ifeq ($(OBJC_RUNTIME_LIB),gnu)
APPLE=0
else
APPLE=1
endif

ifeq ($(APPLE),1)
ADDITIONAL_OBJC_LIBS += -lgnustep-baseadd
Performance_LIBRARIES_DEPEND_UPON = -lgnustep-baseadd
endif

Performance_HEADER_FILES_INSTALL_DIR = Performance

-include GNUmakefile.preamble

include $(GNUSTEP_MAKEFILES)/library.make
# If JIGS is installed, automatically generate Java wrappers as well.
# Because of the '-', should not complain if java-wrapper.make can't be
# found ... simply skip generation of java wrappers in that case.
-include $(GNUSTEP_MAKEFILES)/java-wrapper.make
include $(GNUSTEP_MAKEFILES)/test-tool.make
include $(GNUSTEP_MAKEFILES)/documentation.make

-include GNUmakefile.postamble
