/***
**
** AWKCCC: Abstract Syntax Tree classes
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


/* 
 * File:   awkccc_ast.hpp
 * Author: Julia Clement <Julia at Clement dot nz>
 * 
 * Part of the awkccc project https://github.com/juliaclement/awkccc
 *
 * Created on 17 October 2022, 13:08 PM
 */
#ifndef AWKCCC_AST_HPP
#define AWKCCC_AST_HPP
#include "countedPointer.hpp"
#include "jString.hpp"
#include <vector>

namespace {
    jclib::jString Empty_String;
}
namespace awkccc {
    enum SymbolType {
        VARIABLE,
        CONSTANT,
        REGEX,
        FUNCTION,
        STATEMENT,
        KEYWORD,
        OPERATOR,
        OTHER
    };

    class Symbol: public jclib::CountedPointerTarget {
    public:
        class jclib::jString awk_namespace_;
        class jclib::jString awk_name_;
        class jclib::jString c_name_;
        SymbolType type_;
        int token_;
        bool regex_may_follow_ = true;
        bool is_built_in_ = false;
        class jclib::jString args_;
        class jclib::jString returns_;
        class jclib::jString include_;
        /// tells code generator if it needs to be generated.
        bool is_used_;
        Symbol( class jclib::jString awk_namespace,
                class jclib::jString awk_name,
                class jclib::jString c_name,                
                int token,
                SymbolType type = VARIABLE,
                bool is_built_in = false,
                class jclib::jString args = jclib::jString::get_empty(),
                class jclib::jString returns = jclib::jString::get_empty(),
                class jclib::jString include = jclib::jString::get_empty(),
                bool is_used_ = false )
            : awk_namespace_(awk_namespace)
            , awk_name_(awk_name)
            , c_name_(c_name)
            , type_(type)
            , token_(token)
            , is_built_in_( is_built_in )
            , args_( args )
            , returns_( returns )
            , include_( include )
            {
                if( c_name_.len() == 0) {
                    if( awk_namespace_.len() == 0 )
                        c_name_ = awk_name_ + "_";
                    else
                        c_name_ = awk_namespace_ + "_::" + awk_name_ + "_";
                }
            }
        void set_type( SymbolType type, int token ){
            type_=type;
            token_=token;
        }
    };

    /**
     * Used for quick loading of the symbol table
    */
    struct _Symbol_loader {
        const char * awk_namespace_;
        const char * awk_name_;
        const char * c_name_;
        SymbolType type_;
        int token_ = 0;
        bool is_built_in_ = false;
        bool allow_regex_ = true;
        const char * args_ = "";
        const char * returns_ = "";
        const char * include_ = "";
    };

    /**
     * Used for quick loading of the symbol table into a namespace
    */
    struct _NS_Symbol_loader {
        const char * name_;
        SymbolType type_;
        int token_ = 0;
        bool is_built_in_ = false;
        bool allow_regex_ = true;
    };

    class SymbolTable {
    private:
        SymbolTable (const SymbolTable &) = delete;
    protected:
        SymbolTable ( SymbolTable * next = nullptr )
        : next_( next )
        {
        }
    public:
        virtual void insert(jclib::CountedPointer<Symbol>s) = 0;

        /// @brief SymbolTables may be chained.
        SymbolTable * next_;
        inline void insert(Symbol *s) {
            insert(jclib::CountedPointer<Symbol>(s));
        }

        inline void insert(Symbol &s) {
            insert(jclib::CountedPointer<Symbol>(&s));
        }

        virtual Symbol * insert( 
            class jclib::jString awk_namespace, /// AWK source namespace
            class jclib::jString awk_name, /// AWK source name
            class jclib::jString c_name, /// Name in Generated C++ 
            int token,
            SymbolType type = VARIABLE,
            bool is_built_in = false,
            const char * args_ = "",
            const char * returns_ = "",
            const char * include_ = "" ) = 0;

        virtual Symbol * find( jclib::jString &awk_namespace, jclib::jString &awk_name) = 0;

