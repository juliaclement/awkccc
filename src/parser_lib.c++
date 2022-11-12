/***
**
** AWKCCC Parser -- native C++ class
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

#include "../include/jString.hpp"
#include "../include/Save.hpp"
#include "../include/awkccc_ast.hpp"
using namespace jclib;

namespace awkccc {
    ast_node * ast_node::add_children( std::initializer_list< jclib::CountedPointer<ast_node> > adoptees) {
        for( auto i : adoptees )
            add_child(i);
        return this;;
    }
    
    class ast_printer: public ast_node_visitor {
        public:
            struct printer_ctl {
                CountedPointer<ast_node> node_;
                jclib::jString padding_;
                bool include_children_;
                bool include_siblings_;
                jclib::jString extra_;
            };
            std::ostream & out_;
            printer_ctl ctl_;
            ast_printer( std::ostream & out, ast_node * node, bool include_children, bool include_siblings,jclib::jString extra=jString::get_empty())
            : out_( out )
            , ctl_( {node, jString::get_empty(), include_children, include_siblings, extra })
            {
            }
            void traverse() {
                ctl_.node_->accept( this );
            }
            void print_header( ast_node * node ){
                out_ << ctl_.padding_ << ctl_.extra_ << node->name_;
                if( node->rule_nr_ == -1){
                    out_ << " (Lexer";
                        if( node->sym_ && node->name_.len() == 0) {
                            out_ << " " << node->sym_->name_;
                        }
                    out_ << ") ";
                } else
                    out_ << " (" << node->rule_nr_<<") ";
                out_ << "--- children: " << node->child_nodes_.size();
                if( node->sibling_nodes_.size() > 0)
                    out_ << "--- siblings: " << node->sibling_nodes_.size();
                out_ << "\n";
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
                    out_ << ctl_.padding_ <<"-------" << "\n";
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
                out_ << ctl_.padding_ << "function " << node->function_->sym_->name_  << "\n";
                if( ctl_.include_children_ ) {
                    auto save_ctl = Save(ctl_);
                    ctl_.padding_ = ctl_.padding_ + "    ";
                    ctl_.include_children_ = true;
                    ctl_.include_siblings_ = true;
                    out_ << ctl_.padding_ << "Parameters:\n";
                    node->parameters_->accept( this );
                    out_ << ctl_.padding_ << "Body:\n";
                    node->body_->accept( this );
                    out_ << ctl_.padding_ <<"-------" << "\n";
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
                    out_ << heading_padding << "Condition:\n";
                    node->question_->accept( this );
                    out_ << heading_padding << "True body:\n";
                    node->if_true_->accept( this );
                    if( node->if_false_ != nullptr ) {
                        out_ << heading_padding << "Else body:\n";
                        node->if_false_->accept( this );
                    }
                    out_ << ctl_.padding_ <<"-------" << "\n";
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
                    out_ << heading_padding << "Initialise:\n";
                    node->initialise_->accept( this );
                    out_ << heading_padding << "Condition:\n";
                    node->question_->accept( this );
                    out_ << heading_padding << "Increment:\n";
                    node->increment_->accept( this );
                    out_ << heading_padding << "Body:\n";
                    node->loop_body_->accept( this );
                    out_ << ctl_.padding_ <<"-------" << "\n";
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
                    out_ << heading_padding << "Test:\n";
                    node->question_->accept( this );
                    out_ << heading_padding << "If True:\n";
                    node->if_true_->accept( this );
                    out_ << heading_padding << "If False:\n";
                    node->if_false_->accept( this );
                    out_ << ctl_.padding_ <<"-------" << "\n";
                }
                if( ctl_.include_siblings_ ){
                    for( auto s : node->sibling_nodes_ ) {
                        s->accept( this );
                    }
                }
            }
            
    };

    void print_ast( std::ostream & out, ast_node * node, bool include_children, bool include_siblings,jclib::jString extra ){
        ast_printer printer(out, node, include_children, include_children, extra );
        printer.traverse();
    }

    // Normalise the sibling arrays by promoting each node's sibling's to be its
    // parent's children.
    // Requires walking the tree
    void ast_node::clean_tree(ast_node * parent) {
        if(sibling_nodes_.size() > 0 ) {
            if( parent != nullptr && parent->extra_children_allowed_ ) {
                // Move my siblings to parent's children
                for( auto child : sibling_nodes_ ) {
                    parent->child_nodes_.push_back(child);
                    child->clean_tree( parent );
                }
                sibling_nodes_.clear();
            }
            else{
                    // Notify my siblings
                for( auto child : sibling_nodes_ ) {
                    child->clean_tree( parent );
                }
            }
        }
        if( child_nodes_.size() > 0 ) {
            if( extra_children_allowed_ ) {
                // Add children's siblings. 
                // A bit complex as we may be inserting in the middle of the child array
                // We solve this by copying to a temporary
                std::vector<jclib::CountedPointer<ast_node> > temp_children(child_nodes_);
                child_nodes_.clear();
                for( auto child : temp_children ) {
                    child_nodes_.push_back(child);
                    child->clean_tree( this );
                }
            } else {
                // Still process the children's children
                for( auto child : child_nodes_) {
                    child->clean_tree( this );
                }
            }
            // We may have dummy child notes that are empty
            bool empty_children_exist = false;
            for( auto child : child_nodes_) {
                if (child->dummy_ && child->child_nodes_.empty() && child->sibling_nodes_.empty()) {
                    empty_children_exist = true;
                    break;
                }
            }
            if( empty_children_exist ) {
                std::vector<jclib::CountedPointer<ast_node> > temp_children(child_nodes_);
                child_nodes_.clear();
                for( auto child : temp_children) {
                    if( (!child->dummy_) || (!child->child_nodes_.empty()) || (! child->sibling_nodes_.empty()))
                        child_nodes_.push_back(child);
                }
            }
        }
    }

    // Normalise the sibling arrays by promoting each node's siblings to be its
    // parent's children.
    // Requires walking the tree
    void ast_branch_loop_node::clean_tree(ast_node * parent) {
        // 1 Promote our siblings
        ast_node::clean_tree( parent ); 
        // 2 Tell our dependant nodes to clean themselves
        question_->clean_tree( this );
        if_true_->clean_tree( this );
        if( if_false_ != nullptr )
            if_false_ -> clean_tree( this );
    }

    // Normalise the sibling arrays by promoting each node's siblings to be its
    // parent's children.
    // Requires walking the tree
    void ast_for_loop_node::clean_tree(ast_node * parent) {
        // 1 Promote our siblings
        ast_node::clean_tree( parent ); 
        // 2 Tell our dependant nodes to clean themselves
        initialise_->clean_tree( this );
        question_->clean_tree( this );
        increment_->clean_tree( this );
        loop_body_ -> clean_tree( this );
    }

    // Normalise the sibling arrays by promoting each node's siblings to be its
    // parent's children.
    // Requires walking the tree
    void ast_function_node::clean_tree(ast_node * parent) {
        // 1 Promote our siblings
        ast_node::clean_tree( parent ); 
        // 2 Tell our dependant nodes to clean themselves
        function_->clean_tree( this );
        parameters_->clean_tree( this );
        body_->clean_tree( this );
    }

    // Normalise the sibling arrays by promoting each node's siblings to be its
    // parent's children.
    // Requires walking the tree
    void ast_ternary_op_node::clean_tree(ast_node * parent) {
        // 1 Promote our siblings
        ast_node::clean_tree( parent ); 
        // 2 Tell our dependant nodes to clean themselves
        question_->clean_tree( this );
        if_true_->clean_tree( this );
        if_false_->clean_tree( this );
    }
};