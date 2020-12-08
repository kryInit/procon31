add_executable(tileSet tileSet.cpp classes.hpp constant.hpp io.cpp enumeratePath.cpp utility.cpp utility.hpp)
add_executable(areaAndNdfs areaAndNdfs.cpp classes.hpp constant.hpp io.cpp enumeratePath.cpp utility.cpp utility.hpp)

set_target_properties( tileSet
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "../../../bin/"
)

set_target_properties( areaAndNdfs
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "../../../bin/"
)
