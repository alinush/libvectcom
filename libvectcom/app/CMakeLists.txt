set(vectcom_app_sources
    ParamsGenTrapdoors.cpp
    ParamsGenPowers.cpp
    ParamsValidate.cpp
)

foreach(appSrc ${vectcom_app_sources})
    get_filename_component(appName ${appSrc} NAME_WE)
    set(appDir ../bin/app)

    add_executable(${appName} ${appSrc})
    target_link_libraries(${appName} PUBLIC vectcom)

    set_target_properties(${appName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${appDir})
endforeach()

#install(TARGETS PlayApp DESTINATION bin)
