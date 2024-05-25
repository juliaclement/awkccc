/***
**
** AWKCCC c++ generator
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
#include <stdexcept>
#include "../include/jString.hpp"
#include "../include/Save.hpp"
#include "../include/awkccc.h++"
using namespace jclib;

namespace awkccc {
    /**
     * Class to walk the AST and create C++ code
    */
    class ast_to_cpp: public ast_node_visitor {
        public:
            struct cpp_generator_ctl {
                CountedPointer<ast_node> node_;
                jclib::jString padding_;
                bool include_children_;
                bool include_siblings_;
                jclib::jString extra_;
                int output_;
            };
            std::vector<postream > local_code_;
            std::vector<postream > & code_;
            postream out_;
            cpp_generator_ctl ctl_;
            /// @brief Construct with supplied code segment array. To support unit testing.
            /// @param node The top level node in the AST
            /// @param code code segment array
            /// @param include_children FIXME OBSOLETE?
            /// @param include_siblings FIXME OBSOLETE?
            /// @param extra String added before each output line
            ast_to_cpp( ast_node * node, std::vector< postream > & code, bool include_children, bool include_siblings,jclib::jString extra=jString::get_empty())
            : code_( code )
            , ctl_( {node, jString::get_empty(), include_children, include_siblings, extra })
            {
                if( code_.size() < template_NR_SEGMENTS ) {
                    throw std::invalid_argument("In ast_to_cpp::ast_to_cpp code argument too short" );
                }
                out_ = code_[template_body];
            }
            /// @brief Construct without supplied code segment array.
            /// @param node The top level node in the AST
            /// @param include_children FIXME OBSOLETE?
            /// @param include_siblings FIXME OBSOLETE?
            /// @param extra String added before each output line
            ast_to_cpp( ast_node * node, bool include_children, bool include_siblings,jclib::jString extra=jString::get_empty())
            : code_( local_code_ )
            , ctl_( {node, jString::get_empty(), include_children, include_siblings, extra })
            {
                while( local_code_.size() < template_NR_SEGMENTS ) {
                    std::ostringstream *x = new std::ostringstream; // Deleted in dtor to prevent leaking memory
                    local_code_.push_back( x );
                }
                out_ = code_[template_body];
            }
            ~ast_to_cpp() {
                for( auto x : local_code_ ) {
                    delete x;
                }
            }
            void traverse() {
                ctl_.node_->accept( this );
            }
            void print_header( ast_node * node ){
                (*out_) << ctl_.padding_ << ctl_.extra_ << node->name_;
                if( node->rule_nr_ == -1){
                    (*out_) << " (Lexer";
                        if( node->sym_ && node->name_.len() == 0) {
                            (*out_) << " " << node->sym_->awk_name_;
                        }
                    (*out_) << ") ";
                } else
                    (*out_) << " (" << node->rule_nr_<<") ";
                (*out_) << "--- children: " << node->child_nodes_.size();
                if( node->sibling_nodes_.size() > 0)
                    (*out_) << "--- siblings: " << node->sibling_nodes_.size();
                (*out_) << "\n";
                ctl_.extra_ = jString::get_empty();
            }

            // Base node
            void visit_ast_node( ast_node * node ){
                print_header(node);
                if( ctl_.include_children_ && node->child_nodes_.size() > 0 ) {
                    auto save_ctl = Save(ctl_);
                    ctl_.padding_ = ctl_.padding_ + "    ";
                    for( auto i : node->child_nodes_) {
                        i->accept( this );
                    }
                    (*out_) << ctl_.padding_ <<"-------" << "\n";
                }
                if( ctl_.include_siblings_ & node->sibling_nodes_.size()>0) {
                    for( auto s : node->sibling_nodes_ )
                        s->accept( this );
                }
            }
            // Empty node
            void visit_ast_empty_node( ast_empty_node * node ){
                visit_ast_node( node );
            }
            // Statement node
            void visit_ast_statement_node( ast_statement_node * node ){
                auto save_extra = Save(ctl_.extra_);
                jString extra = ctl_.extra_;
                ctl_.extra_ = ctl_.extra_ + "Statement "+node->kw_node_->name_+" ";
                visit_ast_node( node );
            }
            // Generic operator node
            void visit_ast_op_node( ast_op_node * node ){
                auto save_extra = Save(ctl_.extra_);
                ctl_.extra_ = ctl_.extra_ + "Operator "+node->op_node_->name_+" ";
                visit_ast_node( node );
            }
            // Left Unary operator (e.g -x, ++y) node
            void visit_ast_left_unary_op_node( ast_left_unary_op_node * node ){
                visit_ast_op_node( node );
            }
            // Right Unary operator (e.g x--) node
            void visit_ast_right_unary_op_node( ast_right_unary_op_node * node ){
                visit_ast_op_node( node );
            }
            // Binary operator (e.g. a += b) node
            void visit_ast_bin_op_node( ast_bin_op_node * node ){
                visit_ast_op_node( node );
            }
            // Function node
            void visit_ast_function_node( ast_function_node * node ){
                print_header(node);
                (*out_) << ctl_.padding_ << "function " << node->function_->sym_->awk_name_  << "\n";
                if( ctl_.include_children_ ) {
                    auto save_ctl = Save(ctl_);
                    ctl_.padding_ = ctl_.padding_ + "    ";
                    ctl_.include_children_ = true;
                    ctl_.include_siblings_ = true;
                    (*out_) << ctl_.padding_ << "Parameters:\n";
                    node->parameters_->accept( this );
                    (*out_) << ctl_.padding_ << "Body:\n";
                    node->body_->accept( this );
                    (*out_) << ctl_.padding_ <<"-------" << "\n";
                }
                if( ctl_.include_siblings_ ){
                    for( auto s : node->sibling_nodes_ ) {
                        s->accept(this);
                    }
                }
            }
            // Branch Loop node
            void visit_ast_branch_loop_node( ast_branch_loop_node * node ){
                print_header(node);
                if( ctl_.include_children_ ) {
                    auto save_ctl = Save(ctl_);
                    jString heading_padding = ctl_.padding_ + "    ";
                    ctl_.padding_ = heading_padding+ "    ";
                    ctl_.include_children_ = true;
                    ctl_.include_siblings_ = true;
                    (*out_) << heading_padding << "Condition:\n";
                    node->question_->accept( this );
                    (*out_) << heading_padding << "True body:\n";
                    node->if_true_->accept( this );
                    if( node->if_false_ != nullptr ) {
                        (*out_) << heading_padding << "Else body:\n";
                        node->if_false_->accept( this );
                    }
                    (*out_) << ctl_.padding_ <<"-------" << "\n";
                }
                if( ctl_.include_siblings_ ){
                    for( auto s : node->sibling_nodes_ ) {
                        s->accept( this );
                    }
                }
            }
            // For Loop node
            void visit_ast_for_loop_node( ast_for_loop_node * node ){
                print_header(node);
                if( ctl_.include_children_ ) {
                    auto save_ctl = Save(ctl_);
                    jString heading_padding = ctl_.padding_ + "    ";
                    ctl_.padding_ = heading_padding+ "    ";
                    ctl_.include_children_ = true;
                    ctl_.include_siblings_ = true;
                    (*out_) << heading_padding << "Initialise:\n";
                    node->initialise_->accept( this );
                    (*out_) << heading_padding << "Condition:\n";
                    node->question_->accept( this );
                    (*out_) << heading_padding << "Increment:\n";
                    node->increment_->accept( this );
                    (*out_) << heading_padding << "Body:\n";
                    node->loop_body_->accept( this );
                    (*out_) << ctl_.padding_ <<"-------" << "\n";
                }
                if( ctl_.include_siblings_ ){
                    for( auto s : node->sibling_nodes_ ) {
                        s->accept( this );
                    }
                }
            }
            // ternary op (xx?yy:zz) node
            void visit_ast_ternary_op_node( ast_ternary_op_node * node ){
                print_header(node);
                if( ctl_.include_children_ ) {
                    auto save_ctl = Save(ctl_);
                    jString heading_padding = ctl_.padding_ + "    ";
                    ctl_.padding_ = heading_padding+ "    ";
                    ctl_.include_children_ = true;
                    ctl_.include_siblings_ = true;
                    (*out_) << heading_padding << "Test:\n";
                    node->question_->accept( this );
                    (*out_) << heading_padding << "If True:\n";
                    node->if_true_->accept( this );
                    (*out_) << heading_padding << "If False:\n";
                    node->if_false_->accept( this );
                    (*out_) << ctl_.padding_ <<"-------" << "\n";
                }
                if( ctl_.include_siblings_ ){
                    for( auto s : node->sibling_nodes_ ) {
                        s->accept( this );
                    }
                }
            }
            
    };
    /**
     * Dump the contents of code to program_file stream controlled by template_text
     * Under normal circumstances this will only be called by generate_cpp, it is exposed for unit testing
    */
    void write_to_cpp(  std::istream & template_text,
                        std::ostream & program_file,
                        std::vector< postream > & code ){
        std::string markers[] = {"%includes;","%variables;","%procedures;","%begin;","%begin_file;","%body;","%end_file;","%end;"};
        std::string line;
        while( getline( template_text, line ) ){
            if( line[0] == '%' ) {
                bool found = false;
                for( int i = 0; i <= template_END; ++i ) {
                    auto kw = markers[i];
                    if( line.compare(0,kw.length(),kw)) {
                        found = true;
                        std::string out = code[i]->str();
                        program_file << out;
                        break;
                    }
                }
                if( ! found )
                    program_file << line << "\n";
            } else {
                program_file << line << "\n";
            }
        }
    }
    /**
     * Dump the contents of the ast to program_file stream controlled by template_text
     * Under normal circumstances this routine will be called by the other overload
     * This copy exposes generated code, and code parts for unit testing.
    */
    void generate_cpp(  std::istream & template_text,
                        std::ostream & program_file,
                        awkccc::ast_node * node,
                        std::vector<postream> code ){
        awkccc::ast_to_cpp generator(node, code, true, true, jclib::jString::get_empty() );
        generator.traverse();
        write_to_cpp( template_text, program_file, code );
    }
    /**
     * Dump the contents of the ast to program_file stream controlled by template_text
     * Under normal circumstances this routine will be called by the other overload
     * This copy exposes generated code for unit testing.
    */
    void generate_cpp( std::istream & template_text, std::ostream & program_file, awkccc::ast_node * node ){
        std::vector<postream> code;
        while( code.size() < template_NR_SEGMENTS ) {
            auto x = new std::ostringstream;
            code.push_back(x);
        }
        generate_cpp(template_text,program_file, node);
        for( auto x : code){
            delete x;
        }
    }
    /**
     * Dump the contents of the ast to filename_out stream controlled by template_filename
     * Under normal circumstances this is the routine to be called for this function
    */
    void generate_cpp( jString template_filename, jString filename_out, awkccc::ast_node * node ){
        std::ifstream template_text( template_filename );
        std::ofstream program_file( filename_out, std::ios_base::trunc | std::ios_base::out );
        generate_cpp(template_text,program_file, node);
        program_file.close();
        template_text.close();
    }
} // namespace awkccc
