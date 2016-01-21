INCLUDES = -I $(top_srcdir) -I $(XSTL_PATH)/Include -I $(PELIB_PATH)/Include -I $(DISMOUNT_PATH)/Include -I ${ELFLIB_PATH}/Include

if DEBUG
DBGFLAGS = -ggdb -DDEBUG
else
DBGFLAGS = -g
endif

lib_LTLIBRARIES = libclr_executer_linker.la

libclr_executer_linker_la_SOURCES = ELFLinker.cpp \
                                     FileLinker.cpp \
                                     LinkerFactory.cpp \
                                     LinkerInterface.cpp \
                                     MemoryLinker.cpp

libclr_executer_linker_la_CFLAGS = $(CFLAGS_CLR_COMMON) $(DBGFLAGS) $(AM_CFLAGS)
libclr_executer_linker_la_CPPFLAGS = $(CFLAGS_CLR_COMMON) $(DBGFLAGS) $(AM_CFLAGS)

if UNICODE
libclr_executer_linker_la_CFLAGS+= -DXSTL_UNICODE -D_UNICODE
libclr_executer_linker_la_CPPFLAGS+= -DXSTL_UNICODE -D_UNICODE
endif

if TESTS
libclr_executer_linker_la_CFLAGS+= -DTRACED_CLR
libclr_executer_linker_la_CPPFLAGS+= -DTRACED_CLR
endif

