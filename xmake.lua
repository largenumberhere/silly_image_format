add_requires("raylib 5.5")
add_requires("raygui 4.0");

target("convert")
    add_headerfiles("src/common.hpp")
    add_files("src/convert.cpp", "src/common.cpp")
    set_rundir("$(projectdir)/")
    set_kind("binary")

    add_packages("raylib")
    add_packages("raygui")
    set_languages("c11", "c++23")
    set_symbols("debug")
    
    set_optimize("fastest")
    
target("view")
    add_headerfiles("src/common.hpp")
    add_files("src/view.cpp", "src/common.cpp")
    set_rundir("$(projectdir)/")
    set_kind("binary")

    add_packages("raylib")
    add_packages("raygui")
    set_languages("c11", "c++23")
    set_symbols("debug")
    
    set_optimize("fastest")
    
