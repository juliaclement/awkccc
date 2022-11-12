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
 * File:   LexerTestClass.cpp
 * Author: Julia Clement <Julia at Clement dot nz>
 *
 * Created on 08/11/2022, 17:05:55
 */

#ifdef ONE_FIXTURE
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#endif
#include <cppunit/extensions/HelperMacros.h>
#include "../include/jString.hpp"
#include "../include/awkccc_ast.hpp"
#include "../include/awkccc_lexer.hpp"
using namespace jclib;
using namespace awkccc;

#include "../src/parser.h++"

using namespace jclib;
using namespace awkccc;
typedef void *(*malloc_t(size_t));
typedef void *(*free_t(void *));
#define ParseTOKENTYPE jclib::CountedPointer<ast_node>
// This character to token table is a cut-n-paste from the 
// generated parser.c++ file.
// If the original table changes, it will need recopying.
static
// - - - cut & paste begins - - -
unsigned char PARSER_chars[128] = {
   0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0, 
   0,   15,    0,    0,   22,   13,   33,    0, 
   3,    4,   12,   10,    7,   11,    0,   34, 
   0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,   20,    8,   17,   23,   16,   19, 
   0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    5,    0,    6,   14,    0, 
  30,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    1,   18,    2,   21,    0 };
  int PARSER_char_to_token( char chr ) {return PARSER_chars[chr];}
// - - - cut & paste ends - - -
namespace LexerTestClassNS {
    typedef struct parser_trace {
        int TokenCode_;
        ParseTOKENTYPE token_;
        jclib::CountedPointer<awkccc::ast_node> *pAbc_;
    } ParseTrace;
    typedef class parser_mock : public PARSER_Parser {
        public: 
            std::vector<ParseTrace> trace_;
            void parse( int tokenCode, ParseTOKENTYPE token_, jclib::CountedPointer<awkccc::ast_node> *pAbc ) {
                trace_.push_back( {tokenCode, token_, pAbc} );
            }
            virtual int char_to_token(char chr) {
                return PARSER_chars[chr];
            }
    } ParserMock;
    ParserMock the_parser;
};

using namespace LexerTestClassNS;
PARSER_Parser* get_parser( ) {
    // Don't let reference counter destroy our static variable
    if( the_parser.counter_ == 0 )
        the_parser.counter_++; 
    return & the_parser;
}

void Parse(void *pParser, int tokenCode, ParseTOKENTYPE token_, jclib::CountedPointer<awkccc::ast_node> *pAbc) {
    ParserMock * parser = static_cast<ParserMock *>(pParser);
    parser->parse( tokenCode, token_, pAbc );
}

void ParseTrace(FILE *stream, char *zPrefix) {
}

