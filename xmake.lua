add_rules("mode.debug", "mode.release", "mode.coverage")
add_requires("doctest", "abseil", "fmt", "nanobench", "spdlog")

set_languages("c++20")
add_includedirs("include", "../digraphx-cpp/include", "../ellalgo-cpp/include",
                "../py2cpp/include", "../xnetwork-cpp/include")

if is_mode("release") then
    set_optimize("fastest")
end

if is_mode("coverage") and not is_plat("windows") then
    add_cxflags("-ftest-coverage", "-fprofile-arcs", {force = true})
    add_ldflags("-fprofile-arcs", {force = true})
end

if is_plat("linux") or is_plat("macosx") then
    set_warnings("all", "error")
    -- Termux/Android ships its libc++ headers under $PREFIX
    local prefix = os.getenv("PREFIX")
    if prefix then
        add_cxflags("-Wno-unused-command-line-argument", {force = true})
        add_sysincludedirs(prefix .. "/include/c++/v1", prefix .. "/include", {public = true})
    end
elseif is_plat("windows") then
    add_cxflags("/EHsc /utf-8 /W4 /WX /wd4702", {force = true})
    add_defines("_SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS")
end

-- ellalgo-cpp cutting-plane driver lives there
local ellalgo_dir = path.join(os.projectdir(), "../ellalgo-cpp")
local ellalgo_inc = path.join(ellalgo_dir, "include")

target("EllAlgo")
    set_kind("static")
    add_includedirs(ellalgo_inc, {public = true})
    add_files(path.join(ellalgo_dir, "source/*.cpp"))
    add_packages("fmt", "spdlog")
    set_group("Dependencies")

target("test_netoptim")
    add_deps("EllAlgo")
    add_files("test/source/*.cpp")
    add_packages("doctest", "abseil")
    add_tests("default")

target("bench_neg_cycle")
    set_default(false)
    add_files("benchmark/source/bench_neg_cycle.cpp")
    add_packages("abseil", "nanobench", "fmt")

target("bench_network_oracle")
    set_default(false)
    add_files("benchmark/source/bench_network_oracle.cpp")
    add_packages("abseil", "nanobench", "fmt")

target("bench_optscaling")
    set_default(false)
    add_deps("EllAlgo")
    add_files("benchmark/source/bench_optscaling.cpp")
    add_packages("abseil", "nanobench")
