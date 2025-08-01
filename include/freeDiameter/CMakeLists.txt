#CMake configuration for freeDiameter include directory

Project("freeDiameter includes directory" C)

INCLUDE(FindPkgConfig)

########################
# Configurable parameters

# Disable SCTP support completely ?
OPTION(DISABLE_SCTP "Disable SCTP support?" OFF)
IF (NOT DISABLE_SCTP)
	OPTION(DEBUG_SCTP "Verbose SCTP (for debug)?" OFF)
	OPTION(SCTP_USE_MAPPED_ADDRESSES "Use v6-mapped v4 addresses in SCTP (workaround some SCTP limitations)?" OFF)
ENDIF (NOT DISABLE_SCTP)

# Find TODO items in the code easily ?
OPTION(ERRORS_ON_TODO "(development) Generate compilation errors on TODO items ?" OFF)

# Compiler warnings are fatal?
OPTION(ERRORS_ON_WARNINGS "(development) Compiler warnings cause compilation errors ?" OFF)

# In DEBUG mode, each log can contain pid, calling function and file for easy debug. Set to ON to display this information.
OPTION(DEBUG_WITH_META "Show calling location in logs?" OFF)

# Create the absolute path for searching extensions
SET(DEFAULT_EXTENSIONS_PATH ${CMAKE_INSTALL_PREFIX}/${INSTALL_EXTENSIONS_SUFFIX})

# IDNA considerations
OPTION(DIAMID_IDNA_IGNORE "Ignore completely invalid characters in Diameter Identities (process blindly)?" OFF)
IF (NOT DIAMID_IDNA_IGNORE)
	OPTION (DIAMID_IDNA_REJECT "Reject internationalized Diameter Identities, do not attempt to convert it (stringprep) ?" OFF)
ENDIF (NOT DIAMID_IDNA_IGNORE)

# Disable expiration of connections with dynamically connected peers as per RFC 3539 ? (default is enabled)
# Note: if someone needs, we could also make the delay configurable here...
OPTION(DISABLE_PEER_EXPIRY "Disable RFC3539 Peers Connections Expiration after inactivity?" OFF)

# The following workaround increases compatibility with some implementations without breaking anything in freeDiameter,
# so it can be enabled without risk. We keep it disabled by default anyway for those people who use freeDiameter to check the
# compliance of their implementation with the Diameter RFC...
OPTION(WORKAROUND_ACCEPT_INVALID_VSAI "Do not reject a CER/CEA with a Vendor-Specific-Application-Id AVP containing both Auth- and Acct- application AVPs?" OFF)

MARK_AS_ADVANCED(DISABLE_SCTP DEBUG_SCTP SCTP_USE_MAPPED_ADDRESSES ERRORS_ON_TODO DEBUG_WITH_META DIAMID_IDNA_IGNORE DIAMID_IDNA_REJECT DISABLE_PEER_EXPIRY WORKAROUND_ACCEPT_INVALID_VSAI)

########################
### System checks part

INCLUDE (CheckLibraryExists)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckIncludeFiles)
INCLUDE (CheckSymbolExists)
INCLUDE (CheckCSourceCompiles)
INCLUDE (TestBigEndian)


### System checks -- mandatory support

# We need the getopt_long function
CHECK_FUNCTION_EXISTS (getopt_long HAVE_LONG_OPTIONS)
IF (NOT HAVE_LONG_OPTIONS)
   MESSAGE(SEND_ERROR "The getopt_long function is not found, please add needed library in build system")
ENDIF (NOT HAVE_LONG_OPTIONS)

# getifaddrs ?
CHECK_FUNCTION_EXISTS (getifaddrs HAVE_GETIFADDRS)
IF (NOT HAVE_GETIFADDRS)
   MESSAGE(SEND_ERROR "The getifaddrs function is currently required by freeDiameter.")
ENDIF (NOT HAVE_GETIFADDRS)


### System checks -- for freeDiameter-host.h

# Check byte ordering
TEST_BIG_ENDIAN(HOST_BIG_ENDIAN)

# Check if ntohll is provided on the system
CHECK_SYMBOL_EXISTS(ntohll netinet/in.h HAVE_NTOHLL)

# malloc.h ?
CHECK_INCLUDE_FILES (malloc.h HAVE_MALLOC_H)

# strndup ? Missing on OS X
CHECK_FUNCTION_EXISTS (strndup HAVE_STRNDUP)


### System checks -- for includes / link

