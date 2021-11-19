cmake_minimum_required(VERSION 3.1.0)
# ${LLVM_DIR}
project(assign2)
find_package(LLVM REQUIRED CONFIG HINTS /usr/local/llvm10d /usr/local/llvm10d/lib/cmake/llvm
	                NO_DEFAULT_PATH)

include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS} SYSTEM)
link_directories(${LLVM_LIBRARY_DIRS})
message(STATUS "LLVM_LIB DIR : ${LLVM_LIBRARY_DIRS}")
set(LLVM_LINK_COMPONENTS
  LLVMCore
  LLVMIRReader
  LLVMPasses
  )

message(STATUS "LLVM LIBS : ${LLVM_LINK_COMPONENTS}")
# Support plugins.

#set(SOURCES
#  lib/Main.cpp
#  lib/NodeFactory.cpp
#  lib/ConstraintCollect.cpp
#  lib/ConstraintSolve.cpp
#)
file(GLOB SOURCES "lib/*.cpp")

add_executable(anderson ${SOURCES})

target_link_libraries(anderson
	${LLVM_LINK_COMPONENTS}
	)
target_include_directories(anderson
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)