set(INCLUDE
        component_array.h
        component_manager.h
        coordinator.h
        entity.h
        entity_manager.h
        system.h
        system_manager.h)

set(SOURCE
        component_manager.cpp
        coordinator.cpp
        entity_manager.cpp
        system.cpp)

add_library(ECS STATIC ${SOURCE} ${INCLUDE})

target_include_directories(Renderer PUBLIC .)
