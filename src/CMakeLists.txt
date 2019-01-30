#    /cpp-readline/src/CMakeLists.txt
#
#    @author zmij
#    @date May 17, 2016

cmake_minimum_required(VERSION 2.6)

set(xma_SRCS
	xma_timer.cpp
	xma_service.cpp
	xma_listener.cpp
  xma_thread.cpp
	xma_application.cpp
	xma_shell.cpp
	xma_process.cpp
	xma_epoll.cpp
	xma_socket.cpp
)

add_library(${lib_name} SHARED ${xma_SRCS})
set_target_properties(
    ${lib_name} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)
target_link_libraries(${lib_name} ${Readline_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} event)
