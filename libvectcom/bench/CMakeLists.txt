set(vectcom_bench_sources
    BenchAMT.cpp
    BenchConvertAndMultiexp.cpp
    BenchExp.cpp
    BenchFFT.cpp
    BenchFk.cpp
    BenchMultiexp.cpp
    BenchNtlConversion.cpp
    BenchPairing.cpp
    BenchPointproofs.cpp
    BenchPolyDivideXnc.cpp
    BenchPolynomialOps.cpp
    BenchRootsOfUnityEval.cpp
)

foreach(appSrc ${vectcom_bench_sources})
    get_filename_component(appName ${appSrc} NAME_WE)
    set(appDir ../bin/bench)

    add_executable(${appName} ${appSrc})
    target_link_libraries(${appName} PRIVATE vectcom)

    set_target_properties(${appName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${appDir})
endforeach()
