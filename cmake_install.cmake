# Install script for directory: D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/JUCE")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/build/modules/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/build/extras/Build/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/JUCE-8.0.15" TYPE FILE FILES
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/build/JUCEConfigVersion.cmake"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/build/JUCEConfig.cmake"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/FindCppwinrt.cmake"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/FindWebView2.cmake"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/FindWindowsMIDIServices.cmake"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/JUCECheckAtomic.cmake"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/JUCEHelperTargets.cmake"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/JUCEModuleSupport.cmake"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/JUCEUtils.cmake"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/JuceLV2Defines.h.in"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/LaunchScreen.storyboard"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/PIPAudioProcessor.cpp.in"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/PIPAudioProcessorWithARA.cpp.in"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/PIPComponent.cpp.in"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/PIPConsole.cpp.in"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/RecentFilesMenuTemplate.nib"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/UnityPluginGUIScript.cs.in"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/checkBundleSigning.cmake"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/copyDir.cmake"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/juce_runtime_arch_detection.cpp"
    "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/juce_LinuxSubprocessHelper.cpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/JUCE-8.0.15" TYPE DIRECTORY FILES "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/JUCE/extras/Build/CMake/juce_vst3_helper")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/JUCE-8.0.15" TYPE EXECUTABLE FILES "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/build/juce_lv2_helper.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/build/CMakeFiles/juce_lv2_helper.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/JUCE-8.0.15/LV2_HELPER.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/JUCE-8.0.15/LV2_HELPER.cmake"
         "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/build/CMakeFiles/Export/18a172b556007736bae03eb13c621b9f/LV2_HELPER.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/JUCE-8.0.15/LV2_HELPER-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/JUCE-8.0.15/LV2_HELPER.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/JUCE-8.0.15" TYPE FILE FILES "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/build/CMakeFiles/Export/18a172b556007736bae03eb13c621b9f/LV2_HELPER.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/JUCE-8.0.15" TYPE FILE FILES "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/build/CMakeFiles/Export/18a172b556007736bae03eb13c621b9f/LV2_HELPER-release.cmake")
  endif()
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/build/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "D:/a/SALEK_HIGHTECH/SALEK_HIGHTECH/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
