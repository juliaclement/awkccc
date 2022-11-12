/***
**
** AWKCCC Lexer -- native C++
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
#include <algorithm>
#include <map>
#include <iostream>
#include "../include/awkccc_ast.hpp"
#include "../src/parser.h++"
#include "../include/awkccc_lexer.hpp"

#include "../include/jString.hpp"
using namespace awkccc;
using namespace jclib;

jclib::CountedPointer<awkccc::ast_node> Lexer::ast_out;

typedef std::map<jclib::jString, jclib::CountedPointer<Symbol> > map_t;


class SymbolTableImpl : public SymbolTable {
    public:
        map_t sym_table_;
        virtual void insert(jclib::CountedPointer<Symbol>s) {
            sym_table_[s->name_] = s;
        }
        
        virtual Symbol * insert(
                class jclib::jString name,
                int token,
                SymbolType type = VARIABLE) {
            Symbol * sym = new Symbol( name, token, type);
            sym_table_[sym->name_] = sym;
            return sym;
        }

        virtual Symbol * find( jclib::jString &str) {
            auto search = sym_table_.find(str);
            if (search == sym_table_.end()) {
                return nullptr;
            }
            return search->second.get();
        }

        /**
         * return existing Symbol or create a new one
        */
        virtual Symbol * get( 
                class jclib::jString name,
                int token,
                SymbolType type = VARIABLE) {
            if( Symbol * answer = find(name) )
                return answer;
            return insert( name, token, type );
        }

        static SymbolTableImpl & instance() {
            static SymbolTableImpl the_table;
            return the_table;
        }

        virtual void load(std::initializer_list<struct _Symbol_loader> input) {
            for( auto i : input) {
                insert( i.name_, i.token_, i.type_);
            }
        }

        virtual void loadnamespace(const jclib::jString &namespace_name,
                    bool also_load_global,
                    std::initializer_list<struct _Symbol_loader> input) {
            size_t max_name_len = 0;
            for( auto i : input) {
                max_name_len = std::max<size_t>(max_name_len,strlen(i.name_));
            }
            auto tmp_name = new char[namespace_name.len()+max_name_len+3];
            strcpy(tmp_name,(const char *)namespace_name);
            strcpy(tmp_name+namespace_name.len(),"__");
            auto target = tmp_name+namespace_name.len()+2;
            for( auto i : input) {
                strcpy(target,i.name_);
                insert( tmp_name, i.token_, i.type_);
            }
            if( also_load_global )
                load(input);
        }

        virtual ~SymbolTableImpl() {}
};

SymbolTable & SymbolTable::instance() {
    return SymbolTableImpl::instance();
}

// interface to musami generated parser
// TODO: make the parser a class

#define ParseTOKENTYPE jclib::CountedPointer<ast_node>
typedef void *(*malloc_t(size_t));
typedef void *(*free_t(void *));
void ParseTrace(FILE *stream, char *zPrefix);
int PARSER_char_to_token( char chr );

// Lexer implementation. The generated Lexer::lex is in lexer.c++

void Lexer::parse(int token_code, SymbolType type ) {
	size_t stringlen=buf_-tok_;
    token_ = jString(tok_,buf_);
    auto sym=symbol_table_->get(token_, token_code, type);
    auto ast = new awkccc::ast_node(Expression, sym );
    parser_->parse( sym->token_, ast, & ast_out  );
    allow_regex_ = false; 
}

void Lexer::include_filename(int token_code, SymbolType type) { // @include ((whitespace)) >>filename<<
      expect_include_filename_ = false;
}

void Lexer::parse_string(int token_code, SymbolType type) { // @include((whitespace)) filename
    if( expect_include_filename_)
        include_filename(token_code, type);
    else
        parse(token_code, type);
}

void Lexer::start_namespace(int token_code, SymbolType type) { // @namespace ((whitespace)) >>name<<
    token_ = jString(tok_,buf_);
    if( token_ == jString("awk"))
        namespace_prefix_ = jclib::jString::get_empty();
    else
        namespace_prefix_ = token_+jclib::jString("__");
    expect_namespacename_ = false;
}

void Lexer::parsequalified(int token_code, SymbolType type) { // (Not "@") identifier"::"identifier
        jString token = jString(tok_,buf_);
        auto bits = token.split("::",1);
        token_ = jString("__").join(bits);
        auto sym=symbol_table_->get(token_,PARSER_NAME);
        auto ast = new awkccc::ast_node(Expression, sym );
        parser_->parse( sym->token_, ast, & ast_out  );
        allow_regex_ = false; 
}

Symbol * Lexer::unqualified_to_sym( int token_code, jString & token ) {
        // What do we have here?
        // All caps -> awk::
        // AWK keyword -> awk::
        // namespace == awk:: -> no prefix
        // else namespace__token
        if( ( namespace_prefix_.len()==0) || ( token.isalpha() && token.isupper() ) )
            return symbol_table_->get(token, token_code, VARIABLE );
        
        auto sym=symbol_table_->find(token);
        if( sym ) {
            // "Magic" types
            switch( sym->type_ ) {
                case STATEMENT:
                case KEYWORD:
                case OPERATOR:
                    return sym;
            }
        }
        token=namespace_prefix_+token;
        return symbol_table_->get(token, token_code, VARIABLE );
}

