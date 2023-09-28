
cmake_minimum_required(VERSION 3.0)

add_subdirectory(libio)
add_subdirectory(libraw)
add_subdirectory(libjsonreader)

add_subdirectory(consoleapp)
add_subdirectory(signaturetestconsole)
add_subdirectory(rawrecoveryconsole)

#add_subdirectory(RawRecovery)

project("RecoveryTools")