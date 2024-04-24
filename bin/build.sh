#! /bin/bash
clear
function run {
    echo $@
    $@
}
PROJ=/home/julia/Projects/c++/awkccc
BIN=${PROJ}/bin
SRC=${PROJ}/src
INCLUDE=${PROJ}/include
export PATH=${PATH}:${BIN}
CPP=/usr/bin/g++
# CPP=/usr/bin/clang++
CPPFLAGS="-g -DDEBUG -I${INCLUDE}"
RE2C=/usr/bin/re2c
MUSAMI=${BIN}/musami
echo ${PATH}
cd ${SRC}
pwd
run ${RE2C} --no-debug-info -I${INCLUDE} -o lexer.c++ lexer.re2c
run ${CPP} ${CPPFLAGS} -o ${MUSAMI} musami.c++
run ${MUSAMI} -l -I${INCLUDE} parser.musami
for module in parser_lib parser lexer_lib lexer awkccc; do
    run ${CPP} ${CPPFLAGS} -c -o ${BIN}/${module}.o ${module}.c++ 
done
cd ${BIN}
pwd
run ${CPP} -o awkccc awkccc.o parser_lib.o parser.o lexer_lib.o lexer.o
