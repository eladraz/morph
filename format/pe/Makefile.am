INCLUDES = -I $(top_srcdir) -I $(XSTL_PATH)/Include -I $(PELIB_PATH)/Include -I $(DISMOUNT_PATH)/Include

if DEBUG
DBGFLAGS = -ggdb -DDEBUG
else
DBGFLAGS = -g
endif

lib_LTLIBRARIES = libclr_format_pe.la

libclr_format_pe_la_SOURCES = CilPeLayout.cpp

libclr_format_pe_la_CFLAGS = $(CFLAGS_CLR_COMMON) $(DBGFLAGS) $(AM_CFLAGS)
libclr_format_pe_la_CPPFLAGS = $(CFLAGS_CLR_COMMON) $(DBGFLAGS) $(AM_CFLAGS)

if UNICODE
libclr_format_pe_la_CFLAGS+= -DXSTL_UNICODE -D_UNICODE
libclr_format_pe_la_CPPFLAGS+= -DXSTL_UNICODE -D_UNICODE
endif

if TESTS
libclr_format_pe_la_CFLAGS+= -DTRACED_CLR
libclr_format_pe_la_CPPFLAGS+= -DTRACED_CLR
endif

