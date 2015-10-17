#include "pch.h"
#include "VS2015Printer.h"

/**
 * Callback for a completed test. This function will record
 * the results of the test case, and if it failed, it will also
 * print the failure in a VS2015-compatible format.
 * @param testCase the test case that was just executed
 */
void VS2015Printer::OnTestPartResult(const testing::TestPartResult& test)
{
    if (test.failed())
    {
        // Report the failure
        const char* filename = test.file_name();
        if (filename == nullptr)
        {
            filename = "<unknown source>";
        }
        
        std::cerr << filename << "(" << test.line_number() << "): error: unit test failure: " << test.message() << std::endl;


        // If a debugger is attached, assist the developer by launching it
        if (IsDebuggerPresent())
        {
            DebugBreak();
        }
    }
}


void VS2015Printer::OnTestProgramEnd(const testing::UnitTest& unitTest)
{
    // Check for failures
    if (unitTest.Failed())
    {
        std::cout << "gtest: " << unitTest.failed_test_count() << " test failures in " << unitTest.total_test_count() << " tests ("
            << unitTest.failed_test_case_count() << "/" << unitTest.total_test_case_count() << " cases failed)" << std::endl;
    }
    else
    {
        // Output results
        std::cout << "gtest: " << unitTest.successful_test_count() << " of " << unitTest.total_test_count() << " tests passed in "
            << unitTest.elapsed_time() << "ms" << std::endl;
    }
}
