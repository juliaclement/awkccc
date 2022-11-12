/***
**
** AWKCCC main program
**
** Copyright (C) 2022 Julia Ingleby Clement
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
***/
#include "../include/countedPointer.hpp"
#include "../include/awkccc_ast.hpp"
#include "../include/awkccc_lexer.hpp"
#include "../include/jcargs.hpp"
#include "parser.h++"
#include <iostream>
#include <cstdio>
#include <sstream>

//using namespace jclib;
using namespace awkccc;
typedef void *(*malloc_t(size_t));
typedef void *(*free_t(void *));
#define ParseTOKENTYPE jclib::CountedPointer<ast_node>
void Parse(void *pParser_, int tokenCode, ParseTOKENTYPE token_, jclib::CountedPointer<awkccc::ast_node> *);
void ParseTrace(FILE *stream, char *zPrefix);

using namespace jclib;

struct arg_name_value : public CountedPointerTarget {
    jString arg_;
    jString value_;
    arg_name_value( jString arg, jString value )
    : arg_( arg )
    , value_( value )
    {}
    arg_name_value( )
    : arg_( "" )
    , value_( "" )
    {}
};

template<> bool arg_instance< std::vector<CountedPointer<arg_name_value> > >::set(jString &value) {
    errno = 0;
    arg_name_value * the_aq = new struct arg_name_value();
    the_aq->arg_ = long_name_.len() > 0 ? long_name_ : short_name_;
    the_aq->value_= value;
    answer_.push_back(CountedPointer<arg_name_value>(the_aq));
    multiples_ok_ = true;
    return errno == 0;
}

struct User_Arguments {
    std::vector< jString > includes_;
    std::vector< jString > variables_;
    std::vector< CountedPointer<arg_name_value> > source_files_;
    bool gawk_ = false;
    bool awkccc_ = false;
    bool help_ = false;
    bool version_ = false;
};

int xmain () {
  std::ostringstream oss;
  oss << "One hundred and one: " << 101;
  std::string s = oss.str();
  std::cout << s << '\n';
  return 0;
}

int ymain(int argc, char * argv[]) {

    jString str="a,b,cde,fij,,klm";
    auto arr=str.split(",");
    for( auto i : arr) {
        std::cout << i << "\n";
    }
    arr=str.split(",",2);
    for( auto i : arr) {
        std::cout << i << "\n";
    }
    std::ostringstream oss;
    User_Arguments x;
    arguments args(oss);
    args.load({ arg(x.includes_,"I", "includes", "Directories to search for include files", true, false),
                arg(x.source_files_,"F", "file", "AWK Language source file", true, false),
                arg(x.help_,"h", "help", "Print this help message and exit", false, false),
                arg(x.version_,"V", "version", "Print awkccc version number and exit", false, false),
                arg(x.source_files_,"i", "include", "AWK Language include file", true, false),
                arg(x.source_files_,"e", "source", "AWK Language string", true, false),
                arg(x.variables_,"v", "assign", "Variable assignment", true, false),
                arg(x.gawk_,"", "gawk", "Enable (some) gawk extensions", false, false),
                arg(x.awkccc_,"", "awkccc", "Enable (some) awkccc extensions", false, false)} );
    return 0;
}

int main(int argc, char **argv) {
    
    char buf_[25601];
    bool had_input = false;
    bool clean_tree = true;
    SymbolTable & the_symbol_table = SymbolTable::instance();
    if( argc > 1 ) {
        for( int i = 1; i< argc; ++i ){
            if( argv[i][0] == ':'){
                auto fname = argv[i]+1;
                auto file = fopen(fname, "r");
                auto len = fread(buf_, 1, 25600, file );
                buf_[len]='\0';
				std::cout << buf_ << "\n\n";
                had_input = true;
                PARSER_Parser *parser = PARSER_Parser::Create();
                Lexer lexer (buf_,nullptr,nullptr,nullptr,nullptr,0,&the_symbol_table,parser);
                lexer.initialise_symbol_table();
                lexer.lex();
                if( Lexer::ast_out.isset()) {
                    if( clean_tree )
                        Lexer::ast_out->clean_tree(nullptr);
                    print_ast( std::cout, Lexer::ast_out);
                } else {
                    std::cout << "* * * No parse tree * * *\n";
                }
            }
		}
    }
    if( ! had_input ) {
        //strcpy(buf_,"_p1 += 4;\nanswer=_p1+_param2[13.0]&!7;$1=a/7;$2~/a+b[/]+c+/;$3~/a+b[/]+c+/\n");
        strcpy(buf_,
            "@include \"filename\"\n"
            "BEGIN {b+=a=1+2 < 3+4*5;\n}\n" 
            //"BEGIN {a=1;\nj=$1*$2+$3;\n}\n"
            "END {print a; exit 3;}\n");
        PARSER_Parser *parser = PARSER_Parser::Create();
        Lexer lexer (buf_,nullptr,nullptr,nullptr,nullptr,0,&the_symbol_table,parser);
        lexer.initialise_symbol_table();
        lexer.lex();
        if( clean_tree )
            Lexer::ast_out->clean_tree(nullptr);
        print_ast( std::cout, Lexer::ast_out);
    }
    return 0;
}