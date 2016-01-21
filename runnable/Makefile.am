INCLUDES = -I $(top_srcdir) -I $(XSTL_PATH)/Include -I $(PELIB_PATH)/Include -I $(DISMOUNT_PATH)/Include

if DEBUG
DBGFLAGS = -ggdb -DDEBUG
else
DBGFLAGS = -g
endif

lib_LTLIBRARIES = libclr_runnable.la

libclr_runnable_la_SOURCES = Apartment.cpp \
                                     ApartmentFactory.cpp \
                                     ClrResolver.cpp \
                                     CorlibNames.cpp \
                                     CustomAttribute.cpp \
                                     CustomAttributeArgument.cpp \
                                     FrameworkMethods.cpp \
                                     GlobalContext.cpp \
                                     MethodRunnable.cpp \
                                     MethodSignature.cpp \
                                     ResolverInterface.cpp \
                                     StringReader.cpp \
                                     StringRepository.cpp \
                                     TypedefRepository.cpp \
                                     TypesNameRepository.cpp

libclr_runnable_la_CFLAGS = $(CFLAGS_CLR_COMMON) $(DBGFLAGS) $(AM_CFLAGS)
libclr_runnable_la_CPPFLAGS = $(CFLAGS_CLR_COMMON) $(DBGFLAGS) $(AM_CFLAGS)

if UNICODE
libclr_runnable_la_CFLAGS+= -DXSTL_UNICODE -D_UNICODE
libclr_runnable_la_CPPFLAGS+= -DXSTL_UNICODE -D_UNICODE
endif

if TESTS
libclr_runnable_la_CFLAGS+= -DTRACED_CLR
libclr_runnable_la_CPPFLAGS+= -DTRACED_CLR
endif

