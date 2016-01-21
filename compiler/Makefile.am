INCLUDES = -I $(top_srcdir) -I $(XSTL_PATH)/Include -I $(PELIB_PATH)/Include -I $(DISMOUNT_PATH)/Include

if DEBUG
DBGFLAGS = -ggdb -DDEBUG
else
DBGFLAGS = -g
endif

lib_LTLIBRARIES = libclr_compiler.la

libclr_compiler_la_SOURCES = ArgumentsPositions.cpp CallingConvention.cpp CompilerEngine.cpp CompilerFactory.cpp CompilerException.cpp \
                            CompilerInterface.cpp EmitContext.cpp LocalPositions.cpp MethodBlock.cpp MethodCompiler.cpp \
                            MethodRuntimeBoundle.cpp OptimizerCompilerInterface.cpp StackEntity.cpp TemporaryStackHolder.cpp

libclr_compiler_la_CFLAGS = $(CFLAGS_CLR_COMMON) $(DBGFLAGS) $(AM_CFLAGS)
libclr_compiler_la_CPPFLAGS = $(CFLAGS_CLR_COMMON) $(DBGFLAGS) $(AM_CFLAGS)

if UNICODE
libclr_compiler_la_CFLAGS+= -DXSTL_UNICODE -D_UNICODE
libclr_compiler_la_CPPFLAGS+= -DXSTL_UNICODE -D_UNICODE
endif

if TESTS
libclr_compiler_la_CFLAGS+= -DTRACED_CLR
libclr_compiler_la_CPPFLAGS+= -DTRACED_CLR
endif

