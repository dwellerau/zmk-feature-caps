# zephyr/CMakeLists.txt
# Build this repo as a Zephyr module library.

# 1) Create a library target for the module
zephyr_library()

# 2) Expose public headers to users as: #include <zmk/feature_caps.h>
zephyr_include_directories(${CMAKE_CURRENT_LIST_DIR}/../include)

# 3) Add sources from ../src (switch to explicit list if you prefer)
file(GLOB MODULE_SOURCES ${CMAKE_CURRENT_LIST_DIR}/../src/*.c)
if(MODULE_SOURCES)
  zephyr_library_sources(${MODULE_SOURCES})
endif()

# ---- Optional patterns you may want later ----
# Only build some files when a Kconfig is enabled:
# zephyr_library_sources_ifdef(CONFIG_ZMK_FEATURE_CAPS
#   ${CMAKE_CURRENT_LIST_DIR}/../src/feature_caps.c
# )

# If you add C++ sources, Zephyr handles them automatically.
# If you depend on other Zephyr libs, you could link them:
# zephyr_library_link_libraries(some_other_lib)