set_xmakever("2.8.2")
set_project("argparse")

set_version("2.9.0", { build = "%Y%m%d%H%M" })

option("enable_module")
option("enable_std_import", { defines = "ARGPARSE_MODULE_USE_STD_MODULE" })
option("enable_tests")
option("enable_samples")

add_cxxflags(
    "-Wall",
    "-Wno-long-long",
    "-pedantic",
    "-Wsign-conversion",
    "-Wshadow",
    "-Wconversion",
    { toolsets = { "clang", "gcc" } }
)
add_cxxflags("cl::/W4")

if is_plat("windows") then
    add_defines("_CRT_SECURE_NO_WARNINGS")
end

target("argparse", function()
    set_languages("c++17")
    set_kind("headeronly")
    if get_config("enable_module") then
        set_languages("c++20")
        set_kind("static") -- static atm because of a XMake bug, headeronly doesn't generate package module metadata
    end

    add_options("enable_std_import")

    add_includedirs("include", { public = true })
    add_headerfiles("include/argparse/argparse.hpp")
    if get_config("enable_module") then
        add_files("module/argparse.cppm", { install = true })
    end
end)

if get_config("enable_tests") then
    target("argparse_tests", function()
        set_kind("binary")
        set_languages("c++17")
        set_basename("tests")

        add_includedirs("test")

        add_files("test/main.cpp", { defines = { "DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN" } })
        add_files("test/**.cpp")

        add_deps("argparse")
    end)

    if get_config("enable_module") then
        target("argparse_module_tests", function()
            set_kind("binary")
            set_languages("c++20")
            set_basename("module_tests")

            add_defines("WITH_MODULE")

            add_includedirs("test")

            add_files("test/main.cpp", { defines = { "DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN" } })
            add_files("test/**.cpp")
            add_files("test/argparse_details.cppm")

            add_deps("argparse")
        end)
    end
end

if get_config("enable_samples") then
    for _, sample_file in ipairs(os.files("samples/*.cpp")) do
        target(path.basename(sample_file), function()
            set_kind("binary")
            set_languages("c++17")

            add_files(sample_file)

            set_policy("build.c++.modules", false)

            add_deps("argparse")
        end)
    end
end
