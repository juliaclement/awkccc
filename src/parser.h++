# ifndef PARSER_MUSAMI_PARSER
# define PARSER_MUSAMI_PARSER
#   define PARSER_NEWLINE                          9
#   define PARSER_ADD_ASSIGN                      24
#   define PARSER_SUB_ASSIGN                      25
#   define PARSER_MUL_ASSIGN                      26
#   define PARSER_DIV_ASSIGN                      27
#   define PARSER_MOD_ASSIGN                      28
#   define PARSER_POW_ASSIGN                      29
#   define PARSER_OROR                            31
#   define PARSER_ANDAND                          32
#   define PARSER_CONCATENATE                     35
#   define PARSER_NAME                            36
#   define PARSER_NUMBER                          37
#   define PARSER_STRING                          38
#   define PARSER_ERE                             39
#   define PARSER_FUNC_NAME                       40
#   define PARSER_Begin                           41
#   define PARSER_BeginFile                       42
#   define PARSER_Mainloop                        43
#   define PARSER_EndFile                         44
#   define PARSER_End                             45
#   define PARSER_Break                           46
#   define PARSER_Continue                        47
#   define PARSER_Delete                          48
#   define PARSER_Do                              49
#   define PARSER_Else                            50
#   define PARSER_Exit                            51
#   define PARSER_For                             52
#   define PARSER_Function                        53
#   define PARSER_If                              54
#   define PARSER_In                              55
#   define PARSER_Next                            56
#   define PARSER_NextFile                        57
#   define PARSER_Print                           58
#   define PARSER_Printf                          59
#   define PARSER_Return                          60
#   define PARSER_While                           61
#   define PARSER_BUILTIN_FUNC_NAME               62
#   define PARSER_GETLINE                         63
#   define PARSER_APPEND                          64
#   define PARSER_LE                              65
#   define PARSER_NE                              66
#   define PARSER_EQ                              67
#   define PARSER_GE                              68
#   define PARSER_NO_MATCH                        69
#   define PARSER_INCR                            70
#   define PARSER_DECR                            71
    int PARSER_char_to_token( char chr );

#define PARSER_TOKENTYPE jclib::CountedPointer<awkccc::ast_node>
#define PARSER_ARG_PDECL ,PARSER_TOKENTYPE * pAbc

class PARSER_Parser {
  public:
    // Make compatible with CountedPointers
    int counter_ = 0;
    inline void CountedPointerAttach() { ++counter_; }
    inline void CountedPointerDetach() { if( !--counter_) delete this;}
    static PARSER_Parser * Create();
    virtual void parse( int yymajor,              /* The major token code number */
                        PARSER_TOKENTYPE yyminor       /* The value for the token */
                        PARSER_ARG_PDECL               /* Optional %extra_argument parameter */ ) = 0;
    /* Translate a single character to a token. Returns zero if not known to the grammar */
    virtual int char_to_token( char chr ) = 0;    /* The token */
    virtual ~PARSER_Parser() {}
};
# endif
