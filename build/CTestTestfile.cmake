# CMake generated Testfile for 
# Source directory: C:/Users/shahk/OneDrive/Documents/ProgrammingProjects/CHIP8-Emulator
# Build directory: C:/Users/shahk/OneDrive/Documents/ProgrammingProjects/CHIP8-Emulator/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(opcodes "C:/Users/shahk/OneDrive/Documents/ProgrammingProjects/CHIP8-Emulator/build/Debug/chip8_tests.exe")
  set_tests_properties(opcodes PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/shahk/OneDrive/Documents/ProgrammingProjects/CHIP8-Emulator/CMakeLists.txt;26;add_test;C:/Users/shahk/OneDrive/Documents/ProgrammingProjects/CHIP8-Emulator/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(opcodes "C:/Users/shahk/OneDrive/Documents/ProgrammingProjects/CHIP8-Emulator/build/Release/chip8_tests.exe")
  set_tests_properties(opcodes PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/shahk/OneDrive/Documents/ProgrammingProjects/CHIP8-Emulator/CMakeLists.txt;26;add_test;C:/Users/shahk/OneDrive/Documents/ProgrammingProjects/CHIP8-Emulator/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(opcodes "C:/Users/shahk/OneDrive/Documents/ProgrammingProjects/CHIP8-Emulator/build/MinSizeRel/chip8_tests.exe")
  set_tests_properties(opcodes PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/shahk/OneDrive/Documents/ProgrammingProjects/CHIP8-Emulator/CMakeLists.txt;26;add_test;C:/Users/shahk/OneDrive/Documents/ProgrammingProjects/CHIP8-Emulator/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(opcodes "C:/Users/shahk/OneDrive/Documents/ProgrammingProjects/CHIP8-Emulator/build/RelWithDebInfo/chip8_tests.exe")
  set_tests_properties(opcodes PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/shahk/OneDrive/Documents/ProgrammingProjects/CHIP8-Emulator/CMakeLists.txt;26;add_test;C:/Users/shahk/OneDrive/Documents/ProgrammingProjects/CHIP8-Emulator/CMakeLists.txt;0;")
else()
  add_test(opcodes NOT_AVAILABLE)
endif()