        /**
         * return existing Symbol or create a new one
        */
        virtual Symbol * get( 
                class jclib::jString awk_namespace,
                class jclib::jString awk_name,
                int token,
                bool namespace_required, /// true if forced by code
                SymbolType type = VARIABLE ) = 0;

        virtual void load(std::initializer_list<struct _Symbol_loader>) = 0;
        
        virtual void loadnamespace(const jclib::jString &namespace_name,
                    bool also_load_global,
                    std::initializer_list<struct _NS_Symbol_loader> input) = 0;

        static SymbolTable & instance();
        static SymbolTable & stack_instance();
        static SymbolTable & reset_instance();

        virtual ~SymbolTable() {}
    };
    enum node_types {
        Program,
        Pattern,
        Function,
        Expression,
        Statement,
        If,
        While,
        Do,
        For_C,
        For_Iterator,
        Operator,
        Empty
    };
    class    ast_node_visitor {
        public:
            virtual void visit_ast_node(class ast_node *)=0;
            virtual void visit_ast_empty_node(class ast_empty_node *)=0;
            virtual void visit_ast_statement_node(class ast_statement_node *)=0;
            virtual void visit_ast_op_node(class ast_op_node *)=0;
            virtual void visit_ast_left_unary_op_node(class ast_left_unary_op_node *)=0;
            virtual void visit_ast_right_unary_op_node(class ast_right_unary_op_node *)=0;
            virtual void visit_ast_bin_op_node(class ast_bin_op_node *)=0;
            virtual void visit_ast_function_node(class ast_function_node *)=0;
            virtual void visit_ast_branch_loop_node(class ast_branch_loop_node *)=0;
            virtual void visit_ast_for_loop_node(class ast_for_loop_node *)=0;
            virtual void visit_ast_ternary_op_node(class ast_ternary_op_node *)=0;
    };

    class ast_node: public jclib::CountedPointerTarget {
        public:
            node_types type_;
            jclib::jString name_;
            int rule_nr_;
            bool has_sym_;
            bool extra_children_allowed_;
            bool dummy_;
            jclib::CountedPointer<Symbol> sym_;
            std::vector<jclib::CountedPointer<ast_node> > sibling_nodes_;
            std::vector<jclib::CountedPointer<ast_node> > child_nodes_;
            // Despatcher for Visitors
            virtual void accept( ast_node_visitor * visitor ){
                visitor->visit_ast_node( this );}
            ast_node(   node_types type,
                        jclib::jString name,
                        int rule_nr = -1,
                        bool dummy = false)
            : type_( type )
            , name_( name )
            , rule_nr_( rule_nr )
            , sym_( nullptr )
            , has_sym_( false )
            , extra_children_allowed_(true)
            , dummy_( dummy )
            , sibling_nodes_()
            , child_nodes_()
            {
                sym_ = SymbolTable::instance().get(Empty_String, name_, false, 0, VARIABLE );
            }
            ast_node(   node_types type,
                        jclib::jString awk_namespace,
                        jclib::jString name,
                        bool namespace_required=true,
                        int rule_nr = -1,
                        bool dummy = false)
            : type_( type )
            , name_( name )
            , rule_nr_( rule_nr )
            , sym_( nullptr )
            , has_sym_( false )
            , extra_children_allowed_(true)
            , dummy_( dummy )
            , sibling_nodes_()
            , child_nodes_()
            {
                sym_ = SymbolTable::instance().get(awk_namespace, name_, namespace_required, 0, VARIABLE );
            }
            ast_node( char * full_name, node_types type, Symbol *sym, int rule_nr = -1, bool dummy = false)
            : type_( type )
            , name_(sym->awk_name_)
            , sym_(sym)
            , has_sym_( sym != nullptr )
            , extra_children_allowed_(true)
            , dummy_( dummy )
            , sibling_nodes_()
            , child_nodes_()
            , rule_nr_( rule_nr )
            {
            }
            ast_node( node_types type, jclib::CountedPointer<Symbol> sym, int rule_nr = -1, bool dummy = false)
            : type_( type )
            , name_(sym->awk_name_)
            , sym_(sym)
            , has_sym_( sym.isset() )
            , extra_children_allowed_(true)
            , dummy_( dummy )
            , rule_nr_( rule_nr )
            , sibling_nodes_()
            , child_nodes_()
            {
            }
            ast_node * add_sibling(jclib::CountedPointer<ast_node> adoptee) {
                if( rule_nr_ == -1 )
                    rule_nr_ = adoptee->rule_nr_;
                if( ! has_sym_ && adoptee->has_sym_ ) {
                    sym_ = adoptee->sym_;
                    has_sym_ = adoptee->has_sym_;
                }
                sibling_nodes_.push_back(adoptee);
                return this;
            }
            ast_node * add_child(jclib::CountedPointer<ast_node> adoptee) {
                if( rule_nr_ == -1 )
                    rule_nr_ = adoptee->rule_nr_;
                if( ! has_sym_ && adoptee->has_sym_ ) {
                    sym_ = adoptee->sym_;
                    has_sym_ = adoptee->has_sym_;
                }
                child_nodes_.push_back(adoptee);
                return this;
            }
            ast_node * add_children( std::initializer_list< jclib::CountedPointer<ast_node> > adoptees);

