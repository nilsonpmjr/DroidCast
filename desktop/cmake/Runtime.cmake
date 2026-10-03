# Build-time dependency acquisition. The application never downloads executables.
# URLs and hashes are pinned in this fork's doc/{linux,windows,macos}.md.
if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(runtime_platform win64)
  set(runtime_ext zip)
  set(runtime_hash 5b12172b3264b2889f4583ee64752ce832e29bc8b1089dca81093459697165db)
elseif(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
  set(runtime_platform macos-aarch64)
  set(runtime_ext tar.gz)
  set(runtime_hash 20fd47c9014dd5e0fa77091f3cb7adbda8445a360c4584aeaa0150b5b3988ff3)
elseif(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
  set(runtime_platform macos-x86_64)
  set(runtime_ext tar.gz)
  set(runtime_hash ee2a7223bc8dbdc4f482db1134bcf441178dafb833492b71ca4c22090c58ce72)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
  set(runtime_platform linux-x86_64)
  set(runtime_ext tar.gz)
  set(runtime_hash ad56ae8bfeedf41e824945c11dbf55fcb092b3e615b9b486f48a50e30d389635)
else()
  message(FATAL_ERROR "No pinned runtime for this target. Add a verified platform bundle before distributing DroidCast.")
endif()
set(runtime_archive "scrcpy-${runtime_platform}-v4.1.${runtime_ext}")
set(runtime_download "${CMAKE_CURRENT_BINARY_DIR}/downloads/${runtime_archive}")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/downloads")
if(EXISTS "${runtime_download}")
  file(SHA256 "${runtime_download}" existing_hash)
endif()
if(NOT existing_hash STREQUAL runtime_hash)
  message(STATUS "Downloading pinned DroidCast runtime dependencies (scrcpy 4.1)")
  file(DOWNLOAD "https://github.com/Genymobile/scrcpy/releases/download/v4.1/${runtime_archive}"
    "${runtime_download}" EXPECTED_HASH "SHA256=${runtime_hash}" TLS_VERIFY ON SHOW_PROGRESS)
endif()
file(ARCHIVE_EXTRACT INPUT "${runtime_download}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/runtime-source")
set(runtime_source "${CMAKE_CURRENT_BINARY_DIR}/runtime-source/scrcpy-${runtime_platform}-v4.1")
file(COPY "${runtime_source}/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/runtime" USE_SOURCE_PERMISSIONS)
if(DROIDCAST_BUILD_FORK)
  find_program(MESON_EXECUTABLE meson REQUIRED)
  find_program(NINJA_EXECUTABLE ninja REQUIRED)
  include(ExternalProject)
  ExternalProject_Add(droidcast-engine
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.."
    BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/engine-build"
    DOWNLOAD_COMMAND ""
    CONFIGURE_COMMAND "${MESON_EXECUTABLE}" setup <BINARY_DIR> <SOURCE_DIR>
      --buildtype=release -Dportable=true -Dv4l2=false
      "-Dprebuilt_server=${runtime_source}/scrcpy-server"
    BUILD_COMMAND "${NINJA_EXECUTABLE}" -C <BINARY_DIR>
    BUILD_ALWAYS TRUE
    INSTALL_COMMAND "${CMAKE_COMMAND}" -E copy_if_different
      "<BINARY_DIR>/app/scrcpy${CMAKE_EXECUTABLE_SUFFIX}"
      "${CMAKE_CURRENT_BINARY_DIR}/runtime/scrcpy${CMAKE_EXECUTABLE_SUFFIX}")
  add_dependencies(droidcast-desktop droidcast-engine)
endif()
add_custom_command(TARGET droidcast-desktop POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
    "${CMAKE_CURRENT_BINARY_DIR}/runtime" "$<TARGET_FILE_DIR:droidcast-desktop>/runtime")
# Bundle runtime beside the app binary, including inside a macOS .app.
