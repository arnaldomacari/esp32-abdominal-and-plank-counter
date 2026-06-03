# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/ajm/esp/esp-idf/components/bootloader/subproject"
  "/home/ajm/Projetos/esp32-abdominal-and-plank-counter/firmware/motion_counter/build/bootloader"
  "/home/ajm/Projetos/esp32-abdominal-and-plank-counter/firmware/motion_counter/build/bootloader-prefix"
  "/home/ajm/Projetos/esp32-abdominal-and-plank-counter/firmware/motion_counter/build/bootloader-prefix/tmp"
  "/home/ajm/Projetos/esp32-abdominal-and-plank-counter/firmware/motion_counter/build/bootloader-prefix/src/bootloader-stamp"
  "/home/ajm/Projetos/esp32-abdominal-and-plank-counter/firmware/motion_counter/build/bootloader-prefix/src"
  "/home/ajm/Projetos/esp32-abdominal-and-plank-counter/firmware/motion_counter/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/ajm/Projetos/esp32-abdominal-and-plank-counter/firmware/motion_counter/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/ajm/Projetos/esp32-abdominal-and-plank-counter/firmware/motion_counter/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
