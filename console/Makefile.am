INCLUDES = -I $(top_srcdir) -I $(XSTL_PATH)/Include -I $(PELIB_PATH)/Include -I $(DISMOUNT_PATH)/Include

if DEBUG
DBGFLAGS = -ggdb -DDEBUG
else
DBGFLAGS = -g
endif

bin_PROGRAMS = mcc

mcc_SOURCES = ConsoleAlgorithm.cpp main.cpp minidump.cpp

mcc_CFLAGS = $(CFLAGS_CLR_COMMON) $(DBGFLAGS) $(AM_CFLAGS)
mcc_CPPFLAGS = $(CFLAGS_CLR_COMMON) $(DBGFLAGS) $(AM_CFLAGS)

if UNICODE
mcc_CFLAGS+= -DXSTL_UNICODE -D_UNICODE
mcc_CPPFLAGS+= -DXSTL_UNICODE -D_UNICODE
endif

if TESTS
mcc_CFLAGS+= -DTRACED_CLR
mcc_CPPFLAGS+= -DTRACED_CLR
endif

# mcc_LDFLAGS = -static

mcc_LDADD = $(XSTL_PATH)/out/lib/libxstl.la \
            $(XSTL_PATH)/out/lib/libxstl_data.la \
            $(XSTL_PATH)/out/lib/libxstl_except.la \
            $(XSTL_PATH)/out/lib/libxstl_stream.la \
            $(XSTL_PATH)/out/lib/libxstl_os.la \
            $(XSTL_PATH)/out/lib/libxstl_unix.la \
            $(XSTL_PATH)/out/lib/libxstl_enc.la \
            $(XSTL_PATH)/out/lib/libxstl_digest.la \
            $(XSTL_PATH)/out/lib/libxstl_random.la \
            $(XSTL_PATH)/out/lib/libxstl_encryptions.la \
            $(XSTL_PATH)/out/lib/libxstl_utils.la \
            $(ELFLIB_PATH)/out/lib/libelf.la \
            $(PELIB_PATH)/out/lib/libpe.la \
            $(DISMOUNT_PATH)/out/lib/libdismount.la \
            $(top_srcdir)/data/libclr_data.la \
            $(top_srcdir)/compiler/libclr_compiler.la \
            $(top_srcdir)/compiler/opcodes/libclr_compiler_opcodes.la \
            $(top_srcdir)/compiler/processors/arm/libclr_compiler_proc_arm.la \
            $(top_srcdir)/compiler/processors/ia32/libclr_compiler_proc_ia32.la \
            $(top_srcdir)/compiler/processors/c/libclr_compiler_proc_c.la \
            $(top_srcdir)/format/libclr_format.la \
            $(top_srcdir)/format/pe/libclr_format_pe.la \
            $(top_srcdir)/format/signatures/libclr_format_signatures.la \
            $(top_srcdir)/format/tables/libclr_format_tables.la \
            $(top_srcdir)/executer/compiler/libclr_executer_compiler.la \
            $(top_srcdir)/executer/linker/libclr_executer_linker.la \
            $(top_srcdir)/executer/runtime/libclr_executer_runtime.la \
            $(top_srcdir)/executer/runtime/RuntimeClasses/libclr_executer_runtime_classes.la \
            $(top_srcdir)/runnable/libclr_runnable.la