class LexerTestClass : public CPPUNIT_NS::TestFixture {
public:
    char * buf_ = nullptr;
    LexerTestClass() {}
    virtual ~LexerTestClass() {}
    void setUp(){
    }
    void tearDown(){
    }
    void delete_buffer() {
        delete buf_;
        buf_ = nullptr;
    }
    Lexer * create_lexer( const char * code ) {
        SymbolTable & the_symbol_table = SymbolTable::instance();
        PARSER_Parser * pParser=get_parser();
        ParserMock * parser = static_cast<ParserMock *>(pParser);
        parser->trace_.clear();
        if( buf_ ) 
            delete_buffer();
        if( code ) {
            buf_ = new char[std::strlen(code)+1];
            std::strcpy( buf_, code );
        } else {
            buf_ = new char[2];
            buf_[0] = buf_[1] = '\0';
        }
        Lexer * lexer = new Lexer( buf_ ,nullptr,nullptr,nullptr,nullptr,0,&the_symbol_table,parser);
        lexer->initialise_symbol_table();
        return lexer;
    }
    void lex( const char * code ) {
        Lexer * lexer = create_lexer( code );
        lexer->lex();
        delete lexer;
        delete_buffer();
    }
private:
    void testBegin() {
        lex("BEGIN\n");
        CPPUNIT_ASSERT(the_parser.trace_[0].TokenCode_ == PARSER_Begin );
    }
    void test1CharOp() {
        lex("*\n");
        // Tests that lexer passes the expected token, not that it is the correct token
        CPPUNIT_ASSERT(the_parser.trace_[0].TokenCode_ == PARSER_chars['*'] );
    }
    void testRegex() {
        lex("/[A-Z]+[a-z0-9A-Z]/\n");
        CPPUNIT_ASSERT(the_parser.trace_[0].TokenCode_ == PARSER_ERE );
    }
    void testRegexNotAllowed() {
        // a/... should be interpreted as 'a','/', ... & not as a regex
        lex("a/[A-Z]+[a-z0-9A-Z]/\n");
        CPPUNIT_ASSERT(the_parser.trace_[0].TokenCode_ == PARSER_NAME );
        CPPUNIT_ASSERT(the_parser.trace_[1].TokenCode_ == PARSER_chars['/'] );
    }
    void testRegexNotAllowed2() {
        // a/=... should be interpreted as 'a','/=','...' & not as a regex
        lex("a/=[A-Z]+[a-z0-9A-Z]/\n");
        CPPUNIT_ASSERT(the_parser.trace_[0].TokenCode_ == PARSER_NAME );
        CPPUNIT_ASSERT(the_parser.trace_[1].TokenCode_ == PARSER_DIV_ASSIGN );
    }
    void testPlusEquals() {
        lex("+=");
        CPPUNIT_ASSERT(the_parser.trace_[0].TokenCode_ == PARSER_ADD_ASSIGN );
    }
    void testVariableGoesToVariable() {
        lex("avariable");
        ParseTOKENTYPE token = the_parser.trace_[0].token_;
        CPPUNIT_ASSERT(token->has_sym_);
        auto sym = token->sym_;
        CPPUNIT_ASSERT( sym->type_ ==  awkccc::VARIABLE);
    }
    void testVariableName() {
        lex("avariable");
        ParseTOKENTYPE token = the_parser.trace_[0].token_;
        CPPUNIT_ASSERT(token->has_sym_);
        auto sym = token->sym_;
        CPPUNIT_ASSERT( sym->name_ ==  "avariable");
    }
    void testNamespaceOverridePermutesVariableName() {
        lex("xxx::avariable");
        ParseTOKENTYPE token = the_parser.trace_[0].token_;
        CPPUNIT_ASSERT(token->has_sym_);
        auto sym = token->sym_;
        CPPUNIT_ASSERT( sym->name_ ==  "xxx__avariable");
    }
    void testNamespacePermutesVariableName() {
        lex("@namespace xxx avariable");
        ParseTOKENTYPE token = the_parser.trace_[0].token_;
        CPPUNIT_ASSERT(token->has_sym_);
        auto sym = token->sym_;
        CPPUNIT_ASSERT( sym->name_ ==  "xxx__avariable");
    }
    void testNamespaceIgnoredByAllUppercase() {
        lex("@namespace xxx ABC");
        ParseTOKENTYPE token = the_parser.trace_[0].token_;
        CPPUNIT_ASSERT(token->has_sym_);
        auto sym = token->sym_;
        CPPUNIT_ASSERT( sym->name_ ==  "ABC");
    }
    void testNamespaceIgnoredByReservedWord() {
        lex("@namespace xxx if");
        ParseTOKENTYPE token = the_parser.trace_[0].token_;
        CPPUNIT_ASSERT(token->has_sym_);
        auto sym = token->sym_;
        CPPUNIT_ASSERT( sym->name_ ==  "if");
    }
    void testNamespaceOverrideAllUppercase() {
        lex("xxx::ABC");
        ParseTOKENTYPE token = the_parser.trace_[0].token_;
        CPPUNIT_ASSERT(token->has_sym_);
        auto sym = token->sym_;
        CPPUNIT_ASSERT( sym->name_ ==  "xxx__ABC");
    }
    CPPUNIT_TEST_SUITE(LexerTestClass);
        CPPUNIT_TEST(testBegin);
        CPPUNIT_TEST(test1CharOp);
        CPPUNIT_TEST(testRegex);
        CPPUNIT_TEST(testRegexNotAllowed);
        CPPUNIT_TEST(testRegexNotAllowed2);
        CPPUNIT_TEST(testPlusEquals);
        CPPUNIT_TEST(testVariableGoesToVariable);
        CPPUNIT_TEST(testVariableName);
        CPPUNIT_TEST(testNamespaceOverridePermutesVariableName);
        CPPUNIT_TEST(testNamespacePermutesVariableName);
        CPPUNIT_TEST(testNamespaceIgnoredByAllUppercase);
        CPPUNIT_TEST(testNamespaceIgnoredByReservedWord);
        CPPUNIT_TEST(testNamespaceOverrideAllUppercase);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LexerTestClass);

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
