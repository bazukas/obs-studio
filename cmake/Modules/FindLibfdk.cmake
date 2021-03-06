# Once done these will be defined:
#
#  LIBFDK_FOUND
#  LIBFDK_INCLUDE_DIRS
#  LIBFDK_LIBRARIES
#

if(LIBFDK_INCLUDE_DIRS AND LIBFDK_LIBRARIES)
	set(LIBFDK_FOUND TRUE)
else()
	find_package(PkgConfig QUIET)
	if (PKG_CONFIG_FOUND)
		pkg_check_modules(_LIBFDK QUIET fdk-aac)
	endif()

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(_lib_suffix 64)
	else()
		set(_lib_suffix 32)
	endif()

	find_path(Libfdk_INCLUDE_DIR
		NAMES fdk-aac/aacenc_lib.h
		HINTS
			ENV LibfdkPath
			ENV FFmpegPath
			${_LIBFDK_INCLUDE_DIRS}
			/usr/include /usr/local/include /opt/local/include /sw/include)

	find_library(Libfdk_LIB
		NAMES fdk-aac libfdk-aac
		HINTS
			${Libfdk_INCLUDE_DIR}/../lib
			${Libfdk_INCLUDE_DIR}/lib${_lib_suffix}
			${_LIBFDK_LIBRARY_DIRS}
			/usr/lib /usr/local/lib /opt/local/lib /sw/lib)

	set(LIBFDK_INCLUDE_DIRS ${Libfdk_INCLUDE_DIR} CACHE PATH "Libfdk include dir")
	set(LIBFDK_LIBRARIES ${Libfdk_LIB} CACHE STRING "Libfdk libraries")

	find_package_handle_standard_args(Libfdk DEFAULT_MSG Libfdk_LIB Libfdk_INCLUDE_DIR)
	mark_as_advanced(Libfdk_INCLUDE_DIR Libfdk_LIB)
endif()

