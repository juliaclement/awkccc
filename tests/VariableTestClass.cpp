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
 * File:   VariableTestClass.cpp
 * Author: Julia Clement <Julia at Clement dot nz>
 *
 * Created on 01/05/2024, 13:18:55
 */

#ifdef ONE_FIXTURE
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#endif
#include <cppunit/extensions/HelperMacros.h>
#include <cmath>
#include "../include/awkccc_variable.h++"
using namespace jclib;
using namespace awkccc;

using namespace jclib;
using namespace awkccc;

class VariableTestClass : public CPPUNIT_NS::TestFixture {
public:
    VariableTestClass() {}
    virtual ~VariableTestClass() {}
    void setUp(){
    }
    void tearDown(){
    }
private:
    void testCreateEmpty() {
        Awkccc_variable var;
        CPPUNIT_ASSERT( var.number_is_valid_ );
        CPPUNIT_ASSERT( var.string_is_valid_ );
        CPPUNIT_ASSERT(fabs(var.number_) < Awkccc_variable::epsilon_ );
        CPPUNIT_ASSERT(var.string_ == "" );
    }
    void testCreateInt() {
        Awkccc_variable var(6);
        CPPUNIT_ASSERT( var.number_is_valid_ );
        CPPUNIT_ASSERT( ! var.string_is_valid_ );
        CPPUNIT_ASSERT(fabs(var.number_ - 6) < Awkccc_variable::epsilon_ );
    }
    void testCreateDouble() {
        Awkccc_variable var(6.1);
        CPPUNIT_ASSERT( var.number_is_valid_ );
        CPPUNIT_ASSERT( ! var.string_is_valid_ );
        CPPUNIT_ASSERT(fabs(var.number_ - 6.1) < Awkccc_variable::epsilon_ );
    }
    void testCreateString() {
        Awkccc_variable var("6x1");
        CPPUNIT_ASSERT(! var.number_is_valid_ );
        CPPUNIT_ASSERT( var.string_is_valid_ );
        // FIXME need compare Awkccc_variable
        CPPUNIT_ASSERT(var.string_ == "6x1" );
    }
    void testCreateNumericString() {
        Awkccc_variable var("7",7.0);
        CPPUNIT_ASSERT( var.number_is_valid_ );
        CPPUNIT_ASSERT( var.string_is_valid_ );
        CPPUNIT_ASSERT(fabs(var.number_)-7.0 < Awkccc_variable::epsilon_ );
        CPPUNIT_ASSERT(var.string_ == "7" );
    }
    void testCastStringToNumber() {
        Awkccc_variable var("7");
        CPPUNIT_ASSERT( !var.number_is_valid_ );
        CPPUNIT_ASSERT( var.string_is_valid_ );
        CPPUNIT_ASSERT(var.string_ == "7" );
        CPPUNIT_ASSERT(fabs((double)var)-7.0 < Awkccc_variable::epsilon_ );
        CPPUNIT_ASSERT( var.number_is_valid_ );
        CPPUNIT_ASSERT(fabs(var.number_)-7.0 < Awkccc_variable::epsilon_ );
        CPPUNIT_ASSERT(var.string_ == "7" ); // unchanged
    }
    void testCastIntegerToString() {
        Awkccc_variable var(7);
        CPPUNIT_ASSERT( var.number_is_valid_ );
        CPPUNIT_ASSERT( !var.string_is_valid_ );
        jString ans = (jString)var.format();
        CPPUNIT_ASSERT(ans == "7");
        CPPUNIT_ASSERT(fabs(var.number_)-7.0 < Awkccc_variable::epsilon_ ); // unchanged
    }

    void testCastNegIntegerToString() {
        Awkccc_variable var(-7);
        CPPUNIT_ASSERT( var.number_is_valid_ );
        CPPUNIT_ASSERT( !var.string_is_valid_ );
        jString ans = (jString)var.format();
        CPPUNIT_ASSERT(ans == "-7");
        CPPUNIT_ASSERT(fabs(var.number_)-7.0 < Awkccc_variable::epsilon_ ); // unchanged
    }

    void testCastNumberToString() {
        Awkccc_variable var(7.5);
        CPPUNIT_ASSERT( var.number_is_valid_ );
        CPPUNIT_ASSERT( !var.string_is_valid_ );
        jString ans = (jString)var.format();
        CPPUNIT_ASSERT(ans == "7.5");
        CPPUNIT_ASSERT(fabs(var.number_)-7.5 < Awkccc_variable::epsilon_ ); // unchanged
    }

