/* This file is part of uformat library
 * Copyright 2020 Andrei Ilin <ortfero@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once


#include <cstdint>
#include <utility>
#include <string_view>
#include <cmath>
#include <mutex>
#include <cstdio>
#include "fixed_string.hpp"
#include "continuous_string.hpp"


namespace uformat {


  enum class alignment {
    left, right
  };



  template<typename S>
  class texter {
  public:

    using size_type = size_t;
    using string_type = S;

    static constexpr auto default_precision = 6;

    texter() noexcept = default;
    texter(texter const&) = default;
    texter& operator = (texter const&) = default;
    texter(texter&&) noexcept = default;
    texter& operator = (texter&&) noexcept = default;

    S const& string() const noexcept { return string_; }
    char const* data() const noexcept { return string_.data(); }
    size_type size() const noexcept { return string_.size(); }
    bool empty() const noexcept { return string_.empty(); }
    void clear() noexcept { string_.clear(); }


    size_type capacity() const noexcept {
      return string_.capacity();
    }


    bool filled() const noexcept {
      return string_.size() == string_.max_size();
    }


    void reserve(size_type n) {

      if(n <= string_.capacity())
        return;

      n = nearest_power_of_2(n);
      string_.reserve(n);
    }


    texter& append(char const* cc, size_type n) {
      string_.append(cc, n);
      return *this;
    }


    texter& char_n(char c, size_type n) {
      size_type allocated;
      char* buffer = allocate(n, allocated);
      if (!buffer) return *this;
      char* p = buffer;
      for (char* p = buffer; p != buffer + n; ++p)
        *p = c;
      return shrink(allocated - n);
    }


    template<typename F, typename Arg> texter& as(Arg&& arg, F const& f = F{}) {
      f.print(*this, std::forward<Arg>(arg));
      return *this;
    }


    template<typename Arg>
    texter& align(alignment alignment, size_type width, Arg&& arg) {
      switch (alignment) {
      case alignment::right:
        return right(width, std::forward<Arg>(arg));
      default:
        return left(width, std::forward<Arg>(arg));
      }
    }


    template<typename Arg>
    texter& left(size_type width, Arg&& arg) {
      size_type const previous_size = string_.size();
      (*this) << arg;
      size_type const next_size = string_.size();
      size_type const n = next_size - previous_size;
      if (n >= width)
        return *this;
      size_type const spaces_count = width - n;
      char_n(' ', spaces_count);
      return *this;
    }


    template<typename Arg>
    texter& right(size_type width, Arg&& arg) {
      size_type const previous_size = string_.size();
      (*this) << arg;
      size_type const next_size = string_.size();
      size_type const n = next_size - previous_size;
      if (n >= width)
        return *this;
      size_type const spaces_count = width - n;
      char_n(' ', spaces_count);
      for (size_type i = next_size - 1; i != previous_size - 1; --i)
        string_[i + spaces_count] = string_[i];
      for (size_type i = 0; i != spaces_count; ++i)
        string_[previous_size + i] = ' ';
      return *this;
    }


    texter& print() {
      return *this;
    }


    template<typename Arg, typename... Args>
    texter& print(Arg&& arg, Args&&... args) {
      (*this) << arg;
      print(std::forward<Args>(args)...);
      return *this;
    }


    texter& fixed(double x, unsigned precision) {
      return print_fixed_float(x, precision);
    }


    texter& fixed(float x, unsigned precision) {
      return print_fixed_float(x, precision);
    }


    texter& fixed(uint32_t x, unsigned width) {
      constexpr auto digits = 10;
      return print_fixed_int<digits>(x, width);
    }


    texter& fixed(int32_t x, unsigned width) {
      constexpr auto digits = 11;
      return print_fixed_int<digits>(x, width);
    }


    texter& fixed(uint64_t x, unsigned width) {
      constexpr auto digits = 18;
      return print_fixed_int<digits>(x, width);
    }


    texter& fixed(int64_t x, unsigned width) {
      constexpr auto digits = 19;
      return print_fixed_int<digits>(x, width);
    }


    template<typename T> texter& quoted(T&& arg) {
      string_.push_back('\'');
      (*this) << arg;
      string_.push_back('\'');
      return *this;
    }


    template<typename T> texter& dquoted(T&& arg) {
      string_.push_back('\"');
      (*this) << arg;
      string_.push_back('\"');
      return *this;
    }


    texter& attributes() {
      return *this;
    }


    template<typename Arg, typename... Pairs>
    texter& attributes(std::string_view const& name, Arg&& value, Pairs&&... pairs) {
      string_.push_back('{');
      string_.append(name.data(), name.size());
      string_.push_back(' ');
      string_.push_back('=');
      string_.push_back(' ');
      format_value(std::forward<Arg>(value));
      format_other_attributes(std::forward<Pairs>(pairs)...);
      string_.push_back('}');
      return *this;
    }


    template<typename X>
    friend texter& operator << (texter& p, texter<X> const& q) {
      p.string_.append(q.data(), q.size());
      return p;
    }


    friend texter& operator << (texter& p, char c) {
      p.string_.push_back(c);
      return p;
    }


    friend texter& operator << (texter& p, char const* cc) {
      p.string_.append(cc);
      return p;
    }


    template<size_t N > friend
    texter& operator << (texter& p, char const (&s)[N]) {
      p.string_.append(s, N - 1);
      return p;
    }


    friend texter& operator << (texter& p, std::string_view const& sv) {
      return p.append(sv.data(), sv.size());
    }


    friend texter& operator << (texter& p, std::string const& s) {
      return p.append(s.data(), s.size());
    }


    friend texter& operator << (texter& p, bool x) {
      return x ? p.append("true", 4) : p.append("false", 5);
    }


    friend texter& operator << (texter& p, uint32_t x) {
      constexpr auto digits = 10;
      return p.print_int<digits>(x);
    }


    friend texter& operator << (texter& p, int32_t x) {
      constexpr auto digits = 11;
      return p.print_int<digits>(x);
    }


    friend texter& operator << (texter& p, uint64_t x) {
      constexpr auto digits = 18;
      return p.print_int<digits>(x);
    }


    friend texter& operator << (texter& p, int64_t x) {
      constexpr auto digits = 19;
      return p.print_int<digits>(x);
    }


    friend texter& operator << (texter& p, float x) {
      return p.print_fixed_float(x, default_precision);
    }


    friend texter& operator << (texter& p, double x) {
      return p.print_fixed_float(x, default_precision);
    }


    template<size_t N>
    friend texter& operator << (texter& p, fixed_string<N> const& fs) {
      p.out(fs.data(), fs.size());
      return p;
    }


  private:

    S string_;


    static uint64_t nearest_power_of_2(uint64_t n) {
      if(n < 2)
        return 2;
      n--;
      n |= n >> 1;
      n |= n >> 2;
      n |= n >> 4;
      n |= n >> 8;
      n |= n >> 16;
      n |= n >> 32;
      n++;
      return n;
    }


    void format_other_attributes()
    { }


    template<typename Arg, typename... Pairs>
    void format_other_attributes(std::string_view const& name, Arg&& value, Pairs&&... pairs) {
      string_.push_back(',');
      string_.push_back(' ');
      string_.append(name.data(), name.size());
      string_.push_back(' ');
      string_.push_back('=');
      string_.push_back(' ');
      format_value(std::forward<Arg>(value));
      format_other_attributes(std::forward<Pairs>(pairs)...);
    }


    void format_value(char c) {
      string_.push_back('\"');
      string_.push_back(c);
      string_.push_back('\"');
    }


    template<size_t N> void format_value(char const (&literal)[N]) {
      string_.push_back('\"');
      string_.append(literal, N - 1);
      string_.push_back('\"');
    }


    void format_value(char const* data) {
      string_.push_back('\"');
      string_.append(data);
      string_.push_back('\"');
    }


    void format_value(std::string const& s) {
      string_.push_back('\"');
      string_.append(s.data(), s.size());
      string_.push_back('\"');
    }


    void format_value(std::string& s) {
      format_value(static_cast<std::string const&>(s));
    }


    template<size_t N> void format_value(fixed_string<N> const& s) {
      string_.push_back('\"');
      string_.append(s.data(), s.size());
      string_.push_back('\"');
    }


    template<size_t N> void format_value(fixed_string<N>& s) {
      format_value(static_cast<fixed_string<N> const&>(s));
    }



    void format_value(std::string_view const& sv) {
      string_.push_back('\"');
      string_.append(sv.data(), sv.size());
      string_.push_back('\"');
    }


    void format_value(std::string_view& sv) {
      format_value(static_cast<std::string_view const&>(sv));
    }


    template<typename T> void format_value(T&& arg) {
      (*this) << arg;
    }


    char* allocate(size_type n, size_type& m) {

      size_type const old_size = string_.size();
      size_type new_size = old_size + n;

      if(new_size > string_.capacity()) {
        new_size = nearest_power_of_2(new_size);
        m = size_type(new_size - old_size);
      }
      else {
        m = n;
      }

      string_.resize(new_size);

      if(string_.size() != new_size)
        return nullptr;
      else
        return &string_[old_size];
    }


    texter& shrink(size_type n) {
      string_.resize(string_.size() - n);
      return *this;
    }


    template<unsigned N, typename T> texter& print_int(T x) {
      size_type digits;
      char* buffer = allocate(N, digits);
      if(!buffer) return *this;
      char* p = buffer;
      convert(x, p);
      shrink(digits - (p - buffer));
      return *this;
    }


    template<unsigned N, typename T> texter& print_fixed_int(T x, unsigned width) {
      if(width > N)
        width = N;
      size_type digits;
      char* buffer = allocate(width, digits);
      if(!buffer) return *this;
      char* p = buffer;
      convert(x, p);
      unsigned const actual_width = unsigned(p - buffer);
      if(actual_width >= width)
        return shrink(digits - actual_width);
      unsigned const leadings_count = width - actual_width;
      for(p = buffer + actual_width - 1; p != buffer + leadings_count - 1; --p)
        p[leadings_count] = p[0];
      for(; p != buffer - 1; --p) {
        p[leadings_count] = p[0];
        p[0] = '0';
      }
      shrink(digits - width);
      return *this;
    }


    template<typename T> texter& print_fixed_float(T x, unsigned precision) {
      size_type digits;
      char* buffer = allocate(38, digits);
      if(!buffer) return *this;
      char* p = buffer;
      convert(double(x), p, precision);
      return shrink(digits - (p - buffer));
    }


    static void convert(double x, char*& p, unsigned precision) {
      static double pow10[18] = {
        1E0, 1E1, 1E2, 1E3, 1E4, 1E5, 1E6, 1E7, 1E8, 1E9, 1E10, 1E11, 1E12, 1E13,
        1E14, 1E15, 1E16, 1E17
      };
      if(precision > 16)
        precision = 16;
      if(std::isinf(x)) {
        *p++ = 'I'; *p++ = 'N'; *p++ = 'F';
        return;
      }
      if(std::isnan(x)) {
        *p++ = 'N'; *p++ = 'a'; *p++ = 'N';
        return;
      }
      if(x < 0.0) {
        *p++ = '-';
        x = -x;
      }
      
      uint64_t integer = uint64_t(x);
      convert(integer, p);
      *p++ = '.';
      x -= double(integer);
      x *= pow10[precision];
      x += 0.5;
      convert(uint64_t(x), p);
    }


    static void convert(uint32_t x, char*& p) {
      if(x < 100000000) {
        if(x == 0)
          *p++ = '0';
        else
          begin8(p, x);
      } else {
        begin2(p, x / 100000000);
        middle8(p, x % 100000000);
      }
    }


    static void convert(int32_t x, char*& p) {
      if(x < 0) {
        *p++ = '-';
        x = -x;
      }
      convert(uint32_t(x), p);
    }


    static void convert(uint64_t x, char*& p) {
      if(x < 100000000) {
        if(x == 0)
          *p++ = '0';
        else
          begin8(p, uint32_t(x));
      } else if(x < 10000000000000000) {
        begin8(p, uint32_t(x / 100000000));
        middle8(p, x % 100000000);
      } else {
        begin4(p, uint32_t(x / 10000000000000000));
        middle16(p, x % 10000000000000000);
      }
    }


    static void convert(int64_t x, char*& p) {
      if(x < 0) {
        *p++ = '-';
        x = -x;
      }
      convert(uint64_t(x), p);
    }


    static void begin2(char*& p, uint32_t n) {
      if(n < 10)
        *p++ = char('0' + n);
      else
        middle2(p, n);
    }


    static void middle2(char*& p, uint32_t n) {
      static char const digits_lut[200] = {
        '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8','0','9',
        '1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7','1','8','1','9',
        '2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7','2','8','2','9',
        '3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7','3','8','3','9',
        '4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7','4','8','4','9',
        '5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7','5','8','5','9',
        '6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7','6','8','6','9',
        '7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9',
        '8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',
        '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8','9','9'
      };
      uint32_t const t = n * 2;
      *p++ = digits_lut[t];
      *p++ = digits_lut[t + 1];
    }


    static void begin4(char*& p, uint32_t n) {
      if(n < 100)
        begin2(p, n);
      else {
        begin2(p, n / 100);
        middle2(p, n % 100);
      }
    }


    static void middle4(char*& p, uint32_t n) {
      middle2(p, n / 100);
      middle2(p, n % 100);
    }


    static void begin8(char*& p, uint32_t n) {
      if(n < 10000)
        begin4(p, n);
      else {
        begin4(p, n / 10000);
        middle4(p, n % 10000);
      }
    }


    static void middle8(char*& p, uint32_t n) {
      middle4(p, n / 10000);
      middle4(p, n % 10000);
    }


    static void middle16(char*& p, uint64_t n) {
      middle8(p, uint32_t(n / 100000000));
      middle8(p, uint32_t(n % 100000000));
    }
  }; // texter


  using dynamic_texter = texter<std::string>;
  using short_texter = texter<short_string>;
  using fixed_texter = texter<string>;
  using long_texter = texter<long_string>;
  using page_texter = texter<page_string>;
  using dpage_texter = texter<dpage_string>;
  using large_texter = texter<large_string>;
  using continuous_texter = texter<continuous_string<>>;


  namespace detail::printer {

    inline std::mutex sync;
    inline dynamic_texter buffer;

  }


  template<typename... Args>
  void print(Args&&... args) {
    std::unique_lock g{detail::printer::sync};
    detail::printer::buffer.clear();
    detail::printer::buffer.print(std::forward<Args>(args)...);
    std::fputs(detail::printer::buffer.data(), stdout);
    std::fputc('\n', stdout);
  }


  template<typename R, typename... Args>
  R print_with(R&& result, Args&&... args) {
    print(std::forward<Args>(args)...);
    return std::move(std::forward<R>(result));
  }


  template<typename... Args>
  void error(Args&&... args) {
    std::unique_lock g{detail::printer::sync};
    detail::printer::buffer.clear();
    detail::printer::buffer.print(std::forward<Args>(args)...);
    std::fputs(detail::printer::buffer.data(), stderr);
    std::fputc('\n', stderr);
  }


  template<typename R, typename... Args>
  R error_with(R&& result, Args&&... args) {
    error(std::forward<Args>(args)...);
    return std::move(std::forward<R>(result));
  }


} // uformat
