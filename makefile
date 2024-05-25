BINDIR = bin
SRCDIR = src
TESTDIR = tests
INCDIR = include
RE2C = /usr/bin/re2c
INCS = $(INCDIR)/awkccc_ast.hpp
INCS += $(INCDIR)/countedPointer.hpp
INCS += $(INCDIR)/jString.hpp
INCS += $(INCDIR)/tokens.hpp
INCS += $(INCDIR)/awkccc_lexer.hpp
INCS += $(INCDIR)/jcargs.hpp
INCS += $(INCDIR)/Save.hpp
INCS += $(SRCDIR)/parser.h++
INCS += $(INCDIR)/awkccc.h++
OBJS = $(BINDIR)/lexer_lib.o $(BINDIR)/lexer.o $(BINDIR)/parser_lib.o $(BINDIR)/parser.o $(BINDIR)/generate_cpp.o
CPP = CPP=/usr/bin/g++

build: $(BINDIR)/musami $(BINDIR)/awkccc $(BINDIR)/LexerTestClass $(BINDIR)/GeneratorTestClass

$(BINDIR)/musami: $(SRCDIR)/musami.c++
	g++ -g $< -o $@

$(BINDIR)/awkccc: $(SRCDIR)/awkccc.c++ $(INCS) $(OBJS) 
	g++ -g $< -o $@ $(OBJS)

$(SRCDIR)/lexer.c++: $(SRCDIR)/lexer.re2c
	$(RE2C) --no-debug-info -I$(INCDIR) $< -o $@ 

$(SRCDIR)/parser.c++ $(SRCDIR)/parser.h++:  $(BINDIR)/musami $(BINDIR)/musami_skeleton.c++ $(SRCDIR)/parser.musami
	-$(BINDIR)/musami  -l -I$(INCDIR) $(SRCDIR)/parser.musami
# || echo OK.

$(BINDIR)/LexerTestClass.o: $(TESTDIR)/LexerTestClass.cpp $(INCS)
	g++ -g -DDEBUG -DONE_FIXTURE -std=c++17 -I../$(INCDIR) -I/usr/include -c $< -o $@

$(BINDIR)/GeneratorTestClass.o: $(TESTDIR)/GeneratorTestClass.cpp $(INCS)
	g++ -g -DDEBUG -DONE_FIXTURE -std=c++17 -I../$(INCDIR) -I/usr/include -c $< -o $@

$(BINDIR)/%.o: $(SRCDIR)/%.cpp $(INCS)
	g++ -g  -DDEBUG -I$(INCDIR) -std=c++17 -c $< -o $@

$(BINDIR)/%.o: $(SRCDIR)/%.c++ $(INCS)
	g++ -g -DDEBUG -I$(INCDIR) -std=c++17 -c $< -o $@

$(BINDIR)/%.o: $(TESTDIR)/%.cpp $(INCS)
	g++ -g -DDEBUG -DONE_FIXTURE -std=c++17 -I../$(INCDIR) -I/usr/include -c $< -o $@

$(BINDIR)/LexerTestClass: $(BINDIR)/LexerTestClass.o $(BINDIR)/lexer.o $(BINDIR)/lexer_lib.o $(BINDIR)/parser_lib.o $(BINDIR)/generate_cpp.o
	g++ -o $@ $< $(BINDIR)/lexer.o $(BINDIR)/lexer_lib.o $(BINDIR)/parser_lib.o $(BINDIR)/generate_cpp.o /usr/lib/x86_64-linux-gnu/libcppunit.a

$(BINDIR)/GeneratorTestClass: $(BINDIR)/GeneratorTestClass.o $(BINDIR)/lexer.o $(BINDIR)/lexer_lib.o $(BINDIR)/parser.o $(BINDIR)/parser_lib.o $(BINDIR)/generate_cpp.o
	g++ -o $@ $< $(BINDIR)/lexer.o $(BINDIR)/lexer_lib.o $(BINDIR)/parser.o $(BINDIR)/parser_lib.o $(BINDIR)/generate_cpp.o /usr/lib/x86_64-linux-gnu/libcppunit.a

PHONY : clean
clean :
		-rm $(BINDIR)/awkccc $(BINDIR)/LexerTestClass $(OBJS) $(SRCDIR)/lexer.c++ $(SRCDIR)/parser.c++