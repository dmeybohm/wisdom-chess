add_library(wisdom-viewmodel-base STATIC
    game_viewmodel_base.cpp
    viewmodel_types.cpp
    viewmodel_settings.cpp
)

target_include_directories(wisdom-viewmodel-base
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

target_link_libraries(wisdom-viewmodel-base PUBLIC wisdom::chess)
add_library(wisdom::viewmodel ALIAS wisdom-viewmodel-base)
