#
# Targets
#

add_library(vectcom 
    PolyCrypto.cpp
    KatePublicParameters.cpp
    Kzg.cpp
    PolyOps.cpp
    Utils.cpp
)

target_include_directories(vectcom PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
    "$<INSTALL_INTERFACE:${INSTALL_CMAKE_DIR}/include>"
)

target_link_libraries(vectcom PUBLIC
    ntl ff pthread zm gmp gmpxx xutils xassert
)

include(FindOpenMP)
if(OPENMP_FOUND)
    target_link_libraries(vectcom PUBLIC gomp)
endif()

#target_link_libraries(vectcom PUBLIC Threads::Threads)

#
# Installation
# TODO: Add <Package>Config[Version].cmake files so this package can be easily imported?
# (See https://cmake.org/cmake/help/git-master/manual/cmake-packages.7.html#creating-packages)
#

# This creates the <Package>Config.cmake file and installs it
install(TARGETS vectcom EXPORT vectcomConfig 
   ARCHIVE DESTINATION lib)
install(EXPORT vectcomConfig DESTINATION lib/cmake/vectcom)

# This installs the static or (/and?) dynamic library
install(TARGETS vectcom 
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

# This installs the headers
# WARNING: Don't add / at the end. No slash means vectcom/ directory is created in the destination path
install(DIRECTORY include/vectcom DESTINATION include)
