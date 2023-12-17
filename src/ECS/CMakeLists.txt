target_include_directories(RTracer PUBLIC .)

set(SOURCE
        component_manager.cpp
        coordinator.cpp
        entity_manager.cpp
        system.cpp)

set(INCLUDE
        component_array.h
        component_manager.h
        coordinator.h
        entity.h
        entity_manager.h
        system.h
        system_manager.h)

add_library(ECS STATIC ${SOURCE} ${INCLUDE})