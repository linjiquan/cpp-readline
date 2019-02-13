#    /cpp-readline/example/CMakeLists.txt
#
#    @author brant
#    @date Jan 21, 2019

cmake_minimum_required(VERSION 2.6)

add_executable(client main.cpp client_process.cpp client_service.cpp)
target_link_libraries(client ${lib_name})
