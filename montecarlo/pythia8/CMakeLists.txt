############################################################################
# CMakeLists.txt file for building ROOT montecarlo/pythia8 package
# @author Pere Mato, CERN
############################################################################

ROOT_STANDARD_LIBRARY_PACKAGE(EGPythia8
  HEADERS
    TPythia8.h
    TPythia8Decayer.h
  SOURCES
    TPythia8.cxx
    TPythia8Decayer.cxx
  LIBRARIES
    ${PYTHIA8_LIBRARIES}
  DEPENDENCIES
    Core
    EG
    Graf
    Physics
    VMC
)

ROOT_ADD_CXX_FLAG(_EGPythia8_FLAGS -Wno-delete-non-virtual-dtor)
ROOT_ADD_CXX_FLAG(_EGPythia8_FLAGS -Wno-misleading-indentation)
ROOT_ADD_CXX_FLAG(_EGPythia8_FLAGS -Wno-overloaded-virtual)

separate_arguments(_EGPythia8_FLAGS)

target_compile_options(EGPythia8 PRIVATE ${_EGPythia8_FLAGS})
