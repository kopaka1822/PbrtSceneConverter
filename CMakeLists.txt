cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)
cmake_policy(VERSION 3.0.2)

project(PBRTConverter VERSION 0.1 LANGUAGES CXX C)

# Hide the default CMake targets in a filter
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(PREDEFINED_TARGETS_FOLDER "CMakePredefinedTargets")

# Optionally build the command-line tool
option(PBRTCONVERTER_BUILD_APP "Build command-line tool" ON)

# Let the user decide if he wants to build a shared or static lib
# TODO: for this to generate a static linking artifact, we need to define what
# gets exported!
#option(PBRTCONVERTER_BUILD_SHARED "Build as shared instead of static library" OFF)
#if(PBRTCONVERTER_BUILD_SHARED)
#	set(PBRTCONVERTER_LIB_TYPE SHARED)
#else()
	set(PBRTCONVERTER_LIB_TYPE STATIC)
#endif()

# Headers which are required to be included by the library user (and thus need to be installed)
set(PBRTCONVERTER_LIB_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/ArgumentSet.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/parser_exception.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/system.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/config/eiconfig.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/geometry/Shape.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/PBRT/Light.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/PBRT/Material.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/PBRT/ParamSet.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/PBRT/PbrtScene.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/PBRT/spectrum.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/PBRT/TextureParams.h"
)
# For the file dialog we have to take care to use the proper files
if(WIN32)
	set(PBRTCONVERTER_FILE_DIALOG_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Source/filedialog/nfd_win.cpp")
else()
	set(PBRTCONVERTER_FILE_DIALOG_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Source/filedialog/nfd_gtk.c")
endif()

# Epsilon headers (to build the source groups; we don't actually need them explicitly)
if(NOT TARGET epsilon)
	set(PBRTCONVERTER_EPSILON_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include/ei/2dintersection.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include/ei/2dtypes.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include/ei/3dintersection.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include/ei/3dtypes.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include/ei/configure.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include/ei/conversions.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include/ei/defaultconfig.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include/ei/elementarytypes.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include/ei/prime.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include/ei/quaternion.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include/ei/stdextensions.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include/ei/vector.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include/ei/details/inttemplate.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include/ei/details/primetest.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include/ei/details/vectordetailsA.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include/ei/details/vectortypedef.hpp"
	)
	mark_as_advanced(PBRTCONVERTER_EPSILON_SOURCES)
endif()
set(PBRTCONVERTER_SOURCE_FILES
	"${PBRTCONVERTER_LIB_HEADERS}"
	"${PBRTCONVERTER_FILE_DIALOG_SOURCES}"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/DialogOpenFile.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/DialogOpenFile.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Exception.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/file.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/parser.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/parser.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/parser_exception.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/parser_helper.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/raw_parser.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/system.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Test.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Test.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/TestHeader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/config/eiconfig.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/filedialog/common.h"
	#"${CMAKE_CURRENT_SOURCE_DIR}/Source/filedialog/nfd_cocoa.m"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/filedialog/nfd_common.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/filedialog/nfd_common.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/filedialog/include/nfd.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/geometry/LoopSubDiv.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/geometry/Plymesh.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/geometry/Plymesh.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/geometry/Sphere.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/geometry/SubdivisionHelper.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/geometry/SubdivisionHelper.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/geometry/TriangleMesh.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/PBRT/copper.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/PBRT/ParamSet.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/PBRT/PbrtScene.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/PBRT/spectrum.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/PBRT/system.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/PBRT/volume.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/PBRT/volume.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/rply/rply.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/rply/rply.h"
)
mark_as_advanced(PBRTCONVERTER_LIB_HEADERS)
mark_as_advanced(PBRTCONVERTER_SOURCE_FILES)

# Basic library target
add_library(PBRTConverterLib ${PBRTCONVERTER_LIB_TYPE}
	"${PBRTCONVERTER_SOURCE_FILES}"
	"${PBRTCONVERTER_EPSILON_SOURCES}"
)
if(PBRTCONVERTER_BUILD_APP)
	add_executable(PBRTConverter 
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/main.cpp"
	)
	target_compile_definitions(PBRTConverter PRIVATE _CRT_SECURE_NO_WARNINGS)
	target_compile_features(PBRTConverter PRIVATE cxx_std_11)
	target_link_libraries(PBRTConverter PBRTConverterLib)
	
	install(TARGETS PBRTConverter
		EXPORT pbrtconverterTargets
		LIBRARY DESTINATION lib COMPONENT Runtime
		ARCHIVE DESTINATION lib COMPONENT Development
		RUNTIME DESTINATION bin COMPONENT Runtime
		PUBLIC_HEADER DESTINATION include COMPONENT Development
		BUNDLE DESTINATION bin COMPONENT Runtime
	)
endif()

target_compile_definitions(PBRTConverterLib PRIVATE _CRT_SECURE_NO_WARNINGS)
target_compile_features(PBRTConverterLib PRIVATE cxx_std_11)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PBRTCONVERTER_SOURCE_FILES})

# Target for the header-only dependency w.o CMake support
if(NOT TARGET epsilon)
	add_library(epsilon INTERFACE)
	target_include_directories(epsilon
		INTERFACE
			$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Source/epsilon/include>
			$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Source/config>
			$<INSTALL_INTERFACE:include>
	)
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/Source/epsilon/include/ei DESTINATION include)
	source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source/epsilon/include" PREFIX "Source/epsilon" FILES ${PBRTCONVERTER_EPSILON_SOURCES})
endif()

target_include_directories(PBRTConverterLib
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Source>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Source/config>
		$<INSTALL_INTERFACE:include>
)
target_link_libraries(PBRTConverterLib epsilon)

# Create version compatibility
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/pbrtconverterConfigVersion.cmake"
    VERSION 0.1
    COMPATIBILITY AnyNewerVersion
)

# Install the library to the chosen CMAKE_INSTALL_PREFIX with normal folder structure
install(TARGETS PBRTConverterLib epsilon
    EXPORT pbrtconverterTargets
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
    RUNTIME DESTINATION bin COMPONENT Runtime
    PUBLIC_HEADER DESTINATION include COMPONENT Development
    BUNDLE DESTINATION bin COMPONENT Runtime
)

# Create the configuration file to allow find_package(PBRTConverterLib)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/pbrtconverterConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/pbrtconverterConfig.cmake"
    INSTALL_DESTINATION lib/cmake/mylib
)
install(EXPORT pbrtconverterTargets
	NAMESPACE PBRTConverter::
	DESTINATION lib/cmake/PBRTConverter
)
install(FILES "${PROJECT_BINARY_DIR}/pbrtconverterConfigVersion.cmake"
              "${PROJECT_BINARY_DIR}/pbrtconverterConfig.cmake"
	DESTINATION lib/cmake/PBRTConverter
)
# Copy over the public header files while maintaining directory structure
# Unfortunately we can't just specify these as PUBLIC_HEADER because CMake stubbornly won't place
# public headers in subdirectories...
foreach(header ${PBRTCONVERTER_LIB_HEADERS})
	file(RELATIVE_PATH rel "${CMAKE_CURRENT_SOURCE_DIR}/Source" ${header})
	get_filename_component(dir ${rel} DIRECTORY)
	install(FILES ${header} DESTINATION include/${dir})
endforeach(header)