void Lexer::parseunqualified(int token_code, SymbolType type) { // identifier
    if( expect_namespacename_)
        start_namespace(token_code, type);
    else {
        size_t stringlen=buf_-tok_;
        token_ = jString(tok_,buf_);
        auto sym=unqualified_to_sym(token_code, token_);
        token_ = sym->name_;
        auto ast = new awkccc::ast_node(Expression, sym );
        parser_->parse( sym->token_, ast, & ast_out  );
        allow_regex_ = false; 
    }
}

void Lexer::parsechr(char char_code) {
    int token_code = parser_->char_to_token( char_code );
    if( token_code < 1 ) {
        // construct error message. Once c++17's std::string_view
        // is universally available, switch to that.
        auto buffer=new char[2];
        buffer[0] = *tok_;
        buffer[1]='\0';
        std::cout << "* * * ERROR * * *" << ": \"" << buffer << "\"\n";
        delete[] buffer;
    } else {
        parse( token_code, OPERATOR );
    }
}

// Handle /.../ or /...(eol)
// Posix requires a token starting / should be recognised as
// division wherever a division is syntactically valid otherwise as a regex.
// It was a nightmare trying to get a YACC style parser to inform the
// lexer how to interpret a /, and despite it being really ugly to build
// a subset of grammar rules into the lexer, at least it didn't make my
// brain explode.
// Basically: We recognise / as division only after a variable, a closing
// bracket or parenthesis or a post increment or decrement. 
// allow_regex_ begins as true as program might begin: /[a-e]+/ {do_something;}
// but can't begin with a division
void Lexer::maybe_parse_regex(int token_code) {
    if( allow_regex_)
        parse(token_code,REGEX);
    else if( tok_[1] == '=' ) {  // could be /=
        buf_ = tok_+2;
        parse( PARSER_DIV_ASSIGN, OPERATOR);
    } else {
        static int divide = PARSER_char_to_token( '/' );
        buf_ = tok_+1;
        token_code=*tok_;
        parse( divide, OPERATOR);
    }
}

void Lexer::initialise_symbol_table() {
    symbol_table_->load({
        {"\n",OTHER,PARSER_NEWLINE},
        {"@@@",OPERATOR,PARSER_CONCATENATE},    // Dummy used in parser's AST for concatenate
        {"BEGIN",KEYWORD,PARSER_Begin},
        {"END",KEYWORD,PARSER_End},
        {"break",STATEMENT,PARSER_Break},
        {"continue",STATEMENT,PARSER_Continue},
        {"delete",STATEMENT,PARSER_Delete},
        {"do",STATEMENT,PARSER_Do},
        {"else",STATEMENT,PARSER_Else},
        {"exit",STATEMENT,PARSER_Exit},
        {"for",STATEMENT,PARSER_For},
        {"function",STATEMENT,PARSER_Function},
        {"if",STATEMENT,PARSER_If},
        {"in",KEYWORD,PARSER_In},
        {"next",STATEMENT,PARSER_Next},
        {"print",STATEMENT,PARSER_Print},
        {"printf",STATEMENT,PARSER_Printf},
        {"return",STATEMENT,PARSER_Return},
        {"while",STATEMENT,PARSER_While},
        {"getline",STATEMENT,PARSER_GETLINE},
        {"atan2",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"cos",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"sin",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"exp",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"log",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"sqrt",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"int",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"rand",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"srand",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"gsub",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"index",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"length",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"match",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"split",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"sprintf",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"sub",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"substr",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"tolower",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"toupper",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"close",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"system",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
        {"+=",OPERATOR,PARSER_ADD_ASSIGN},
        {"-=",OPERATOR,PARSER_SUB_ASSIGN},
        {"*=",OPERATOR,PARSER_MUL_ASSIGN},
        {"/=",OPERATOR,PARSER_DIV_ASSIGN},
        {"%=",OPERATOR,PARSER_MOD_ASSIGN},
        {"^=",OPERATOR,PARSER_POW_ASSIGN},
        {"||",OPERATOR,PARSER_OROR},
        {"&&",OPERATOR,PARSER_ANDAND},
        {"!~",OPERATOR,PARSER_NO_MATCH},
        {"==",OPERATOR,PARSER_EQ},
        {"<=",OPERATOR,PARSER_LE},
        {">=",OPERATOR,PARSER_GE},
        {"!=",OPERATOR,PARSER_NE},
        {"++",OPERATOR,PARSER_INCR},
        {"--",OPERATOR,PARSER_DECR},
        {">>",OPERATOR,PARSER_APPEND},
    });
    symbol_table_->loadnamespace("gawk",true,{
        {"BEGINFILE",KEYWORD,PARSER_BeginFile},
        {"ENDFILE",KEYWORD,PARSER_EndFile},
        {"nextfile",STATEMENT,PARSER_NextFile},
    });
    symbol_table_->loadnamespace("awkccc",true,{
        {"BEGINFILE",KEYWORD,PARSER_BeginFile},
        {"MAINLOOP",KEYWORD,PARSER_Mainloop},
        {"ENDFILE",KEYWORD,PARSER_EndFile},
        {"nextfile",STATEMENT,PARSER_NextFile},
    });
    symbol_table_->loadnamespace("awkccc",false,{
        {"blocksize",VARIABLE,PARSER_NAME},
        {"wait_for_pipe_close",VARIABLE,PARSER_NAME},
        {"support_RS",VARIABLE,PARSER_NAME},
        {"local_environ",VARIABLE,PARSER_NAME},
        {"to_string",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
    });
}