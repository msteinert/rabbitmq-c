project(librabbitmq "C")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Stuff dealing with code generation
set(AMQP_CODEGEN_PY "${CMAKE_CURRENT_BINARY_DIR}/amqp_codegen.py")
set(CODEGEN_PY "${CMAKE_CURRENT_BINARY_DIR}/codegen.py")
set(AMQP_SPEC_JSON_PATH "${AMQP_CODEGEN_DIR}/amqp-rabbitmq-0.9.1.json")

#generate amqp_framing.h/amqp_framing.c
add_custom_command(
  OUTPUT ${CODEGEN_PY}
  COMMAND ${CMAKE_COMMAND} ARGS -E copy ${CMAKE_CURRENT_SOURCE_DIR}/codegen.py ${CODEGEN_PY}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/codegen.py
  VERBATIM)

add_custom_command(
  OUTPUT ${AMQP_CODEGEN_PY}
  COMMAND ${CMAKE_COMMAND} ARGS -E copy ${AMQP_CODEGEN_DIR}/amqp_codegen.py ${AMQP_CODEGEN_PY}
  DEPENDS ${AMQP_CODEGEN_DIR}/amqp_codegen.py ${AMQP_CODEGEN_TARGET}
  VERBATIM)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/amqp_framing.h
  COMMAND ${PYTHON_EXECUTABLE} ARGS ${CODEGEN_PY} header ${AMQP_SPEC_JSON_PATH} ${CMAKE_CURRENT_BINARY_DIR}/amqp_framing.h
  DEPENDS ${AMQP_SPEC_JSON_PATH} ${CODEGEN_PY} ${AMQP_CODEGEN_PY}
  VERBATIM)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/amqp_framing.c
  COMMAND ${PYTHON_EXECUTABLE} ARGS ${CODEGEN_PY} body ${AMQP_SPEC_JSON_PATH} ${CMAKE_CURRENT_BINARY_DIR}/amqp_framing.c
  DEPENDS ${AMQP_SPEC_JSON_PATH} ${CODEGEN_PY} ${AMQP_CODEGEN_PY}
  VERBATIM)


SET(CONFIG_CONTENTS "#define VERSION \"${VERSION}\"
#ifndef __cplusplus
# define inline ${C_INLINE}
#endif /* __cplusplus */
")

#prepare config.h
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/config.h" ${CONFIG_CONTENTS})

if(WIN32)
  set(SOCKET_IMPL "win32")
else(WIN32)
  set(SOCKET_IMPL "unix")
endif(WIN32)

if(MSVC)
  if(MSVC_VERSION LESS 1600)
    set(MSINTTYPES_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/win32/msinttypes")
    set(STDINT_H_INSTALL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/win32/msinttypes/stdint.h")
  endif(MSVC_VERSION LESS 1600)
endif(MSVC)

set(LIBRABBITMQ_INCLUDE_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${SOCKET_IMPL}
	${MSINTTYPES_INCLUDE}
	)

include_directories(${LIBRABBITMQ_INCLUDE_DIRS})

set(LIBRABBITMQ_INCLUDE_DIRS
	${LIBRABBITMQ_INCLUDE_DIRS}
	PARENT_SCOPE)

add_definitions(-DHAVE_CONFIG_H)

if (ENABLE_SSL_SUPPORT)
  add_definitions(-DWITH_SSL=1)

  if (SSL_ENGINE STREQUAL "OpenSSL")
    set(AMQP_SSL_SRCS amqp-ssl-socket.h amqp-openssl.c)
    include_directories(${OPENSSL_INCLUDE_DIR})
    set(AMQP_SSL_LIBS ${OPENSSL_LIBRARIES})

  elseif (SSL_ENGINE STREQUAL "cyaSSL")
    set(AMQP_SSL_SRCS amqp-ssl-socket.h amqp-cyassl.c)
    include_directories(${CYASSL_INCLUDE_DIR})
    set(AMQP_SSL_LIBS ${CYASSL_LIBRARIES})

  elseif (SSL_ENGINE STREQUAL "GnuTLS")
    set(AMQP_SSL_SRCS amqp-ssl-socket.h amqp-gnutls.c)
    include_directories(${GNUTLS_INCLUDE_DIR})
    add_definitions(${GNUTLS_DEFINITIONS})
    set(AMQP_SSL_LIBS ${GNUTLS_LIBRARIES})

  elseif (SSL_ENGINE STREQUAL "PolarSSL")
    set(AMQP_SSL_SRCS amqp-ssl-socket.h amqp-polarssl.c)
    include_directories(${POLARSSL_INCLUDE_DIR})
    set(AMQP_SSL_LIBS ${POLARSSL_LIBRARIES})

  else()
    message(FATAL_ERROR "Unknown SSL_ENGINE ${SSL_ENGINE}")
  endif()

  if (ENABLE_THREAD_SAFETY)
    add_definitions(-DENABLE_THREAD_SAFETY)
    if (WIN32)
      set(AMQP_SSL_SRCS ${AMQP_SSL_SRCS} win32/threads.h win32/threads.c)
    else()
      set(AMQP_SSL_SRCS ${AMQP_SSL_SRCS} unix/threads.h)
    endif()
  endif()
endif()

set(RABBITMQ_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/amqp_framing.h
    ${CMAKE_CURRENT_BINARY_DIR}/amqp_framing.c
    amqp_api.c  amqp.h
    amqp_connection.c  amqp_mem.c  amqp_private.h  amqp_socket.c  amqp_table.c
    amqp_url.c amqp-socket.h amqp-tcp-socket.c amqp-tcp-socket.h
    ${SOCKET_IMPL}/socket.h ${SOCKET_IMPL}/socket.c
    ${AMQP_SSL_SRCS}
)

add_definitions(-DAMQP_BUILD)

if (NOT BUILD_SHARED_LIBS)
  add_definitions(-DAMQP_STATIC)
endif()

add_library(rabbitmq ${RABBITMQ_SOURCES})

if (ENABLE_SSL_SUPPORT)
  target_link_libraries(rabbitmq ${AMQP_SSL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()

if(WIN32)
  target_link_libraries(rabbitmq ws2_32)
endif(WIN32)

install(TARGETS rabbitmq
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	PUBLIC_HEADER DESTINATION include)

install(FILES amqp.h ${CMAKE_CURRENT_BINARY_DIR}/amqp_framing.h ${STDINT_H_INSTALL_FILE}
	DESTINATION include)


