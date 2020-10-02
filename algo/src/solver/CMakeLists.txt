add_executable(solver solver.cpp classes.hpp constant.hpp io.cpp enumeratePath.cpp utility.cpp utility.hpp)

set_target_properties( solver
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "../../../bin/"
)

