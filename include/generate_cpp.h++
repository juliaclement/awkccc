/***
**
** AWKCCC: Generate cpp from an AST
**
** Copyright (C) 2024 Julia Ingleby Clement
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


/* 
 * File:   awkccc_ast.hpp
 * Author: Julia Clement <Julia at Clement dot nz>
 * 
 * Part of the awkccc project https://github.com/juliaclement/awkccc
 *
 * Created on 24 May 2024, 16:54
 */
#ifndef AWKCCC_GENERATE_CPP_HPP
#define AWKCCC_GENERATE_CPP_HPP

#include "../include/awkccc_ast.hpp"
#include <sstream>
#include "../include/jString.hpp"
#include "../include/jcargs.hpp"
#include "../src/parser.h++"

// Write Output using a template. Defined in generate_cpp.c++
#define template_INCLUDES 0
#define template_VARS 1
#define template_PROCS 2
#define template_BEGIN 3
#define template_BEGIN_FILE 4
#define template_body 5
#define template_END_FILE 6
#define template_END 7
#define template_NR_SEGMENTS 8
namespace awkccc {
    typedef std::ostringstream * postream;
    /** write_to_cpp
     *  writes the generated code to the output
     *  usually called by generate_cpp() but publically exposed to support being called from unit tests
     * 
    */
    void write_to_cpp(  std::istream & template_text,
                        std::ostream & program_file,
                        std::vector< postream > & code );

    /**
     * Dump the contents of the ast to program_file stream controlled by template_text
     * Under normal circumstances this routine will be called by the other overload
     * This copy exposes generated code, and code parts for unit testing.
    */
    void generate_cpp(  std::istream & template_text,
                        std::ostream & program_file,
                        awkccc::ast_node * node,
                        std::vector<postream> code );

    /**
     * Dump the contents of the ast to program_file stream controlled by template_text
     * Under normal circumstances this routine will be called by the other overload
     * This copy exposes generated code for unit testing.
    */
    void generate_cpp(  std::istream & template_text,
                        std::ostream & program_file,
                        awkccc::ast_node * node );

    /**
     * Dump the contents of the ast to filename_out stream controlled by template_filename
     * Under normal circumstances this is the routine to be called for this function
    */
    void generate_cpp(  jclib::jString template_filename,
                        jclib::jString filename_out,
                        awkccc::ast_node * node );
}

#endif