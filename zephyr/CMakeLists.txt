zephyr_library()

# your module headers
zephyr_include_directories(${CMAKE_CURRENT_LIST_DIR}/../include)

# app/zephyr headers (robust to odd workflows)
zephyr_include_directories(${CMAKE_SOURCE_DIR}/include)
zephyr_library_link_libraries(zephyr_interface)

# ⬇️ Build caps_status.c only when ZMK's Caps Word is available
zephyr_library_sources_ifdef(CONFIG_ZMK_FEATURE_CAPS_WORD_INDICATOR
  ${CMAKE_CURRENT_LIST_DIR}/../src/caps_status.c
)

# If you have other files that don't need Caps Word, list them unconditionally:
# zephyr_library_sources(${CMAKE_CURRENT_LIST_DIR}/../src/other_feature.c)
