# zephyr/CMakeLists.txt
# Build this repository as a Zephyr module library.

# 1) Create a library target for this module
zephyr_library()

# 2) Expose public headers so users can: #include <zmk/feature_caps.h>
zephyr_include_directories(${CMAKE_CURRENT_LIST_DIR}/../include)

# 3) Add sources from ../src (switch to an explicit list if you prefer)
file(GLOB MODULE_SOURCES ${CMAKE_CURRENT_LIST_DIR}/../src/*.c)
if(MODULE_SOURCES)
  zephyr_library_sources(${MODULE_SOURCES})
endif()

# 4) Inherit the application target’s include paths (brings in zmk/app/include),
#    which avoids missing-headers issues if a workflow injects EXTRA_MODULES.
zephyr_library_link_libraries(app)

# ---- Optional patterns you may enable later ----
# Only compile specific sources when a Kconfig option is enabled:
# zephyr_library_sources_ifdef(CONFIG_ZMK_FEATURE_CAPS
#   ${CMAKE_CURRENT_LIST_DIR}/../src/feature_caps.c
# )
