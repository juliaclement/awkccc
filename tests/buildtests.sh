#! /bin/bash
clear
function run {
    echo $@
    $@
}
PROJ=/home/julia/Projects/c++/awkccc
BIN=${PROJ}/bin
SRC=${PROJ}/src
TESTS=${PROJ}/tests
INCLUDE=${PROJ}/include
export PATH=${PATH}:${BIN}
CPP=/usr/bin/g++
# CPP=/usr/bin/clang++
CPPFLAGS="-g -DDEBUG  -DONE_FIXTURE -I${INCLUDE}  -I/usr/include"
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
cd ${TESTS}
pwd
echo '#include "../tests/LexerTestClass.cpp"' > ${SRC}/LexerTestClass.cpp
for module in LexerTestClass; do
    # run ${CPP} ${CPPFLAGS} -c -o ${BIN}/${module} -std=c++17 -D ONE_FIXTURE -I${INCLUDE} ${SRC}/${module}.cpp /usr/lib/x86_64-linux-gnu/libcppunit.a ${BIN}/parser_lib.o ${BIN}/lexer_lib.o ${BIN}/lexer.o
    run ${CPP} ${CPPFLAGS} -c -o ${BIN}/${module}.o -std=c++17 ${SRC}/${module}.cpp
done
cd ${BIN}
run ${CPP} -o LexerTestClass LexerTestClass.o parser_lib.o lexer_lib.o lexer.o /usr/lib/x86_64-linux-gnu/libcppunit.a
cp LexerTestClass ${TESTS}/
