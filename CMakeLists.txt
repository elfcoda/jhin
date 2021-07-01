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

add_kaleidoscope_chapter(jhin
  main.cpp
  )

export_executable_symbols(jhin)

set_target_properties(jhin PROPERTIES
                      CXX_STANDARD 17
                      CXX_EXTENSIONS OFF
                      )

# target_compile_options(jhin PUBLIC -frtti)

