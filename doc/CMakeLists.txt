
#  Targets
#  =======

add_custom_target(doxygen
  COMMAND doxygen doxygen.config
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/doc")

add_custom_target(docs
  COMMAND make html
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/doc")
add_dependencies(docs doxygen)

set(lager_ssh_method
  ssh -p 5488
      -o StrictHostKeyChecking=no
      -i ${CMAKE_SOURCE_DIR}/tools/travis/ssh-key)

add_custom_target(upload-docs
  COMMAND
  rsync -av -e \"${lager_ssh_method}\"
        ${CMAKE_SOURCE_DIR}/doc/_build/html/*
        raskolnikov@sinusoid.es:public/lager/)
