/***
**
** AWKCCC template manager
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
#include "../include/awkccc.h++"
using namespace jclib;
void generate_output( std::ifstream & template_text, jclib::jString filename_out, std::vector<jString> code[7] ){
    std::ofstream program_file(filename_out, std::ios_base::trunc | std::ios_base::out );
    std::string markers[] = {"%variables;","%procedures;","%begin;","%begin_file;","%body;","%end_file;","%end;"};
    std::string line;
    while( getline( template_text, line ) ){
        if( line[0] == '%' ) {
            bool found = false;
            for( int i = 0; i <= template_END; ++i ) {
                auto kw = markers[i];
                if( line.compare(0,kw.length(),kw)) {
                    found = true;
                    for( auto block_line : code[i] ){
                        program_file << block_line << "\n";
                    }
                    break;
                }
            }
            if( ! found )
                program_file << line << "\n";
        } else {
            program_file << line << "\n";
        }
    }
    program_file.close();
}
void generate_output( jString template_filename, jString filename_out, std::vector<jString> code[7] ){
    std::ifstream template_text( template_filename );
    generate_output( template_text, filename_out, code );
    template_text.close();
}