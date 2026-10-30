add_executable(nirea-test runner/test_runner.c)

set(TEST_WORK_DIR ${CMAKE_CURRENT_BINARY_DIR}/work)

file(GLOB RUN_TESTS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/run/*.ni)
foreach(test_file IN LISTS RUN_TESTS)
    get_filename_component(name ${test_file} NAME_WE)
    add_test(NAME run/${name}
             COMMAND nirea-test run $<TARGET_FILE:nirea> ${test_file} ${TEST_WORK_DIR}/run/${name})
endforeach()

file(GLOB ERROR_TESTS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/errors/*.ni)
foreach(test_file IN LISTS ERROR_TESTS)
    get_filename_component(name ${test_file} NAME_WE)
    add_test(NAME errors/${name}
             COMMAND nirea-test error $<TARGET_FILE:nirea> ${test_file} ${TEST_WORK_DIR}/errors/${name})
endforeach()

file(GLOB PROJECT_TESTS LIST_DIRECTORIES true CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/projects/*)
foreach(project_dir IN LISTS PROJECT_TESTS)
    if(IS_DIRECTORY ${project_dir})
        get_filename_component(name ${project_dir} NAME)
        add_test(NAME projects/${name}
                 COMMAND nirea-test project $<TARGET_FILE:nirea> ${project_dir} ${TEST_WORK_DIR}/projects/${name})
    endif()
endforeach()
