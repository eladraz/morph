cmake_minimum_required (VERSION 2.6)
project (mcc)
set (mcc_VERSION_MAJOR 1)
set (mcc_VERSION_MINOR 0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fPIC  -Wno-write-strings")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC  -Wno-write-strings")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

include_directories($ENV{XSTL_PATH}/Include)
include_directories($ENV{PELIB_PATH}/Include)
include_directories($ENV{ELFLIB_PATH}/Include)
include_directories($ENV{DISMOUNT_PATH}/Include)
include_directories(.)

add_definitions(-DXSTL_UNICODE)
add_definitions(-D_UNICODE)

list(APPEND MCC_LIB_FILES
	compiler/ArgumentsPositions.cpp
	compiler/CallingConvention.cpp
	compiler/CompilerEngine.cpp
	compiler/CompilerFactory.cpp
	compiler/CompilerException.cpp
	compiler/CompilerInterface.cpp
	compiler/EmitContext.cpp
	compiler/LocalPositions.cpp
	compiler/MethodBlock.cpp
	compiler/MethodCompiler.cpp
	compiler/MethodRuntimeBoundle.cpp
	compiler/OptimizerCompilerInterface.cpp
	compiler/StackEntity.cpp
	compiler/TemporaryStackHolder.cpp
	compiler/opcodes/ArrayOpcodes.cpp
	compiler/opcodes/Bin32Opcodes.cpp
	compiler/opcodes/BinaryOpcodes.cpp
	compiler/opcodes/CompilerOpcodes.cpp
	compiler/opcodes/ExceptionOpcodes.cpp
	compiler/opcodes/ObjectOpcodes.cpp
	compiler/opcodes/RegisterEvaluatorOpcodes.cpp
	compiler/processors/arm/ARMCompilerInterface.cpp
	compiler/processors/arm/THUMBCompilerInterface.cpp
	compiler/processors/c/32C.cpp
	compiler/processors/ia32/IA32CompilerInterface.cpp
)

list(APPEND MCC_LIB_FILES
	console/ConsoleAlgorithm.cpp
	console/main.cpp
	console/minidump.cpp
)

list(APPEND MCC_LIB_FILES
	data/ConstElements.cpp
	data/ElementType.cpp
)

list(APPEND MCC_LIB_FILES
	executer/compiler/BinaryGetterInterface.cpp
	executer/compiler/CompilerEngineThread.cpp
	executer/compiler/DefaultCompilerAlgorithm.cpp
	executer/compiler/PrecompiledRepository.cpp
	executer/compiler/ScanningAlgorithmInterface.cpp
	executer/linker/ELFLinker.cpp
	executer/linker/FileLinker.cpp
	executer/linker/LinkerFactory.cpp
	executer/linker/LinkerInterface.cpp
	executer/linker/MemoryLinker.cpp
 	executer/runtime/Executer.cpp
 	executer/runtime/ExternalModuleResolver.cpp
 	executer/runtime/ExternalModuleTable.cpp
 	executer/runtime/MethodBinder.cpp
 	executer/runtime/RuntimeClasses/Runtime.cpp
 	executer/runtime/RuntimeClasses/RuntimeClass.cpp
)

list(APPEND MCC_LIB_FILES
	format/EncodingUtils.cpp
	format/metadataHeader.cpp
	format/metadataStream.cpp
	format/MetadataTables.cpp
	format/methodHeader.cpp
	format/MSILScanInterface.cpp
	format/MSILStreams.cpp
	format/pe/CilPeLayout.cpp
	format/signatures/FieldSig.cpp
	format/signatures/LocalVarSignature.cpp
	format/signatures/MethodDefOrRefSignature.cpp
	format/tables/AssemblyRefTable.cpp
	format/tables/AssemblyTable.cpp
	format/tables/ClassLayoutTable.cpp
	format/tables/ConstantTable.cpp
	format/tables/CustomAttributeTable.cpp
	format/tables/DeclSecurityTable.cpp
	format/tables/FieldRVATable.cpp
	format/tables/FieldTable.cpp
	format/tables/GenericParamTable.cpp
	format/tables/InterfaceImplTable.cpp
	format/tables/MemberRefTable.cpp
	format/tables/MethodImplTable.cpp
	format/tables/MethodSemanticsTable.cpp
	format/tables/MethodSpecTable.cpp
	format/tables/MethodTable.cpp
	format/tables/ModuleTable.cpp
	format/tables/NestedClassTable.cpp
	format/tables/ParamTable.cpp
	format/tables/PropertyMapTable.cpp
	format/tables/PropertyTable.cpp
	format/tables/StandAloneSigTable.cpp
	format/tables/Table.cpp
	format/tables/TableFactory.cpp
	format/tables/TypedefTable.cpp
	format/tables/TyperefTable.cpp
	format/tables/TypeSpecTable.cpp
)

list(APPEND MCC_LIB_FILES
	runnable/Apartment.cpp
	runnable/ApartmentFactory.cpp
	runnable/ClrResolver.cpp
	runnable/CorlibNames.cpp
	runnable/CustomAttribute.cpp
	runnable/CustomAttributeArgument.cpp
	runnable/FrameworkMethods.cpp
	runnable/GlobalContext.cpp
	runnable/MethodRunnable.cpp
	runnable/MethodSignature.cpp
	runnable/ResolverInterface.cpp
	runnable/StringReader.cpp
	runnable/StringRepository.cpp
	runnable/TypedefRepository.cpp
	runnable/TypesNameRepository.cpp
)

add_executable(mcc ${MCC_LIB_FILES})

find_package (Threads)
target_link_libraries(mcc ${CMAKE_THREAD_LIBS_INIT})

find_library(LIBXSTL xStlcm HINTS $ENV{XSTL_PATH}/out/lib)
find_library(LIBPE pe HINTS $ENV{PELIB_PATH}/out/lib)
find_library(LIBELF elf HINTS $ENV{ELFLIB_PATH}/out/lib)
find_library(LIBDISMOUNT dismount HINTS $ENV{DISMOUNT_PATH}/out/lib)
target_link_libraries(mcc ${LIBXSTL} ${LIBPE} ${LIBELF} ${LIBDISMOUNT})

if (UNIX)
	set(CMAKE_MACOSX_RPATH 1)
	add_definitions(-DLINUX)
endif()
if (WIN32)
	add_definitions(-DWIN32)
endif()
