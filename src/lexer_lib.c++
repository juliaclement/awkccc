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

namespace {
    jclib::jString Empty_Str;
    jclib::jString Awk("Awk");
}
class SymbolTableImpl : public SymbolTable {
    public:
        map_t sym_table_;
        virtual void insert(jclib::CountedPointer<Symbol>s) {
            sym_table_[s->awk_name_] = s;
        }
        
        virtual Symbol * insert(
            class jclib::jString awk_namespace, /// AWK source namespace
            class jclib::jString awk_name,
            class jclib::jString c_name,
            int token,
            SymbolType type = VARIABLE,
            bool is_built_in = false,
            const char * args = "",
            const char * returns = "",
            const char * include = "" ) {
            Symbol * sym = new Symbol( awk_namespace, awk_name, c_name, token, type, is_built_in, args, returns, include );
            sym_table_[sym->awk_name_] = sym;
            return sym;
        }

        virtual Symbol * find( jclib::jString &awk_namespace, jclib::jString &awk_name ) {
            jclib::jString str = ( awk_namespace.len( ) == 0 )
                ? ( awk_namespace + "::" + awk_name )
                : awk_name;
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
                class jclib::jString awk_namespace,
                class jclib::jString name,
                int token,
                bool namespace_required, /// true if forced by code
                SymbolType type = VARIABLE) {
            if( ! namespace_required )
                if( Symbol * exact = find(Empty_Str,name) )
                    return exact;
            auto parts = name.split("::",1);
            auto size = parts.size();
            jString namespace_;
            jString c_name;
            if( size == 1 ) {
                /*
                    c_name = jString("Awk::")+name;
                    name = jString("Awk::")+name;
                */
               namespace_ = awk_namespace;
            } else {
                // c_name = parts[0]+jString("::")+parts[1];
                namespace_ = parts[0];
            }
            if( Symbol * answer = find(namespace_,name) )
                return answer;
            return insert( namespace_, name, name, token, type, false );
        }
        static SymbolTable * the_table_;
        static SymbolTable & instance() {
            if( ! the_table_ ) {
                the_table_ = new SymbolTableImpl;
            }
            return * the_table_;
        }
        static SymbolTable & stack_instance() {
            delete the_table_;
            the_table_ = nullptr;
            return instance();
        }
        static SymbolTable & reset_instance() {
            SymbolTable * next = the_table_->next_;
            delete the_table_;
            the_table_ = next;
            return instance();
        }

        virtual void load(std::initializer_list<struct _Symbol_loader> input) {
            for( auto i : input) {
                insert( i.awk_namespace_, i.awk_name_, i.c_name_, i.token_, i.type_, i.is_built_in_, i.returns_, i.args_, i.include_ );
            }
        }

        virtual void loadnamespace(const jclib::jString &namespace_name,
                    bool also_load_global,
                    std::initializer_list<struct _NS_Symbol_loader> input) {
            size_t max_name_len = 0;
            for( auto i : input) {
                max_name_len = std::max<size_t>(max_name_len,strlen(i.name_));
            }
            /*
            auto tmp_awk_name = new char[namespace_name.len()+max_name_len+3];
            auto tmp_c_name = new char[namespace_name.len()+max_name_len+3];
            strcpy(tmp_c_name,(const char *)namespace_name);
            strcpy(tmp_awk_name,(const char *)namespace_name);
            strcpy(tmp_c_name+namespace_name.len(),"__");
            strcpy(tmp_awk_name+namespace_name.len(),"::");
            auto awk_target = tmp_awk_name+namespace_name.len()+2;
            auto c_target = tmp_c_name+namespace_name.len()+2;
            */
            for( auto i : input) {
                //strcpy(awk_target,i.name_);
                //strcpy(c_target,i.name_);
                jclib::jString cname = i.name_;
                cname = cname + "_";
                insert( namespace_name, i.name_, cname, i.token_, i.type_, false);
            }
            if( also_load_global )
                loadnamespace("Awk",false,input);
        }

        virtual ~SymbolTableImpl() {}
};

SymbolTable * SymbolTableImpl::the_table_ = nullptr;

SymbolTable & SymbolTable::instance() {
    return SymbolTableImpl::instance();
}
SymbolTable & SymbolTable::stack_instance() {
    return SymbolTableImpl::stack_instance();
}
SymbolTable & SymbolTable::reset_instance() {
    return SymbolTableImpl::reset_instance();
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
    auto sym=symbol_table_->get(Empty_Str, token_, false, token_code, type);
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
    awk_namespace_prefix_ = token_+jclib::jString("::");
    namespace_prefix_ = token_+jclib::jString("__");
    expect_namespacename_ = false;
}

