cmake_minimum_required(VERSION 2.6)

if(enable_smpi)
  set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}")
  include_directories("${CMAKE_HOME_DIRECTORY}/include/smpi")

  add_executable(masterslave_mailbox_smpi masterslave_mailbox_smpi.c)
  target_link_libraries(masterslave_mailbox_smpi simgrid)
endif()

set(tesh_files
  ${tesh_files}
  ${CMAKE_CURRENT_SOURCE_DIR}/msg_smpi.tesh
  PARENT_SCOPE
  )
set(xml_files
  ${xml_files}
  ${CMAKE_CURRENT_SOURCE_DIR}/deployment_masterslave_mailbox_smpi.xml
  PARENT_SCOPE
  )
set(examples_src
  ${examples_src}
  ${CMAKE_CURRENT_SOURCE_DIR}/masterslave_mailbox_smpi.c
  PARENT_SCOPE
  )
set(bin_files
  ${bin_files}
  PARENT_SCOPE
  )
set(txt_files
  ${txt_files}
  PARENT_SCOPE
  )
