/***
**
** AWKCCC main includes
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
#include "../include/countedPointer.hpp"
#include "../include/awkccc_ast.hpp"
#include "../include/awkccc_lexer.hpp"
#include "../include/jString.hpp"
#include "../include/jcargs.hpp"
#include "../src/parser.h++"
#include <iostream>
#include <cstdio>
#include <sstream>

// Write Output using a template. Defined in generate_cpp.c++
#define template_VARS 0
#define template_PROCS 1
#define template_BEGIN 2
#define template_BEGIN_FILE 3
#define template_body 4 
#define template_END_FILE 5
#define template_END 6
void generate_output( std::ifstream &template_text, jclib::jString filename_out, std::vector<jclib::jString> code[7] );

void generate_output( jclib::jString template_filename, jclib::jString filename_out, std::vector<jclib::jString> code[7] );