            /*** Recursively promote siblings to parent (if allowed) */
            virtual void clean_tree(ast_node * parent);
    };
    typedef  jclib::CountedPointer<awkccc::ast_node> ast_node_ptr;

    void print_ast( std::ostream & out, 
                    ast_node * node,
                    bool include_children=true,
                    bool include_siblings=true,
                    jclib::jString extra=jclib::jString::get_empty());


    class ast_empty_node: public ast_node {
        public:
            virtual void accept( ast_node_visitor * visitor ){
                visitor->visit_ast_empty_node( this );}
            ast_empty_node( node_types type, jclib::jString name, int rule_nr = -1, bool dummy = true)
                : ast_node( type, Empty_String, name, false, rule_nr, dummy )
            {}
            ast_empty_node( node_types type, jclib::jString namespace_, jclib::jString name, int rule_nr = -1, bool dummy = true)
                : ast_node( type, namespace_, name, false, rule_nr, dummy )
            {}
    };

    inline jclib::CountedPointer<ast_node> empty_node( jclib::jString rulename, int rule_nr = -1, bool dummy = true ) {
        return new ast_empty_node( Empty, rulename, rule_nr, dummy);
    }

    inline jclib::CountedPointer<ast_node> empty_node( const char * rulename, int rule_nr = -1 ) {
        return new ast_empty_node( Empty, jclib::jString(rulename), rule_nr);
    }
    
    class ast_statement_node: public ast_node {
        public:
            jclib::CountedPointer< ast_node > kw_node_;
            virtual void accept( ast_node_visitor * visitor ){
                visitor->visit_ast_statement_node( this );}
            ast_statement_node( ast_node * kw_node, awkccc::node_types type, jclib::jString name, int rule_nr = -1)
                : ast_node( type, Empty_String, name, false, rule_nr )
                , kw_node_( kw_node )
                {
                }
            ast_statement_node( jclib::CountedPointer< ast_node > kw_node, awkccc::node_types type, jclib::jString name, int rule_nr = -1)
                : ast_node( type, Empty_String, name, false, rule_nr )
                , kw_node_( kw_node )
                {
                }
    };

    inline jclib::CountedPointer<ast_node> statement_node( ast_node * kw_node, int rule_nr = -1 ) {
        return new ast_statement_node( kw_node, Statement, kw_node->sym_->awk_name_, rule_nr);
    }

    inline jclib::CountedPointer<ast_node> statement_node( ast_node * kw_node, std::initializer_list< jclib::CountedPointer<ast_node> > children, int rule_nr = -1 ) {
        auto answer = new ast_statement_node( kw_node, Statement, kw_node->sym_->awk_name_, rule_nr);
        answer->add_children( children);
        return answer;
    }

