set(LLVM_LINK_COMPONENTS
    all
  )
  
add_definitions(-DCOMPILER -DJHIN_DEBUG)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../build)

add_kaleidoscope_chapter(jhin
  main.cpp
)

export_executable_symbols(jhin)

set_target_properties(jhin PROPERTIES
                      CXX_STANDARD 17
                      CXX_EXTENSIONS OFF
                      )

target_compile_options(jhin PUBLIC -frtti)
