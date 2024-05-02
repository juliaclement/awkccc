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
#ifndef AWKCCC_VARIABLE_HPP
#define AWKCCC_VARIABLE_HPP 1
#include <charconv>
#include <cstdio> // FIXME replace with <format> once C++20 in all target systems
#include <cmath>
#include "../include/jString.hpp"
namespace awkccc {
/** A variable at any moment may be any of:
 *  uninitalised equivalent to "" and 0.0,
 *  a string,
 *  a number (represented by a double),
 *  a numeric string as defined in IEEE Std 1003.1-2017.
 */
enum Awkccc_data_type{
    Uninitialised,
    String,
    Number,
    Numeric_String
};

/** AWK variables are untyped, but coerce their values to string or number on demand*/
class Awkccc_variable {
    public:
        static constexpr double epsilon_ = 1e-6;
        jclib::jString string_;
        double number_;
        Awkccc_data_type data_type_;
        bool number_is_valid_;
        bool string_is_valid_;
        Awkccc_variable()
            : string_( jclib::jString::get_empty() )
            , number_(0.0)
            , data_type_( Uninitialised )
            , number_is_valid_( true )
            , string_is_valid_( true )
            {}
        Awkccc_variable( const Awkccc_variable &old )
            : string_( old.string_ )
            , number_( old.number_ )
            , data_type_( old.data_type_ )
            , number_is_valid_( old.number_is_valid_ )
            , string_is_valid_( old.string_is_valid_ )
            {}
        Awkccc_variable( const jclib::jString & string )
            : string_( string )
            , data_type_( String )
            , number_is_valid_( false )
            , string_is_valid_( true )
            {}
        Awkccc_variable( double number )
            : string_( jclib::jString::get_empty() )
            , number_ (number )
            , data_type_( Number )
            , number_is_valid_( true )
            , string_is_valid_( false )
            {}
        /** Create a Numeric String */
        Awkccc_variable( const jclib::jString & string, double number )
            : string_( string )
            , number_ (number )
            , data_type_( Numeric_String )
            , number_is_valid_( true )
            , string_is_valid_( true )
            {}
        /** Ensure number_ is valid */
        Awkccc_variable & ensure_double() {
            std::string_view sv( string_);
            std::from_chars(sv.begin(), sv.end(), number_);
            number_is_valid_ = true;
            return *this;
        }
        /** Get or create double value, may change this despite constness */
        inline operator double() const {
            return number_is_valid_ ? number_ : const_cast<Awkccc_variable*>(this)->ensure_double().number_;
        }
        /** Get or create double value, may change this */
        inline operator double() {
            return number_is_valid_ ? number_ : ensure_double().number_;
        }
        /** Get or create long long value using operator double. Required for % operator */
        inline operator long long() const {
            return (long long)( double(*this));
        }
        /** Get or create jString value.
         *  FIXME 1 use OFMT / CONVFMT depending on context
         *  FIXME 2 replace with <format> once C++20 in all target systems
         *          test __cpp_lib_format
         * */ 
        jclib::jString format() {
            if( string_is_valid_ )
                return string_;
            double intpart;
            char buf[48];
            if( std::fabs(std::modf(number_,&intpart)) < epsilon_ ){
                std::snprintf(buf, 48, "%lld", (long long)number_);
            } else {
                std::snprintf(buf, 48, "%.6g", number_);
            }
            return buf;
        }
        inline operator jclib::jString() const {
            return string_is_valid_ ? string_ : const_cast<Awkccc_variable*>(this)->format();
        }
        Awkccc_variable & operator = ( const Awkccc_variable &old ){
            string_ = old.string_;
            number_ = old.number_;
            data_type_ = old.data_type_;
            number_is_valid_ = old.number_is_valid_ ;
            return *this;
        }
        
