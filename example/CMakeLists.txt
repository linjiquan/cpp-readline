#    /cpp-readline/example/CMakeLists.txt
#
#    @author brant
#    @date Jan 21, 2019

cmake_minimum_required(VERSION 2.6)

add_executable(cpp_readline main.cpp)
target_link_libraries(cpp_readline ${lib_name} event)