void Lexer::parsequalified(int token_code, SymbolType type) { // (Not "@") identifier"::"identifier
        token_=jString(tok_,buf_);
         jString token = jString(tok_,buf_);
         auto bits = token.split("::",1);
        auto sym=symbol_table_->get(bits[0],bits[1], true,PARSER_NAME);
        auto ast = new awkccc::ast_node(Expression, sym );
        parser_->parse( sym->token_, ast, & ast_out  );
        allow_regex_ = false; 
}

Symbol * Lexer::unqualified_to_sym( int token_code, jString & token ) {
        // What do we have here?
        // All caps -> Awk::
        // AWK keyword -> Awk::
        // namespace == Awk:: -> no prefix
        // else namespace__token
        if( ( namespace_prefix_.len()==0) || ( token.isalpha() && token.isupper() ) )
            return symbol_table_->get(Awk, token, token_code, false, VARIABLE );
        
        auto sym=symbol_table_->find(namespace_prefix_,token);
        if( sym ) {
            // "Magic" types
            switch( sym->type_ ) {
                case STATEMENT:
                case KEYWORD:
                case OPERATOR:
                    return sym;
            }
        }
        return symbol_table_->get(namespace_prefix_,token, token_code, false, VARIABLE );
}

void Lexer::parseunqualified(int token_code, SymbolType type) { // identifier
    if( expect_namespacename_)
        start_namespace(token_code, type);
    else {
        size_t stringlen=buf_-tok_;
        token_ = jString(tok_,buf_);
        auto sym=unqualified_to_sym(token_code, token_);
        token_ = sym->awk_name_;
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
        {Empty_Str, "\n","\n",OTHER,PARSER_NEWLINE},
        {Empty_Str, "@@@", "@@@", OPERATOR,PARSER_CONCATENATE},    // Dummy used in parser's AST for concatenate
        {Empty_Str, "+=","+=",OPERATOR,PARSER_ADD_ASSIGN},
        {Empty_Str, "-=","-=",OPERATOR,PARSER_SUB_ASSIGN},
        {Empty_Str, "*=","*=",OPERATOR,PARSER_MUL_ASSIGN},
        {Empty_Str, "/=","/=",OPERATOR,PARSER_DIV_ASSIGN},
        {Empty_Str, "%=","%=",OPERATOR,PARSER_MOD_ASSIGN},
        {Empty_Str, "^=","^=",OPERATOR,PARSER_POW_ASSIGN},
        {Empty_Str, "||","||",OPERATOR,PARSER_OROR},
        {Empty_Str, "&&","&&",OPERATOR,PARSER_ANDAND},
        {Empty_Str, "!~","!~",OPERATOR,PARSER_NO_MATCH},
        {Empty_Str, "==","==",OPERATOR,PARSER_EQ},
        {Empty_Str, "<=","<=",OPERATOR,PARSER_LE},
        {Empty_Str, ">=",">=",OPERATOR,PARSER_GE},
        {Empty_Str, "!=","!=",OPERATOR,PARSER_NE},
        {Empty_Str, "++","++",OPERATOR,PARSER_INCR},
        {Empty_Str, "--","--",OPERATOR,PARSER_DECR},
        {Empty_Str, ">>",">>",OPERATOR,PARSER_APPEND},
        {Empty_Str, "break","break",STATEMENT,PARSER_Break},
        {Empty_Str, "continue","continue",STATEMENT,PARSER_Continue},
        {Empty_Str, "delete","delete",STATEMENT,PARSER_Delete},
        {Empty_Str, "do","do",STATEMENT,PARSER_Do},
        {Empty_Str, "else","else",STATEMENT,PARSER_Else},
        {Empty_Str, "exit","exit",STATEMENT,PARSER_Exit},
        {Empty_Str, "for","for",STATEMENT,PARSER_For},
        {Empty_Str, "function","function",STATEMENT,PARSER_Function},
        {Empty_Str, "if","if",STATEMENT,PARSER_If},
        {Empty_Str, "in","in",KEYWORD,PARSER_In},
        {Empty_Str, "next","next",STATEMENT,PARSER_Next},
        {Empty_Str, "print","print",STATEMENT,PARSER_Print},
        {Empty_Str, "printf","printf",STATEMENT,PARSER_Printf},
        {Empty_Str, "return","return",STATEMENT,PARSER_Return},
        {Empty_Str, "while","while",STATEMENT,PARSER_While},
        {Empty_Str, "getline","getline",STATEMENT,PARSER_GETLINE},});

    symbol_table_->load({
        // Mathematic functions
        {Empty_Str, "atan2","atan2",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "FF", "F","<cmath>"},
        {Empty_Str, "cos", "cos",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "F", "F","<cmath>"},
        {Empty_Str, "sin","sin",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "F", "F","<cmath>"},
        {Empty_Str, "exp","exp",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "F", "F","<cmath>"},
        {Empty_Str, "log","log",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "F", "F","<cmath>"},
        {Empty_Str, "sqrt","sqrt",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "F", "F","<cmath>"},
        // int_least_64 is a typedef not a function, working on the basis here that a cast will be close enough
        {Empty_Str, "int","int_least64_t",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "F", "I","<cstdint>"},
        {Empty_Str, "rand","rand",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "-", "F","<cmath>"},
        {Empty_Str, "srand","srand",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "F", "F","<cmath>"},
        // String functions
        {Empty_Str, "gsub","gsub",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,true, "", "I",""}, // FIXME
        {Empty_Str, "index","index",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "", "I",""}, // FIXME
        {Empty_Str, "length","length",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "", "I",""}, // FIXME
        {Empty_Str, "match","match",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,true, "", "I",""}, // FIXME
        {Empty_Str, "split","split",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "", "F",""}, // FIXME
        {Empty_Str, "sub","sub",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "", "I",""}, // FIXME
        {Empty_Str, "substr","substr",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "", "F",""}, // FIXME
        {Empty_Str, "tolower","tolower",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "", "F",""}, // FIXME
        {Empty_Str, "toupper","toupper", FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "", "F",""}, // FIXME
        {Empty_Str, "sprintf","sprintf",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "", "S",""}, // FIXME
        {Empty_Str, "close","close",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "", "I",""}, // FIXME
        {Empty_Str, "system","std::system",FUNCTION,PARSER_BUILTIN_FUNC_NAME,true,false, "S", "I","<stdlib>"},});

    symbol_table_->loadnamespace("Awk",false,{
        {"BEGIN",KEYWORD,PARSER_Begin},
        {"END",KEYWORD,PARSER_End},
        {"ARGC", VARIABLE, PARSER_NAME, true },
        {"ARGV", VARIABLE, PARSER_NAME, true },
        {"CONVFMT", VARIABLE, PARSER_NAME, true },
        {"ENVIRON", VARIABLE, PARSER_NAME, true },
        {"FILENAME", VARIABLE, PARSER_NAME, true },
        {"FNR", VARIABLE, PARSER_NAME, true },
        {"FS", VARIABLE, PARSER_NAME, true },
        {"NF", VARIABLE, PARSER_NAME, true },
        {"NR", VARIABLE, PARSER_NAME, true },
        {"OFMT", VARIABLE, PARSER_NAME, true },
        {"OFS", VARIABLE, PARSER_NAME, true },
        {"ORS", VARIABLE, PARSER_NAME, true },
        {"RLENGTH", VARIABLE, PARSER_NAME, true },
        {"RS", VARIABLE, PARSER_NAME, true },
        {"RSTART", VARIABLE, PARSER_NAME, true },
        {"SUBSEP", VARIABLE, PARSER_NAME, true },
    });
    symbol_table_->loadnamespace("gawk",true,{
        {"BEGINFILE",KEYWORD,PARSER_BeginFile},
        {"ENDFILE",KEYWORD,PARSER_EndFile},
        {"nextfile",STATEMENT,PARSER_NextFile},
        {"ARGIND", VARIABLE, PARSER_NAME, true },
        {"ERRNO", VARIABLE, PARSER_NAME, true },
        {"RT", VARIABLE, PARSER_NAME, true },
    });
    symbol_table_->loadnamespace("awkccc",true,{
        {"BEGINFILE",KEYWORD,PARSER_BeginFile},
        {"MAINLOOP",KEYWORD,PARSER_Mainloop},
        {"ENDFILE",KEYWORD,PARSER_EndFile},
        {"nextfile",STATEMENT,PARSER_NextFile},
        {"ARGIND", VARIABLE, PARSER_NAME, true },
        {"ERRNO", VARIABLE, PARSER_NAME, true },
        {"RT", VARIABLE, PARSER_NAME, true },
    });
    symbol_table_->loadnamespace("awkccc",false,{
        {"blocksize",VARIABLE,PARSER_NAME},
        {"wait_for_pipe_close",VARIABLE,PARSER_NAME},
        {"support_RS",VARIABLE,PARSER_NAME},
        {"local_environ",VARIABLE,PARSER_NAME},
        {"to_string",FUNCTION,PARSER_BUILTIN_FUNC_NAME},
    });
}