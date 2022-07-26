add_library(
  ttlindkvistACTS SHARED
  src/ModifiedAMVFAlgorithm.cpp
  src/ModifiedIterativeVertexFinderAlgorithm.cpp
  src/NTupleReaderOptions.cpp
  src/RootNTupleReader.cpp
)
target_include_directories(
  ttlindkvistACTS
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
  
set(_common_libraries
  ActsCore
  ActsExamplesFramework
  ActsExamplesCommon
  ActsExamplesGenerators
  ActsExamplesGeneratorsPythia8
  ActsExamplesTruthTracking
  ActsExamplesVertexing
  Boost::program_options)
  
target_link_libraries(
  ttlindkvistACTS
  PUBLIC
    ActsCore ${_common_libraries} ActsPluginIdentification ActsExamplesDigitization
    ActsExamplesFramework ActsExamplesPropagation ActsExamplesTruthTracking ActsExamplesMaterialMapping
    Threads::Threads
  PRIVATE ROOT::Core ROOT::Hist ROOT::Tree ActsExamplesTruthTracking)

install(
  TARGETS ttlindkvistACTS
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
