enable_testing()

include(GoogleTest)
add_executable(tests 
    container_tests.cc
    factoryregistrationbuilder_tests.cc
    interfaceregistrationbuilder_tests.cc
    listregistrationbuilder_tests.cc
    registrar_tests.cc
    registration_tests.cc
    typeregistrationbuilder_tests.cc)
gtest_discover_tests(tests)

target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tests PRIVATE cdif gtest gtest_main)
