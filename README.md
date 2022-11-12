# AWK Compiler / Translator
## Welcome
Awkccc will be a compiler / transpiler for the AWK language.

If you are looking for a working AWK to Python transpiler, you are here a bit early. The [awktopython project](https://github.com/juliaclement/awktopython) is still available and supports a large subset of AWK.

## Status
* Tokenizer / Lexer: Working
* Parser: Compiles syntactically correct AWK programs to AST data structures in memory
* Fails (rather than reporting errors) on incorrect programs.
* Does not support multiple source files (Or other command line parameters)
* No code generation yet written
## Purpose

Awkccc is designed to convert a large subset of the AWK language into other languages. Initially C++ 17 and Python 3.8 and later. It is being developed using the GNU C++ and Clang++ compilers with the -std=c++17 setting. Python will be tested under the latest version of Cpython (currently 3.10.8) & pypy3 (currently 7.3.9 which documents that it has Python 3.8 compatibility). Cobol, Python 2 & K&R C will not be supported.

The translated program can either be immediately executed or saved for
later execution.

## Dependencies
* Under Linux & the BSDs: gnu make, a C++ compiler with C++-17 support (e.g. gcc or Clang), CPPUnit (libcppunit & friends) which should all be available through your package manager.  
  
  Other operating systems and recent compilers should work but I lack the means to test them. Apple XCode should support Clang.

* [Re2c](https://re2c.org/) lexer generator. I'm developing on Debian Bookworm & just did an _apt install re2c_ to get the current version. Your distro's package manager should provide it in their apt, yum or ports/pkg collections. If not the [re2c website](https://re2c.org/build/build.html) has instructions for building it.
* Several classes from my [jclib](https://github.com/juliaclement/jclib) library are used. MIT licence. Currently, they need to be manually copied into the awkccc source tree.
* Musami needs to be compiled ahead of compiling the parser. Make should just do it, but if not try "make bin/musami".

## Licences
I don't modify or distribute Re2c, Clang++, GCC, CPPUnit, Make, Python, etc. Their licenses are available on their websites.

Lemon is public domain, there is some doubt if this is universally available & so some corporate lawyers advise their clients against using public domain software (huh???). Other public domain equivalent licences such as BSD-0 are only approved by certain organisations. I've dual licenced my changes to musami as either public domain or the [Boost 1.0 license](https://opensource.org/licenses/bsl1.0.html) which has wide acceptance & is sufficiently permissive it would be difficult to break its terms.

Jclib is MIT

Awktopython was licensed under the Apache Licence 2.0 & I intend recycling the runtime from that into this project so I am currently posting that as the license for the whole project.

## History
This is a follow-on from the earlier [awktopython project](https://github.com/juliaclement/awktopython) which is now frozen. Awkccc's compiler & code generators are a complete rewrite from scratch (in a different language) although the awktopython Python runtime library is decent and will probably be recycled.

### Rewrite Rationale
After a few years of not writing anything major I wanted a reasonably sized project to reignite my passion for software development. I'd done some fixes to an ancient Python screenplay editor and written fountain<->ODT translators (fountain is a markdown language specialised for screenplays) both in Python & I decided on a whim to write an AWK to Python translator.

The original plan was to write a reasonably sized subset of AWK (A language I use on a regular basis) covering the constructs I use with proper unit testing of the translator. It began as an ugly hack with a flakey tokenizer and a recursive descent parser, but once at the expression level, most of the source token strings were passed on to Python without regard for operator precedence, etc.

Then scope creep stepped in ... Most of AWK is now implemented. What's left wouldn't be hard to finish & I actually use it as my default AWK executable. When using PyPy3 as the back end, it runs at something between 2/3 & 3/4 the speed of Gawk.

I eventually rewrote the tokenizer from scratch, but everything else is very close to whatever bit of code I originally wrote for that function. Python code is generated directly from within the parser as constructs are parsed. There is insufficient separation between the tokenizer & the parser and no separation at all between the parser & the code generator.

Then I thought it would be cool to refactor the parser to generate an AST to allow options such as optimising code or even generating different output languages but the mixing of parser and code generator meant that would be close to a total rewrite. Which leads to a
### Language choice
Awktopython was written in Python with a homegrown tokenizer and a recursive descent parser, awkccc is written in C++ with a tokenizer in [re2c](https://re2c.org/)'s language and a parser generated by a modified version of Richard Hipps' excellent [lemon](https://sqlite.org/src/doc/trunk/doc/lemon.html) parser generator which I have named musami (another citrus fruit, sometimes called the sweet lemon).

I didn't want to write another recursive descent parser so I started by looking for suitable Python (re-)implementations of Yacc-like parser generators, found some but none of them matched my objective and subjective criteria. 

This pointed me back to C++ with a suitable parser generator. I've been using C++ since the days of Zortech C++ over 30 years ago and before that dabbled with MS-DOS ports/clones of Yacc & Lex in my C days (No longer sure whose implementations those were). When I decided to rewrite in C++, looking for a Yacc clone as my parser generator was a high priority. I eventually decided on the lemon parser generator then decided I didn't like a couple of minor aspects of it, so I modified it, and in a fit of whimsy named my fork "Sweet lemon" which became "musami". Since then I've made a few more relatively minor changes to to make it more C++ friendly. Re2c seems a popular choice for making lexers for use with lemon. The simple syntax and speed were compelling reasons for choosing it.

Lemon is an integral part of the widly used SQLite project and maintained by the SQLite developer(s) while re2c is the lexer for PHP and maintained by a team so both are unlikely to be orphanned any time soon.