/***
**
** AWKCCC: Lexer class
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
#include "../include/jString.hpp"
#include "../src/parser.h++"
#include "awkccc_ast.hpp"
#include <cstdlib>
#include <string>
#include <initializer_list>

namespace awkccc {

  class Lexer {
    private:
      Lexer(const Lexer &) = delete;
      Lexer& operator =(const Lexer &) = delete;
    public:
      char *buf_;
      char *lim_, *cur_, *mar_, *tok_;
      int line_;
      class jclib::jString token_;
      class awkccc::SymbolTable * symbol_table_;
      jclib::CountedPointer<class PARSER_Parser> parser_; 
      // Part of determining if a token starting / should be
      // recognised as division or a regex.
      // Initially True as a program can begin with a regex 
      // but can't begin with a division
      bool allow_regex_ = true; 
      bool expect_include_filename_ = false;
      bool expect_namespacename_ = false;
      class jclib::jString namespace_prefix_ = jclib::jString::get_empty();
      // the awk namespace is magicish, or is it?
      // currently making it prefix with "" looks sensible
      class jclib::jString awk_namespace_prefix_ = jclib::jString::get_empty();
      static jclib::CountedPointer<awkccc::ast_node> ast_out;

      void parse(int token_code, SymbolType type);
      void include_filename(int token_code, SymbolType type);
      void parse_string(int token_code, SymbolType type);
      void start_namespace(int token_code, SymbolType type);
      void parsequalified(int token_code, SymbolType type); // "@"identifier"::"identifier
      Symbol * unqualified_to_sym(int token_code, jclib::jString & token); // identifier
      void parseunqualified(int token_code, SymbolType type); // identifier
      void parsechr(char char_code);
      void maybe_parse_regex(int token_code);
      int lex();
      void initialise_symbol_table();

      Lexer(char * buf, char * lim, char * cur, char * mar, char * tok, int line,
            awkccc::SymbolTable * symbol_table, PARSER_Parser *parser )
            : buf_(buf)
            , lim_(lim)
            , cur_(cur)
            , mar_(mar)
            , tok_(tok)
            , line_(line)
            , symbol_table_(symbol_table)
            , parser_(parser)
            // , token_("")
            // , allow_regex_
      {

      }
  };
};