set(_common_libraries
  ActsCore
  ActsExamplesFramework
  ActsExamplesCommon
  ActsExamplesGenerators
  ActsExamplesGeneratorsPythia8
  ActsExamplesTruthTracking
  ActsExamplesVertexing
  ttlindkvistACTS
  Boost::program_options)

add_executable(
  ntupleReader
  ntupleReader.cpp)
target_link_libraries(
  ntupleReader
  PRIVATE ${_common_libraries} )
target_include_directories(
  ntupleReader
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
  
add_executable(
    AMVFvsIterativeVertexFinder
    AMVFvsIterativeVertexFinder.cpp)
target_include_directories(
  AMVFvsIterativeVertexFinder
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_link_libraries(
    AMVFvsIterativeVertexFinder
    PRIVATE ${_common_libraries} )

install(
  TARGETS
    ntupleReader
    AMVFvsIterativeVertexFinder
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
