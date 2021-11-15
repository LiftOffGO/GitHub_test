cmake_minimum_required(VERSION 3.0)

set(ObjName mytest)

project(${ObjName})


include_directories(include) 

aux_source_directory(src SRC_LIST)  

add_executable(${ObjName} main.cpp ${SRC_LIST}) 
find_package(Threads REQUIRED)
target_link_libraries(${ObjName} Threads::Threads)