    class ast_op_node: public ast_node {
        public:
            jclib::CountedPointer< ast_node > op_node_;
            virtual void accept( ast_node_visitor * visitor ){
                visitor->visit_ast_op_node( this );
            }
            ast_op_node( ast_node * op, awkccc::node_types type, jclib::jString name, int rule_nr = -1)
                : ast_node( type, Empty_String, name, false, rule_nr )
                , op_node_( op )
                {
                }
            ast_op_node( jclib::CountedPointer< ast_node > op, awkccc::node_types type, jclib::jString name, int rule_nr = -1)
                : ast_node( type, Empty_String, name, false, rule_nr )
                , op_node_( op )
                {
                }
    };

    class ast_left_unary_op_node: public ast_op_node {
        public:
            virtual void accept( ast_node_visitor * visitor ){
                visitor->visit_ast_left_unary_op_node( this );
            }
            ast_left_unary_op_node( ast_node * op, ast_node * right, int rule_nr = -1)
                : ast_op_node( op, Expression, "LEFT UNARY_OP", rule_nr )
            {
                add_child(right);
            }
            ast_left_unary_op_node( jclib::CountedPointer<ast_node> op, jclib::CountedPointer<ast_node> right, int rule_nr = -1)
                : ast_op_node( op, Expression, "LEFT UNARY_OP", rule_nr )
            {
                add_child(right);
            }
    };
    inline jclib::CountedPointer<ast_node> ast_left_unary_op( jclib::CountedPointer<ast_node> op, jclib::CountedPointer<ast_node> right, int rule_nr = -1) {
        return new ast_left_unary_op_node( op, right, rule_nr);
    }

    class ast_right_unary_op_node: public ast_op_node {
        public:
            virtual void accept( ast_node_visitor * visitor ){
                visitor->visit_ast_right_unary_op_node( this );
            }
            ast_right_unary_op_node( ast_node * left, ast_node * op, int rule_nr = -1 )
                : ast_op_node( op, Expression, "RIGHT UNARY_OP", rule_nr )
            {
                add_child(left);
            }
    };
    inline jclib::CountedPointer<ast_node> ast_right_unary_op( ast_node * left, ast_node * op, int rule_nr = -1 ) {
        return new ast_right_unary_op_node( left, op, rule_nr);
    }

    class ast_bin_op_node: public ast_op_node {
        public:
            virtual void accept( ast_node_visitor * visitor ){
                visitor->visit_ast_bin_op_node( this );  }
            ast_bin_op_node( jclib::jString name, ast_node * left,  ast_node * op, ast_node * right, int rule_nr = -1)
                : ast_op_node( op, Expression, jclib::jString("BIN_OP "), rule_nr )
            {
                add_child(left);
                add_child(right);
            }
            ast_bin_op_node( jclib::CountedPointer<ast_node> left,  jclib::CountedPointer<ast_node> op, jclib::CountedPointer<ast_node> right, int rule_nr = -1)
                : ast_op_node( op, Expression,  jclib::jString("BIN_OP "), rule_nr )
            {
                add_child(left);
                add_child(right);
            }
    };

    inline jclib::CountedPointer<ast_node> ast_bin_op( jclib::CountedPointer<ast_node> left,  jclib::CountedPointer<ast_node> op, jclib::CountedPointer<ast_node> right, int rule_nr = -1) {
        return new ast_bin_op_node( left,  op, right, rule_nr);
    }

    inline jclib::CountedPointer<ast_node> ast_concatenate_expr( jclib::CountedPointer<ast_node> left, jclib::CountedPointer<ast_node> right, int rule_nr = -1) {
        auto kw = jclib::jString("@@@");
        Symbol * op_sym = SymbolTable::instance().find( Empty_String,kw);
        jclib::CountedPointer<ast_node> op = new ast_node( Operator, op_sym);
        return new ast_bin_op_node( left,  op, right, rule_nr);
    }

