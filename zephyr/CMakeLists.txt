# zephyr/CMakeLists.txt
zephyr_library()

# Public headers of your module
zephyr_include_directories(${CMAKE_CURRENT_LIST_DIR}/../include)

# ✅ Explicitly add ZMK app headers (gives you zmk/event_manager.h, etc.)
zephyr_include_directories(${CMAKE_SOURCE_DIR}/include)

# (keep your sources)
file(GLOB MODULE_SOURCES ${CMAKE_CURRENT_LIST_DIR}/../src/*.c)
if(MODULE_SOURCES)
  zephyr_library_sources(${MODULE_SOURCES})
endif()

# Helpful to inherit global Zephyr interface include dirs too
zephyr_library_link_libraries(zephyr_interface)