# pthreads
INCLUDE(FindThreads)
SET(CMAKE_THREAD_LIBS_INIT ${CMAKE_THREAD_LIBS_INIT} PARENT_SCOPE)

# clock_gettime
SET(HAVE_CLOCK_GETTIME "")
CHECK_FUNCTION_EXISTS (clock_gettime HAVE_NATIVE_CLOCK_GETTIME)
IF (HAVE_NATIVE_CLOCK_GETTIME)
   SET(CLOCK_GETTIME_LIBS "")
   SET(HAVE_CLOCK_GETTIME 1)
ELSE (HAVE_NATIVE_CLOCK_GETTIME)
   CHECK_LIBRARY_EXISTS (rt clock_gettime "" HAVE_LIBRT)
   IF (HAVE_LIBRT)
     SET(CLOCK_GETTIME_LIBS "-lrt")
     SET(HAVE_CLOCK_GETTIME 1)
   ELSE (HAVE_LIBRT)
     CHECK_LIBRARY_EXISTS (posix4 clock_gettime "" HAVE_LIBPOSIX4)
     IF (HAVE_LIBPOSIX4)
       SET(CLOCK_GETTIME_LIBS "-lposix4")
       SET(HAVE_CLOCK_GETTIME 1)
     ENDIF (HAVE_LIBPOSIX4)
   ENDIF (HAVE_LIBRT)
ENDIF (HAVE_NATIVE_CLOCK_GETTIME)
SET(CLOCK_GETTIME_LIBS ${CLOCK_GETTIME_LIBS} PARENT_SCOPE)

# dlopen and dlclose: CMAKE_DL_LIBS