    class ast_function_node: public ast_node {
        public:
            jclib::CountedPointer<ast_node> function_;
            jclib::CountedPointer<ast_node> parameters_;
            jclib::CountedPointer<ast_node> body_;
            virtual void accept( ast_node_visitor * visitor ){
                visitor->visit_ast_function_node( this ); }
            ast_function_node( node_types type,
                    jclib::jString name,
                    jclib::CountedPointer<ast_node> function,
                    jclib::CountedPointer<ast_node> parameters,
                    jclib::CountedPointer<ast_node> body,
                    int rule_nr = -1)
                : ast_node( type, Empty_String, name, false, rule_nr, false)
                , function_(function)
                , parameters_(parameters)
                , body_(body)
            {
                extra_children_allowed_ = false;
            }

            /*** Recursively promote siblings to parent (if allowed) */
            virtual void clean_tree(ast_node * parent);
    };

    inline jclib::CountedPointer<ast_node> function_node( jclib::jString rulename,
                    jclib::CountedPointer<ast_node> function,
                    jclib::CountedPointer<ast_node> parameters,
                    jclib::CountedPointer<ast_node> body,
                    int rule_nr = -1 ) {
        return new ast_function_node( Function, rulename, function, parameters, body, rule_nr);
    }


    class ast_branch_loop_node: public ast_statement_node {
        public:
            jclib::CountedPointer<ast_node> question_;
            jclib::CountedPointer<ast_node> if_true_;
            jclib::CountedPointer<ast_node> if_false_;
            virtual void accept( ast_node_visitor * visitor ){
                visitor->visit_ast_branch_loop_node( this );  }
            ast_branch_loop_node( node_types type,
                    jclib::jString name,
                    ast_node * op,
                    ast_node * question,
                    ast_node * if_true,
                    ast_node * if_false,
                    int rule_nr = -1)
                : ast_statement_node( op, Expression, jclib::jString("Branch_Loop"), rule_nr )
                , question_(question)
                , if_true_(if_true)
                , if_false_(if_false)
            {
                extra_children_allowed_ = false;
            };
            ast_branch_loop_node(
                    node_types type,
                    jclib::CountedPointer<ast_node> op,
                    jclib::CountedPointer<ast_node> question,
                    jclib::CountedPointer<ast_node> if_true,
                    jclib::CountedPointer<ast_node> if_false,
                    int rule_nr = -1)
                : ast_statement_node( op, type,  jclib::jString("Branch_Loop"), rule_nr )
                , question_(question)
                , if_true_(if_true)
                , if_false_(if_false)
            {
                extra_children_allowed_ = false;
            }

            /*** Recursively promote siblings to parent (if allowed) */
            virtual void clean_tree(ast_node * parent);
    };
    inline jclib::CountedPointer<ast_node> ast_if_statement( ast_node * op, 
                                                             ast_node * question, 
                                                             ast_node * if_true, 
                                                             int rule_nr = -1) {
        return new ast_branch_loop_node( If, op, question, if_true, nullptr, rule_nr);
    }
    inline jclib::CountedPointer<ast_node> ast_if_else_statement( ast_node * op, 
                                                                  ast_node * question, 
                                                                  ast_node * if_true, 
                                                                  ast_node * if_false,
                                                                  int rule_nr = -1) {
        return new ast_branch_loop_node( If, op, question, if_true, if_false, rule_nr);
    }
    inline jclib::CountedPointer<ast_node> ast_while_statement( ast_node * op, 
                                                             ast_node * question, 
                                                             ast_node * if_true, 
                                                             int rule_nr = -1) {
        return new ast_branch_loop_node( While, op, question, if_true, nullptr, rule_nr);
    }
    inline jclib::CountedPointer<ast_node> ast_do_statement( ast_node * op, 
                                                             ast_node * body, 
                                                             ast_node * question, 
                                                             int rule_nr = -1) {
        return new ast_branch_loop_node( Do, op, question, body, nullptr, rule_nr);
    }

