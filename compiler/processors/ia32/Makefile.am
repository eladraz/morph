INCLUDES = -I $(top_srcdir) -I $(XSTL_PATH)/Include -I $(PELIB_PATH)/Include -I $(DISMOUNT_PATH)/Include

if DEBUG
DBGFLAGS = -ggdb -DDEBUG
else
DBGFLAGS = -g
endif

lib_LTLIBRARIES = libclr_compiler_proc_ia32.la

libclr_compiler_proc_ia32_la_SOURCES = IA32CompilerInterface.cpp

libclr_compiler_proc_ia32_la_CFLAGS = $(CFLAGS_CLR_COMMON) $(DBGFLAGS) $(AM_CFLAGS)
libclr_compiler_proc_ia32_la_CPPFLAGS = $(CFLAGS_CLR_COMMON) $(DBGFLAGS) $(AM_CFLAGS)

if UNICODE
libclr_compiler_proc_ia32_la_CFLAGS+= -DXSTL_UNICODE -D_UNICODE
libclr_compiler_proc_ia32_la_CPPFLAGS+= -DXSTL_UNICODE -D_UNICODE
endif

if TESTS
libclr_compiler_proc_ia32_la_CFLAGS+= -DTRACED_CLR
libclr_compiler_proc_ia32_la_CPPFLAGS+= -DTRACED_CLR
endif

