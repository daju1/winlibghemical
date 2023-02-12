#pragma once
#include <string>
#include <iostream>
#include <fstream>

//---------------------------------------------------
// пространство имен, в которое заносятся детали реализации
namespace detail {
   //http://forum.sources.ru/index.php?showtopic=83356&view=showall
   //http://www.tdoc.ru/c/programming/cpp/cpp-traits-page4.html
   template <typename num_type, typename state_type = std::mbstate_t>
   class pos_type_t {
      typedef pos_type_t<num_type, state_type> my_type;

      num_type    m_pos;
      state_type  m_state;

      static state_type initial_state;

   public:
      // конструкторы
      pos_type_t(std::streampos off) : m_pos(off), m_state(initial_state) {}
      pos_type_t(num_type off = 0) : m_pos(off), m_state(initial_state) {}
      pos_type_t(state_type state, num_type pos) : m_pos(pos), m_state(state) {}

      // получение состояния потока
      state_type state() const {
         return(m_state);
      }

      // установка состояния потока
      void state(state_type st) {
         m_state = st;
      }

      // получение позиции
      num_type seekpos() const    {
         return(m_pos);
      }

      // оператор преобразования
      operator num_type() const {
         return(m_pos);
      }

      // далее идут операторы, которые осуществляют арифметические операции

      num_type operator- (const my_type& rhs) const {
         return(static_cast<num_type>(*this) - static_cast<num_type>(rhs));
      }

      my_type& operator+= (num_type pos) {
         m_pos += pos;
         return(*this);
      }

      my_type& operator-= (num_type pos) {
         m_pos -= pos;
         return(*this);
      }

      my_type operator+ (num_type pos) const {
         my_type tmp(*this);
         return(tmp += pos);
      }

      my_type operator- (num_type pos) const {
         my_type tmp(*this);
         return(tmp -= pos);
      }

      // операторы сравнения

      bool operator== (const my_type& rhs) const {
         return(static_cast<num_type>(*this) == static_cast<num_type>(rhs));
      }

      bool operator!= (const my_type& rhs) const {
         return(!(*this == rhs));
      }
   };
   //---------------------------------------------------
   // статическая константа, которая обозначает начальное состояние
   template <typename num_type, typename state_type>
   state_type pos_type_t<num_type, state_type>::initial_state;
}
//---------------------------------------------------
// наконец-то наш класс свойств:
template <typename char_t, typename long_pos_t>
struct long_pointer_traits : public std::char_traits<char_t> {
   // тип, используемый для представления позиции в потоке
   // определение pos_type через наш только что написанный класс
   typedef detail::pos_type_t<long_pos_t> pos_type;

   // тип, используемый для представления
   // смещений между позициями в потоке
   // определение off_type через тип, переданный во 2 аргументе шаблона
   typedef long_pos_t off_type;
};


//---------------------------------------------------
// вводим тип "длинного" файла
// "длинные" типы:

typedef std::basic_filebuf <char, long_pointer_traits<char, __int64> > long_filebuf;
typedef std::basic_ifstream<char, long_pointer_traits<char, __int64> > long_ifstream;
typedef std::basic_ofstream<char, long_pointer_traits<char, __int64> > long_ofstream;
typedef std::basic_fstream <char, long_pointer_traits<char, __int64> > long_fstream;

typedef std::basic_string  <char, long_pointer_traits<char, __int64> > long_string;

//---------------------------------------------------
/*int main(int argc, char** argv) {
long_ifstream infile(argv[1], std::ios::binary);
infile.seekg(0L, std::ios::end);
long_ifstream::pos_type nSize = infile.tellg();
std::cout << nSize;
return(0);

long_ifstream infile(strFileName, std::ios::binary);
long_string res;
std::getline(infile, res);
}*/

