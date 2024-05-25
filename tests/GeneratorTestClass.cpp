/*
Copyright (c) 2024 Julia Ingleby Clement

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
 * File:   GeneratorTestClass.cpp
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
#include "../include/generate_cpp.h++"
using namespace jclib;
using namespace awkccc;

#include "../src/parser.h++"

using namespace jclib;
using namespace awkccc;
typedef void *(*malloc_t(size_t));
typedef void *(*free_t(void *));
#define ParseTOKENTYPE jclib::CountedPointer<ast_node>
/*
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
*/
namespace CppGeneratorTestClassNS {
    PARSER_Parser * the_parser = nullptr;
};

using namespace CppGeneratorTestClassNS;
PARSER_Parser* get_parser( ) {
    return PARSER_Parser::Create();
}

void Parse(void *pParser, int tokenCode, ParseTOKENTYPE token_, jclib::CountedPointer<awkccc::ast_node> *pAbc) {
    PARSER_Parser * parser = static_cast<PARSER_Parser *>(pParser);
    parser->parse( tokenCode, token_, pAbc );
}

// void ParseTrace(FILE *stream, char *zPrefix) {}

class GeneratorTestClass : public CPPUNIT_NS::TestFixture {
public:
    char * buf_ = nullptr;
    GeneratorTestClass() {}
    virtual ~GeneratorTestClass() {}
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
        PARSER_Parser * parser = static_cast<PARSER_Parser *>(pParser);
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
    ast_node_ptr lex( const char * code ) {
        Lexer * lexer = create_lexer( code );
        lexer->lex();
        if( Lexer::ast_out.isset()) {
            Lexer::ast_out->clean_tree(nullptr);
        }
        ast_node_ptr node = Lexer::ast_out;
        delete lexer;
        delete_buffer();
        return node;
    }
    std::vector< postream> code_;
    std::ostringstream output_;
    void generate_cpp( const char * template_txt, ast_node_ptr node) {
        for( auto x : code_) {
            delete x;
        }
        code_.clear();
        for( int i = 0; i < template_NR_SEGMENTS; ++i) {
            code_.push_back( new std::ostringstream );
        }
        std::istringstream template_stream( template_txt );
        output_.clear();
        awkccc::generate_cpp( template_stream, output_, node, code_  );
    }
    void generate_cpp( jclib::jString & template_txt, ast_node_ptr node) {
        generate_cpp( (const char *)template_txt, node );
    }

private:
    void testBegin() {
        ast_node_ptr node = lex("BEGIN {\na=12;\n}\n");
        generate_cpp( "%begin;",node );
        std::cout << code_[template_BEGIN]->str() << "\n";
        std::cout << code_[template_body]->str() << "\n";
    }
/*
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
        CPPUNIT_ASSERT( sym->awk_name_ ==  "Awk::avariable");
        CPPUNIT_ASSERT( sym->c_name_ ==  "Awk__avariable");
    }
    void testNamespaceOverridePermutesVariableName() {
        lex("xxx::avariable");
        ParseTOKENTYPE token = the_parser.trace_[0].token_;
        CPPUNIT_ASSERT(token->has_sym_);
        auto sym = token->sym_;
        CPPUNIT_ASSERT( sym->awk_name_ ==  "xxx::avariable");
        CPPUNIT_ASSERT( sym->c_name_ ==  "xxx__avariable");
    }
    void testNamespacePermutesVariableName() {
        lex("@namespace xxx avariable");
        ParseTOKENTYPE token = the_parser.trace_[0].token_;
        CPPUNIT_ASSERT(token->has_sym_);
        auto sym = token->sym_;
        CPPUNIT_ASSERT( sym->c_name_ ==  "xxx__avariable");
        CPPUNIT_ASSERT( sym->awk_name_ ==  "xxx::avariable");
    }
    void testNamespaceIgnoredByAllUppercase() {
        lex("@namespace xxx ABC");
        ParseTOKENTYPE token = the_parser.trace_[0].token_;
        CPPUNIT_ASSERT(token->has_sym_);
        auto sym = token->sym_;
        CPPUNIT_ASSERT( sym->c_name_ ==  "Awk__ABC");
        CPPUNIT_ASSERT( sym->awk_name_ ==  "Awk::ABC");
    }
    void testNamespaceIgnoredByReservedWord() {
        lex("@namespace xxx if");
        ParseTOKENTYPE token = the_parser.trace_[0].token_;
        CPPUNIT_ASSERT(token->has_sym_);
        auto sym = token->sym_;
        CPPUNIT_ASSERT( sym->awk_name_ ==  "if");
        CPPUNIT_ASSERT( sym->c_name_ ==  "if");
    }
    void testNamespaceOverrideAllUppercase() {
        lex("xxx::ABC");
        ParseTOKENTYPE token = the_parser.trace_[0].token_;
        CPPUNIT_ASSERT(token->has_sym_);
        auto sym = token->sym_;
        CPPUNIT_ASSERT( sym->c_name_ ==  "xxx__ABC");
    }
    */
    CPPUNIT_TEST_SUITE(GeneratorTestClass);
        CPPUNIT_TEST(testBegin);
    /*
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
    */
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(GeneratorTestClass);

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
