set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  ExecutionEngine
  InstCombine
  Object
  OrcJIT
  RuntimeDyld
  ScalarOpts
  Support
  TransformUtils
  native
  )

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../build)

add_kaleidoscope_chapter(jhini
  main.cpp
)

export_executable_symbols(jhini)

set_target_properties(jhini PROPERTIES
                      CXX_STANDARD 17
                      CXX_EXTENSIONS OFF
                      )

target_compile_options(jhini PUBLIC -frtti)