        template< typename T > double operator + ( const T &&old ) const {
            return double(*this) + double(old);
        }
        template< typename T > double operator - ( const T &&old ) const {
            return double(*this) - double(old) ;
        }
        template< typename T > double operator * ( const T &&old ) const {
            return double(*this) + double(old) ;
        }
        template< typename T > double operator / ( const T &&old ) const {
            return double(*this) / double(old) ;
        }
        /** Modulo operator %.
         *  C++ standard requires that both operators are an integral type,
         *  but we store numbers as double so we force the issue.
         *  Datatype long long was chosen as it gives the greatest chance of 
         *  a correct translation.
         * */
        template< typename T > long long operator % ( const T &&old ) const {
            return ((long long) (*this)) % ((long long) (old)) ;
        }
        // prefix operator ++x
        long long operator ++() {
            ensure_double();
            string_is_valid_ = false;
            return ++number_;
        }
        // postfix operator x++
        long long operator ++(int) {
            ensure_double();
            string_is_valid_ = false;
            return number_++;
        }
        // prefix operator --x
        long long operator --() {
            ensure_double();
            string_is_valid_ = false;
            return --number_;
        }
        // postfix operator x--
        long long operator --(int) {
            ensure_double();
            string_is_valid_ = false;
            return --number_;
        }
        /** Comparison operators.
         * All comparisons are routed through the single compare() routine which returns negative, 0 or
         * positive with the same meaning as strcmp.
         * 
         * POSIX 2017 states:
         * " Comparisons (with the '<', "<=", "!=", "==", '>', and ">=" operators) shall be made
         *   numerically if both operands are numeric, if one is numeric and the other has a string
         *   value that is a numeric string, or if one is numeric and the other has the uninitialized
         *   value.
         *   Otherwise, operands shall be converted to strings as required and a string comparison
         *   shall be made as follows:
         *   For the "!=" and "==" operators, the strings should be compared to check if they are
         *   identical but may be compared using the locale-specific collation sequence to check if
         *   they collate equally.
         *   For the other operators, the strings shall be compared using the locale-specific collation
         *   sequence.
         *   The value of the comparison expression shall be 1 if the relation is true, or 0 if the
         *  relation is false."
         * FIXME: Haven't yet implemented locale-specific collation as required by the standard
         */
        int compare( const Awkccc_variable &rhs ) const {
            if( number_is_valid_ && rhs.number_is_valid_ ){
                const double diff = number_ - rhs.number_;
                return  ( fabs( diff ) < epsilon_ )
                        ? 0
                        : (diff < 0)
                            ? -1
                            : 1;
            }
            jclib::jString lhss(*this),rhss(rhs);
            return lhss.compare( rhss );
        }
        bool operator <( const Awkccc_variable &rhs ) const {
            return compare( rhs ) < 0;
        }
        bool operator <=( const Awkccc_variable &rhs ) const {
            return compare( rhs ) <= 0;
        }
        bool operator >( const Awkccc_variable &rhs ) const {
            return compare( rhs ) > 0;
        }
        bool operator >=( const Awkccc_variable &rhs ) const {
            return compare( rhs ) >= 0;
        }
        bool operator ==( const Awkccc_variable &rhs ) const {
            return compare( rhs ) == 0;
        }
        bool operator !=( const Awkccc_variable &rhs ) const {
            return compare( rhs ) != 0;
        }
    };
    namespace {
        template< typename T > inline double operator + ( const T &&left, const Awkccc_variable && right ) {
            return double(left) + double(right);
        }
        template< typename T > inline double operator - ( const T &&left, const Awkccc_variable && right ) {
            return double(left) - double(right) ;
        }
        template< typename T > inline double operator * ( const T &&left, const Awkccc_variable && right ) {
            return double(left) + double(right) ;
        }
        template< typename T > inline double operator / ( const T &&left, const Awkccc_variable && right ) {
            return double(left) - double(right) ;
        }
        template< typename T > inline long long operator % ( const T &&left, const Awkccc_variable && right ) {
            return (long long) (left) % (long long) (right) ;
        }
    }
}
#endif