    class ast_for_loop_node: public ast_statement_node {
        public:
            jclib::CountedPointer<ast_node> initialise_;
            jclib::CountedPointer<ast_node> question_;
            jclib::CountedPointer<ast_node> increment_;
            jclib::CountedPointer<ast_node> loop_body_;
            virtual void accept( ast_node_visitor * visitor ){
                visitor->visit_ast_for_loop_node( this );   }
            ast_for_loop_node(
                    jclib::jString name,
                    ast_node * op,
                    ast_node * initialise,
                    ast_node * question,
                    ast_node * increment,
                    ast_node * loop_body,
                    int rule_nr = -1)
                : ast_statement_node( op, For_C, jclib::jString("Branch_Loop"), rule_nr )
                , initialise_(initialise)
                , question_(question)
                , increment_(increment)
                , loop_body_(loop_body)
            {
                extra_children_allowed_ = false;
            }
            ast_for_loop_node(
                    jclib::CountedPointer<ast_node> op,
                    jclib::CountedPointer<ast_node> initialise,
                    jclib::CountedPointer<ast_node> question,
                    jclib::CountedPointer<ast_node> increment,
                    jclib::CountedPointer<ast_node> loop_body,
                    int rule_nr = -1)
                : ast_statement_node( op, For_C,  jclib::jString("Branch_Loop"), rule_nr )
                , initialise_(initialise)
                , question_(question)
                , increment_(increment)
                , loop_body_(loop_body)
            {
                extra_children_allowed_ = false;
            }

            /*** Recursively promote siblings to parent (if allowed) */
            virtual void clean_tree(ast_node * parent);
    };
    inline jclib::CountedPointer<ast_node> ast_for_C_statement( ast_node * op, 
                                                                ast_node * initialise, 
                                                                ast_node * question,
                                                                ast_node * increment, 
                                                                ast_node * body, 
                                                                int rule_nr = -1) {
        return new ast_for_loop_node( op, initialise, question, increment, body, rule_nr);
    }


    class ast_ternary_op_node: public ast_op_node {
        public:
            jclib::CountedPointer<ast_node> question_;
            jclib::CountedPointer<ast_node> if_true_;
            jclib::CountedPointer<ast_node> if_false_;
            virtual void accept( ast_node_visitor * visitor ){
                visitor->visit_ast_ternary_op_node( this );   }
            ast_ternary_op_node( jclib::jString name,
                                 ast_node * op,
                                 ast_node * question,
                                 ast_node * if_true,
                                 ast_node * if_false,
                                 int rule_nr = -1)
                : ast_op_node( op, Expression, jclib::jString("TERNARY_OP "), rule_nr )
                , question_(question)
                , if_true_(if_true)
                , if_false_(if_false)
            {
                extra_children_allowed_ = false;
            };
            ast_ternary_op_node(
                    jclib::CountedPointer<ast_node> op,
                    jclib::CountedPointer<ast_node> question,
                    jclib::CountedPointer<ast_node> if_true,
                    jclib::CountedPointer<ast_node> if_false,
                    int rule_nr = -1)
                : ast_op_node( op, Expression,  jclib::jString("TERNARY_OP "), rule_nr )
                , question_(question)
                , if_true_(if_true)
                , if_false_(if_false)
            {
                extra_children_allowed_ = false;
            }

            /*** Recursively promote siblings to parent (if allowed) */
            virtual void clean_tree(ast_node * parent);
    };
    inline jclib::CountedPointer<ast_node> ast_ternary_op(  ast_node * question, ast_node * op, ast_node * if_true, ast_node * if_false, int rule_nr = -1) {
        return new ast_ternary_op_node( op, question, if_true, if_false, rule_nr);
    }

    inline jclib::CountedPointer<ast_node> ast_ternary_op(
                jclib::CountedPointer<ast_node> question,
                jclib::CountedPointer<ast_node> op,
                jclib::CountedPointer<ast_node> if_true,
                jclib::CountedPointer<ast_node> if_false,
                int rule_nr = -1) {
        return new ast_ternary_op_node( op, question, if_true, if_false, rule_nr);
    }
};
#endif