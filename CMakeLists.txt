cmake_minimum_required(VERSION 3.21)

project(PersonalExpenseTracker
    VERSION 3.0.0
    DESCRIPTION "A local desktop personal finance tracker built with C++ and Qt 6"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(ExpenseTracker
    src/main.cpp
    src/Expense.cpp
    src/ExpenseStorage.cpp
    src/Income.cpp
    src/IncomeStorage.cpp
    src/Budget.cpp
    src/BudgetStorage.cpp
    src/RecurringRule.cpp
    src/RecurringStorage.cpp
    src/MainWindow.cpp
    include/Expense.h
    include/ExpenseStorage.h
    include/Income.h
    include/IncomeStorage.h
    include/Budget.h
    include/BudgetStorage.h
    include/RecurringRule.h
    include/RecurringStorage.h
    include/MainWindow.h
)

target_include_directories(ExpenseTracker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ExpenseTracker PRIVATE Qt6::Widgets)

set_target_properties(ExpenseTracker PROPERTIES
    WIN32_EXECUTABLE ON
)

if(MSVC)
    target_compile_options(ExpenseTracker PRIVATE /W4 /permissive-)
else()
    target_compile_options(ExpenseTracker PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(ExpenseTrackerModelTests
        tests/model_storage_tests.cpp
        src/Expense.cpp
        src/ExpenseStorage.cpp
        src/Income.cpp
        src/IncomeStorage.cpp
        src/Budget.cpp
        src/BudgetStorage.cpp
        src/RecurringRule.cpp
        src/RecurringStorage.cpp
        include/Expense.h
        include/ExpenseStorage.h
        include/Income.h
        include/IncomeStorage.h
        include/Budget.h
        include/BudgetStorage.h
        include/RecurringRule.h
        include/RecurringStorage.h
    )
    target_include_directories(ExpenseTrackerModelTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(ExpenseTrackerModelTests PRIVATE Qt6::Core)
    if(MSVC)
        target_compile_options(ExpenseTrackerModelTests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(ExpenseTrackerModelTests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME ExpenseTrackerModelStorageTests COMMAND ExpenseTrackerModelTests)
endif()