    void testCastNegNumberToString() {
        Awkccc_variable var(-7.5);
        CPPUNIT_ASSERT( var.number_is_valid_ );
        CPPUNIT_ASSERT( !var.string_is_valid_ );
        jString ans = (jString)var.format();
        CPPUNIT_ASSERT(ans == "-7.5");
        CPPUNIT_ASSERT(fabs(var.number_)-7.5 < Awkccc_variable::epsilon_ ); // unchanged
    }

    void testBasicMath() {
        Awkccc_variable two("2"), eleven("11"), ans;
        // Addition
        ans=two+eleven;
        CPPUNIT_ASSERT(fabs(ans.number_)-13 < Awkccc_variable::epsilon_);
        ans=two+11.0;
        CPPUNIT_ASSERT(fabs(ans.number_)-13 < Awkccc_variable::epsilon_);
        ans=2+eleven;
        CPPUNIT_ASSERT(fabs(ans.number_)-13 < Awkccc_variable::epsilon_);
        // Subtraction
        ans=eleven-two;
        CPPUNIT_ASSERT(fabs(ans.number_)-9 < Awkccc_variable::epsilon_);
        ans=eleven-2.0;
        CPPUNIT_ASSERT(fabs(ans.number_)-9 < Awkccc_variable::epsilon_);
        ans=11-two;
        CPPUNIT_ASSERT(fabs(ans.number_)-9 < Awkccc_variable::epsilon_);
        // Multiply
        ans=eleven*two;
        CPPUNIT_ASSERT(fabs(ans.number_)-22 < Awkccc_variable::epsilon_);
        ans=eleven*2.0;
        CPPUNIT_ASSERT(fabs(ans.number_)-22 < Awkccc_variable::epsilon_);
        ans=11*two;
        CPPUNIT_ASSERT(fabs(ans.number_)-22 < Awkccc_variable::epsilon_);
        // Division
        ans=eleven/two;
        CPPUNIT_ASSERT(fabs(ans.number_)-5.5 < Awkccc_variable::epsilon_);
        ans=eleven/2.0;
        CPPUNIT_ASSERT(fabs(ans.number_)-5.5 < Awkccc_variable::epsilon_);
        ans=11/two;
        CPPUNIT_ASSERT(fabs(ans.number_)-5.5 < Awkccc_variable::epsilon_);
    }

    void testIncDec() {
        Awkccc_variable countup("13");
        CPPUNIT_ASSERT(countup ++ - 13 < Awkccc_variable::epsilon_);
        CPPUNIT_ASSERT(countup - 14 < Awkccc_variable::epsilon_);
        CPPUNIT_ASSERT(++ countup - 15 < Awkccc_variable::epsilon_);
        CPPUNIT_ASSERT(countup - 15 < Awkccc_variable::epsilon_);
        Awkccc_variable countdown(100);
        CPPUNIT_ASSERT(countdown -- - 100 < Awkccc_variable::epsilon_);
        CPPUNIT_ASSERT(countdown - 99 < Awkccc_variable::epsilon_);
        CPPUNIT_ASSERT(-- countdown - 98 < Awkccc_variable::epsilon_);
        CPPUNIT_ASSERT(countdown - 98 < Awkccc_variable::epsilon_);
    }

    CPPUNIT_TEST_SUITE(VariableTestClass);
        CPPUNIT_TEST(testCreateEmpty);
        CPPUNIT_TEST(testCreateInt);
        CPPUNIT_TEST(testCreateDouble);
        CPPUNIT_TEST(testCreateString);
        CPPUNIT_TEST(testCreateNumericString);
        CPPUNIT_TEST(testCastStringToNumber);
        CPPUNIT_TEST(testCastIntegerToString);
        CPPUNIT_TEST(testCastNegIntegerToString);
        CPPUNIT_TEST(testCastNumberToString);
        CPPUNIT_TEST(testCastNegNumberToString);
        CPPUNIT_TEST(testBasicMath);
        CPPUNIT_TEST(testIncDec);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(VariableTestClass);

#ifdef ONE_FIXTURE
int main(int argc, char* argv[])
{
    // Get the top level suite from the registry
    CPPUNIT_NS::Test *suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

    CppUnit::TextUi::TestRunner runner;
    runner.addTest(suite);
    bool wasSucessful = runner.run();
    char* c = (char*) malloc(10 * sizeof (char));
    //scanf (c,"%c");
    return wasSucessful ? 0 : 1;
}
#endif
