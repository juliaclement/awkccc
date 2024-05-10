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
#ifndef AWKCCC_RUNTIME_HPP
#define AWKCCC_RUNTIME_HPP 1
#include <map>
#include "../include/awkccc_variable.h++"
using namespace awkccc;
/** The Awkccc_runtime class acts as a wrapper around the generated C++ code
 *  It provides the runtime variables & implements the Awk processing loop
 *  Generated code supplies the business logic in the final program
 **/
class Awkccc_runtime {
    public:
        int ARGC;
        std::map<jclib::jString, Awkccc_variable> ARGV;
        Awkccc_variable CONVFMT;
        std::map<jclib::jString, Awkccc_variable>  ENVIRON;
        Awkccc_variable FILENAME;
        long FNR;
        Awkccc_variable FS;
        int NF;
        int NR;
        Awkccc_variable OFMT;
        Awkccc_variable OFS;
        Awkccc_variable ORS;
        int RLENGTH;
        Awkccc_variable RS;
        Awkccc_variable RSTART;
        Awkccc_variable SUBSEP;
};
#endif
