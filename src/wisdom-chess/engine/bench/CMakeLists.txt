CPMAddPackage("gh:martinus/nanobench@4.3.11")

add_executable(wisdom-chess-benchmarks
    bench_main.cpp
    bench_move_generation.cpp
    bench_threats.cpp
    bench_legality.cpp
    bench_perft.cpp)

target_link_libraries(wisdom-chess-benchmarks PRIVATE wisdom::chess)
target_link_libraries(wisdom-chess-benchmarks PRIVATE nanobench)

if (PCH_ENABLED)
    target_precompile_headers(wisdom-chess-benchmarks PRIVATE PRIVATE ../global.hpp)
endif()
