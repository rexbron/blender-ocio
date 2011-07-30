# ***** BEGIN GPL LICENSE BLOCK *****
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# The Original Code is Copyright (C) 2006, Blender Foundation
# All rights reserved.
#
# The Original Code is: all of this file.
#
# Contributor(s): xat
#
# ***** END GPL LICENSE BLOCK *****

include(ExternalProject)

###############################################################################
### tinyxml ###
set(TINYXML_VERSION 2_6_1)
set(TINYXML_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/ext/dist)
if(CMAKE_TOOLCHAIN_FILE)
    set(TINYXML_CMAKE_ARGS ${TINYXML_CMAKE_ARGS} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
endif()
ExternalProject_Add(tinyxml
    URL ${CMAKE_CURRENT_SOURCE_DIR}/ext/tinyxml_${TINYXML_VERSION}.tar.gz
    PATCH_COMMAND patch -f -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/ext/tinyxml_${TINYXML_VERSION}.patch
    BINARY_DIR ext/build/tinyxml
    INSTALL_DIR ext/dist
    CMAKE_ARGS ${TINYXML_CMAKE_ARGS}
)
set(TINYXML_STATIC_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/ext/dist/lib/libtinyxml.a)

###############################################################################
### YAML ###
set(YAML_CPP_VERSION r482)
set(YAML_CPP_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/ext/dist -DYAML_CPP_BUILD_TOOLS:BOOL=FALSE)
if(CMAKE_TOOLCHAIN_FILE)
    set(YAML_CPP_CMAKE_ARGS ${YAML_CPP_CMAKE_ARGS} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
endif()
ExternalProject_Add(YAML_CPP_LOCAL
    URL ${CMAKE_CURRENT_SOURCE_DIR}/ext/yaml-cpp-${YAML_CPP_VERSION}.tar.gz
    BINARY_DIR ext/build/yaml-cpp
    PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/ext/yaml-cpp-${YAML_CPP_VERSION}.patch
    INSTALL_DIR ext/dist
    CMAKE_ARGS ${YAML_CPP_CMAKE_ARGS}
)
set(YAML_CPP_STATIC_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/ext/dist/lib/libyaml-cpp.a)

###############################################################################
### Externals ###
set(EXTERNAL_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/ext/dist/include)
set(EXTERNAL_COMPILE_FLAGS "-DTIXML_USE_STL -fPIC -fvisibility-inlines-hidden -fvisibility=hidden")
set(EXTERNAL_LINK_FLAGS "")
set(EXTERNAL_LIBRARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}/ext/dist/lib)
set(EXTERNAL_LIBRARIES
    ${TINYXML_STATIC_LIBRARIES}
    ${YAML_CPP_STATIC_LIBRARIES})

###############################################################################


set(OCIO_VERSION_MAJOR 0)
set(OCIO_VERSION_MINOR 8)
set(OCIO_VERSION_PATCH 4)
set(OCIO_VERSION "${OCIO_VERSION_MAJOR}.${OCIO_VERSION_MINOR}.${OCIO_VERSION_PATCH}")
set(SOVERSION ${OCIO_VERSION_MAJOR})

set(OCIO_NAMESPACE OpenColorIO)

set(OCIO_USE_BOOST_PTR 0)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/export/OpenColorIO/OpenColorABI.h.in
				${CMAKE_CURRENT_BINARY_DIR}/export/OpenColorABI.h @ONLY)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
add_definitions("-DUSE_SSE")

add_definitions(${EXTERNAL_COMPILE_FLAGS})

set(INC
	.
	export
	${CMAKE_CURRENT_BINARY_DIR}/export
	${EXTERNAL_INCLUDE_DIRS}
)

set(INC_SYS
)

set(SRC
	src/core/AllocationOp.cpp
	src/core/AllocationTransform.cpp
	src/core/Baker.cpp
	src/core/Caching.cpp
	src/core/CDLTransform.cpp
	src/core/ColorSpace.cpp
	src/core/ColorSpaceTransform.cpp
	src/core/Config.cpp
	src/core/Context.cpp
	src/core/DisplayTransform.cpp
	src/core/Exception.cpp
	src/core/ExponentOps.cpp
	src/core/ExponentTransform.cpp
	src/core/FileFormat3DL.cpp
	src/core/FileFormatCCC.cpp
	src/core/FileFormatCC.cpp
	src/core/FileFormatCSP.cpp
	src/core/FileFormatHDL.cpp
	src/core/FileFormatIridas.cpp
	src/core/FileFormatPandora.cpp
	src/core/FileFormatSpi1D.cpp
	src/core/FileFormatSpi3D.cpp
	src/core/FileFormatSpiMtx.cpp
	src/core/FileFormatTruelight.cpp
	src/core/FileFormatVF.cpp
	src/core/FileTransform.cpp
	src/core/GpuAllocationNoOp.cpp
	src/core/GpuShaderDesc.cpp
	src/core/GpuShaderUtils.cpp
	src/core/GroupTransform.cpp
	src/core/HashUtils.cpp
	src/core/ImageDesc.cpp
	src/core/ImagePacking.cpp
	src/core/LogOps.cpp
	src/core/LogTransform.cpp
	src/core/Look.cpp
	src/core/LookTransform.cpp
	src/core/Lut1DOp.cpp
	src/core/Lut3DOp.cpp
	src/core/MathUtils.cpp
	src/core/MatrixOps.cpp
	src/core/MatrixTransform.cpp
	src/core/OCIOYaml.cpp
	src/core/Op.cpp
	src/core/ParseUtils.cpp
	src/core/PathUtils.cpp
	src/core/Processor.cpp
	src/core/ScanlineHelper.cpp
	src/core/Transform.cpp
	src/core/TruelightOp.cpp
	src/core/TruelightTransform.cpp
#	src/core/UnitTest.cpp
	
	src/core/AllocationOp.h
	src/core/CDLTransform.h
	src/core/ExponentOps.h
	src/core/FileTransform.h
	src/core/GpuShaderUtils.h
	src/core/HashUtils.h
	src/core/ImagePacking.h
	src/core/LogOps.h
	src/core/Lut1DOp.h
	src/core/Lut3DOp.h
	src/core/MathUtils.h
	src/core/MatrixOps.h
	src/core/Mutex.h
	src/core/OCIOYaml.h
	src/core/OpBuilders.h
	src/core/Op.h
	src/core/ParseUtils.h
	src/core/PathUtils.h
	src/core/Platform.h
	src/core/PrivateTypes.h
	src/core/Processor.h
	src/core/ScanlineHelper.h
	src/core/SSE.h
	src/core/TruelightOp.h
#	src/core/UnitTest.h
	
	src/core/md5/md5.h
	src/core/md5/md5.cpp
	
	src/core/pystring/pystring.h
	src/core/pystring/pystring.cpp
	
	export/OpenColorIO/OpenColorIO.h
	export/OpenColorIO/OpenColorTransforms.h
	export/OpenColorIO/OpenColorTypes.h
	
	ocio-capi.cpp
	ocio-capi.h
)

blender_add_lib(extern_ocio "${SRC}" "${INC}" "${INC_SYS}")

target_link_libraries(extern_ocio ${EXTERNAL_LIBRARIES})
add_dependencies(extern_ocio tinyxml YAML_CPP_LOCAL)
