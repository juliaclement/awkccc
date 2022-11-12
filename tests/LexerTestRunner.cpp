/*
Copyright (c) 2022 Julia Ingleby Clement

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
/* 
 * File:   jStringTestRunner.cpp
 * Author: Julia Clement <Julia at Clement dot nz>
 * 
 * Created on 25/10/2022, 5:15:39 PM
 */

// CppUnit site http://sourceforge.net/projects/cppunit/files

#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>

#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/portability/Stream.h>

class ProgressListener : public CPPUNIT_NS::TestListener {
public:

    ProgressListener()
    : m_lastTestFailed(false) {
    }

    ~ProgressListener() {
    }

    void startTest(CPPUNIT_NS::Test *test) {
        CPPUNIT_NS::stdCOut() << test->getName();
        CPPUNIT_NS::stdCOut() << "\n";
        CPPUNIT_NS::stdCOut().flush();

        m_lastTestFailed = false;
    }

    void addFailure(const CPPUNIT_NS::TestFailure &failure) {
        CPPUNIT_NS::stdCOut() << " : " << (failure.isError() ? "error" : "assertion");
        m_lastTestFailed = true;
    }

    void endTest(CPPUNIT_NS::Test *test) {
        if (!m_lastTestFailed)
            CPPUNIT_NS::stdCOut() << " : OK";
        CPPUNIT_NS::stdCOut() << "\n";
    }

private:
    /// Prevents the use of the copy constructor.
    ProgressListener(const ProgressListener &copy);

    /// Prevents the use of the copy operator.
    void operator=(const ProgressListener &copy);

private:
    bool m_lastTestFailed;
};

int main(int argc, char* argv[]) {
    // Create the event manager and test controller
    CPPUNIT_NS::TestResult controller;

    // Add a listener that colllects test result
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener(&result);

    // Add a listener that print dots as test run.
    ProgressListener progress;
    controller.addListener(&progress);

    // Add the top suite to the test runner
    CPPUNIT_NS::TestRunner runner;
    runner.addTest(CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest());
    runner.run(controller);

    // Print test in a compiler compatible format.
    CPPUNIT_NS::CompilerOutputter outputter(&result, CPPUNIT_NS::stdCOut());
    outputter.write();

    return result.wasSuccessful() ? 0 : 1;
}
