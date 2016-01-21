INCLUDES = -I $(top_srcdir) -I $(XSTL_PATH)/Include -I $(PELIB_PATH)/Include -I $(DISMOUNT_PATH)/Include

if DEBUG
DBGFLAGS = -ggdb -DDEBUG
else
DBGFLAGS = -g
endif

bin_PROGRAMS = tdump

tdump_SOURCES = DefaultStringsTableFormatter.cpp  printTable.cpp  resolver.cpp  TDump.cpp

tdump_CFLAGS = $(CFLAGS_CLR_COMMON) $(DBGFLAGS) $(AM_CFLAGS)
tdump_CPPFLAGS = $(CFLAGS_CLR_COMMON) $(DBGFLAGS) $(AM_CFLAGS)

if UNICODE
tdump_CFLAGS+= -DXSTL_UNICODE -D_UNICODE
tdump_CPPFLAGS+= -DXSTL_UNICODE -D_UNICODE
endif

if TESTS
tdump_CFLAGS+= -DTRACED_CLR
tdump_CPPFLAGS+= -DTRACED_CLR
endif


tdump_LDADD = -L$(XSTL_PATH)/out/lib -lxstl \
            -L$(XSTL_PATH)/out/lib -lxstl_data \
            -L$(XSTL_PATH)/out/lib -lxstl_except \
            -L$(XSTL_PATH)/out/lib -lxstl_stream \
            -L$(XSTL_PATH)/out/lib -lxstl_os \
            -L$(XSTL_PATH)/out/lib -lxstl_unix \
            -L$(XSTL_PATH)/out/lib -lxstl_enc \
            -L$(XSTL_PATH)/out/lib -lxstl_digest \
            -L$(XSTL_PATH)/out/lib -lxstl_random \
            -L$(XSTL_PATH)/out/lib -lxstl_encryptions \
            -L$(XSTL_PATH)/out/lib -lxstl_utils \
            -L$(ELFLIB_PATH)/out/lib -lelf \
            -L$(PELIB_PATH)/out/lib -lpe \
            -L$(DISMOUNT_PATH)/out/lib -ldismount \
            -L$(top_srcdir)/data/ -lclr_data \
            -L$(top_srcdir)/compiler -lclr_compiler \
            -L$(top_srcdir)/compiler/opcodes -lclr_compiler_opcodes \
            -L$(top_srcdir)/compiler/processors/arm -lclr_compiler_proc_arm \
            -L$(top_srcdir)/compiler/processors/ia32 -lclr_compiler_proc_ia32 \
            -L$(top_srcdir)/compiler/processors/c -lclr_compiler_proc_c \
            -L$(top_srcdir)/format -lclr_format \
            -L$(top_srcdir)/format/pe -lclr_format_pe \
            -L$(top_srcdir)/format/signatures -lclr_format_signatures \
            -L$(top_srcdir)/format/tables -lclr_format_tables \
            -L$(top_srcdir)/executer/compiler -lclr_executer_compiler \
            -L$(top_srcdir)/executer/linker -lclr_executer_linker \
            -L$(top_srcdir)/executer/runtime -lclr_executer_runtime \
            -L$(top_srcdir)/executer/runtime/RuntimeClasses -lclr_executer_runtime_classes \
            -L$(top_srcdir)/runnable -lclr_runnable