# We need the sctp_connectx function among others
# We need the IPPROTO_SCTP symbol from sys/socket.h, netinet/in.h or netinet/sctp.h
IF(NOT DISABLE_SCTP)
	CHECK_FUNCTION_EXISTS(sctp_connectx HAVE_NATIVE_SCTP)
	IF(NOT HAVE_NATIVE_SCTP)
		# FIND_PACKAGE(SCTP REQUIRED)
	ENDIF(NOT HAVE_NATIVE_SCTP)
	# Now check the number of args of this function, since it changed between Ubuntu 9.04 and 9.10
   	SET(CHECK_SCTP_CONNECTX_4_ARGS_SOURCE_CODE "
		#include <unistd.h>
		#include <netinet/sctp.h>
		int main() {
		   return sctp_connectx(0, NULL, 0, NULL);
		}
		")
	SET(CMAKE_REQUIRED_INCLUDES ${SCTP_INCLUDE_DIR})
	SET(CMAKE_REQUIRED_LIBRARIES ${SCTP_LIBRARIES})
	CHECK_C_SOURCE_COMPILES("${CHECK_SCTP_CONNECTX_4_ARGS_SOURCE_CODE}" SCTP_CONNECTX_4_ARGS)
ELSE (NOT DISABLE_SCTP)
	MESSAGE(STATUS "Disabled SCTP support.")
ENDIF(NOT DISABLE_SCTP)
SET(SCTP_INCLUDE_DIR ${SCTP_INCLUDE_DIR} PARENT_SCOPE)
SET(SCTP_LIBRARIES ${SCTP_LIBRARIES} PARENT_SCOPE)

# IDNA process: we use GNU libidn2
IF(NOT DIAMID_IDNA_IGNORE AND NOT DIAMID_IDNA_REJECT)
	PKG_CHECK_MODULES(IDNA libidn2)
	IF(NOT IDNA_FOUND)
		MESSAGE(SEND_ERROR "Unable to find libidn2, please install libidn2-dev or equivalent, or set DIAMID_IDNA_IGNORE or DIAMID_IDNA_REJECT")
	ENDIF()
ELSE (NOT DIAMID_IDNA_IGNORE  AND NOT DIAMID_IDNA_REJECT)
	MESSAGE(STATUS "Non-default Internationalized Domain Names (IDN) behavior selected (no stringprep).")
ENDIF(NOT DIAMID_IDNA_IGNORE  AND NOT DIAMID_IDNA_REJECT)


# Require GNU TLS for building the library
FIND_PACKAGE(GNUTLS REQUIRED)
SET(GNUTLS_INCLUDE_DIR ${GNUTLS_INCLUDE_DIR} PARENT_SCOPE)
SET(GNUTLS_LIBRARIES ${GNUTLS_LIBRARIES} PARENT_SCOPE)

FIND_PATH(GCRYPT_INCLUDE_DIR NAMES gcrypt.h)
IF(NOT GCRYPT_INCLUDE_DIR)
	MESSAGE(SEND_ERROR "Unable to find gcrypt.h, please install libgcrypt-dev or equivalent")
ENDIF(NOT GCRYPT_INCLUDE_DIR)
MARK_AS_ADVANCED(GCRYPT_INCLUDE_DIR)
SET(GCRYPT_INCLUDE_DIR ${GCRYPT_INCLUDE_DIR} PARENT_SCOPE)

# Also we need libgcrypt to... display its version :(
FIND_LIBRARY(GCRYPT_LIBRARY
  NAMES gcrypt
)
IF(NOT GCRYPT_LIBRARY)
	MESSAGE(SEND_ERROR "Unable to find libgcrypt, please install libgcrypt or equivalent")
ENDIF(NOT GCRYPT_LIBRARY)
SET(GCRYPT_LIBRARY ${GCRYPT_LIBRARY} PARENT_SCOPE)


# Check if AI_ADDRCONFIG is available on the system
CHECK_SYMBOL_EXISTS(AI_ADDRCONFIG "netdb.h" HAVE_AI_ADDRCONFIG)


# Check if barriers are available (for test_fifo)
SET(CMAKE_REQUIRED_INCLUDES "pthread.h")
SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
CHECK_FUNCTION_EXISTS (pthread_barrier_wait HAVE_PTHREAD_BAR)
SET(HAVE_PTHREAD_BAR ${HAVE_PTHREAD_BAR} PARENT_SCOPE)


##########################

# Additional version when relevant, stored in version.h
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
	# Search for git binary to use
	find_package(Git)
	ADD_CUSTOM_TARGET(version_information
				COMMAND ${CMAKE_COMMAND} -D GIT_EXECUTABLE="${GIT_EXECUTABLE}" -D SRC="${CMAKE_CURRENT_SOURCE_DIR}/version.h.in" -D DST="${CMAKE_CURRENT_BINARY_DIR}/version.h" -P "${CMAKE_SOURCE_DIR}/cmake/Modules/GetVersionWithGit.cmake"
				COMMENT "Retrieving version of the git repository"
				)
else (EXISTS "${CMAKE_SOURCE_DIR}/.git")
	# We use the pure version number without extension
	CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
	ADD_CUSTOM_TARGET(version_information DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/version.h)
endif (EXISTS "${CMAKE_SOURCE_DIR}/.git")


##########################

# LFDPROTO_LIBS = libraries required by the libfdproto.
SET(LFDPROTO_LIBS ${CLOCK_GETTIME_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${IDNA_LINK_LIBRARIES} PARENT_SCOPE)
# And includes paths
SET(LFDPROTO_INCLUDES ${IDNA_INCLUDE_DIRS} PARENT_SCOPE)
# Dependencies: the libraries required by any code linking to libfdproto.
SET(LFDPROTO_LINK_INTERFACES ${CMAKE_THREAD_LIBS_INIT} PARENT_SCOPE)

# LFDCORE_LIBS = libraries required by the libfdcore (in addition to libfdproto and its dependencies)
SET(LFDCORE_LIBS ${CLOCK_GETTIME_LIBS} ${CMAKE_DL_LIBS} ${SCTP_LIBRARIES} ${GCRYPT_LIBRARY} ${GNUTLS_LIBRARIES} PARENT_SCOPE)
# And includes paths
SET(LFDCORE_INCLUDES ${SCTP_INCLUDE_DIR} ${GNUTLS_INCLUDE_DIR} ${GCRYPT_INCLUDE_DIR} PARENT_SCOPE)
# And dependencies
SET(LFDCORE_LINK_INTERFACES "" PARENT_SCOPE)
		# We don't force other libraries, the programs will link with what it needs
                # (such as libgnutls if it uses GNUTLS_DEBUG() macro
		# or libfdproto if it uses some of its interfaces directly)
		# See freeDiameterd/CMakeLists.txt for an example.

##########################

# Generate the host.h file
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/freeDiameter-host.h.in ${CMAKE_CURRENT_BINARY_DIR}/freeDiameter-host.h)

####
## INSTALL section ##

# The headers from this directory are required to develop new extensions for freeDiameter.
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/freeDiameter-host.h libfdproto.h libfdcore.h extension.h
	DESTINATION ${INSTALL_HEADERS_SUFFIX}
	COMPONENT freeDiameter-dev)
