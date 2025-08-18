# zmk-feature-caps /zephyr/CMakeLists.txt

zephyr_library()

# Module headers
zephyr_include_directories(${CMAKE_CURRENT_LIST_DIR}/../include)

# App/Zephyr headers (robust to odd workflows)
zephyr_include_directories(${CMAKE_SOURCE_DIR}/include)
zephyr_library_link_libraries(zephyr_interface)

# Build caps_status.c only when both the display stack and the feature are enabled.
# (Prevents lvgl.h include errors on non-display builds like `settings_reset`.)
if(CONFIG_ZMK_DISPLAY AND CONFIG_ZMK_FEATURE_CAPS_WORD_INDICATOR)
  zephyr_library_sources(${CMAKE_CURRENT_LIST_DIR}/../src/caps_status.c)
endif()