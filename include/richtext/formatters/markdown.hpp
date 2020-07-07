#pragma once


#include <string>
#include <cstdio>
#include "../richtext.hpp"


namespace richtext::formatters {


class markdown: public formatter {
public:

  struct options {
    std::size_t margin{ 80 };
  };

  markdown() noexcept = default;
  markdown(markdown const&) = delete;
  markdown& operator = (markdown const&) = delete;
  explicit operator bool() const noexcept { return file_ != nullptr; }
  
  

private:

  FILE* file_{nullptr};

};



}
