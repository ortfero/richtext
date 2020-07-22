#pragma once


#include <cstdint>
#include <string_view>
#include <string>
#include <stdexcept>


#if defined(_WIN32)

#if !defined(_X86_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_)
#if defined(_M_IX86)
#define _X86_
#elif defined(_M_AMD64)
#define _AMD64_
#elif defined(_M_ARM)
#define _ARM_
#elif defined(_M_ARM64)
#define _ARM64_
#endif
#endif

#include <minwindef.h>
#include <memoryapi.h>
#include <handleapi.h>

#else
  
#error Unsupported system

#endif // WIN32



namespace uformat {
  

template<std::uint64_t MCAP = 2147483648>
class continuous_string {
public:

  using value_type = char;
  using size_type = std::uint64_t;
  using difference_type = std::ptrdiff_t;
  using pointer = char*;
  using const_pointer = char const*;
  using reference = char&;
  using const_reference = char const&;
  using iterator = char*;
  using const_iterator = char const*;
    
  static constexpr size_type npos = size_type(-1);
  
  
  continuous_string() noexcept { reserve(); }  
  ~continuous_string() { dispose(); }
  
  
  continuous_string(continuous_string const& other) noexcept {
    if(!reserve() || !commit(other.size()))
      return;
    std::memcpy(data_, other.data_, other.size_);
    size_ = other.size_;
  }
  
  
  continuous_string& operator = (continuous_string const& other) noexcept {
    if(!commit(other.size_))
      return *this;
    std::memcpy(data_, other.data_, other.size_);
    size_ = other.size_;
    return *this;
  }
  
  
  continuous_string(continuous_string&& other) noexcept:
    reserved_capacity_{other.reserved_capacity_}, committed_capacity_{other.committed_capacity_},
    size_{other.size_}, data_{other.data_} {    
    other.reserved_capacity_ = 0;
    other.committed_capacity_ = 0;
    other.size_ = 0;
    other.data_ = nullptr;
  }
  
  
  continuous_string& operator = (continuous_string&& other) noexcept {
    dispose();
    reserved_capacity_ = other.reserved_capacity_; other.reserved_capacity_ = 0;
    committed_capacity_ = other.committed_capacity_; other.committed_capacity_ = 0;
    size_ = other.size_; other.size_ = 0;
    data_ = other.data_; other.data_ = nullptr;
    return *this;
  }
  
  
  template<std::size_t M>
  explicit continuous_string(char const (&data)[M]) noexcept {
    reserve();
    assign(data, data + M);
  }
  
  
  template<std::size_t M>
  explicit continuous_string(wchar_t const (&data)[M]) noexcept {
    reserve();
    assign(data, data + M);
  }


  explicit continuous_string(char const* data) noexcept {
    reserve();
    assign(data);
  }


  explicit continuous_string(wchar_t const* data) noexcept {
    reserve();
    assign(data);
  }


  continuous_string(char const* b, char const* e) noexcept {
    reserve();
    assign(b, e);
  }


  continuous_string(wchar_t const* b, wchar_t const* e) noexcept {
    reserve();
    assign(b, e);
  }


  explicit continuous_string(std::string const& rhs) noexcept {
    reserve();
    assign(rhs.begin(), rhs.end());
  }


  explicit continuous_string(std::string_view const& rhs) noexcept {
    reserve();
    assign(rhs.begin(), rhs.end());
  }
  
  continuous_string& operator = (char const* data) noexcept {
    return assign(data);
  }

  continuous_string& operator = (std::string const& rhs) noexcept {
    return assign(rhs.begin(), rhs.end());
  }

  continuous_string& operator = (std::string_view const& rhs) noexcept {
    return assign(rhs.begin(), rhs.end());
  }
  
  
  char* begin() noexcept { return data_; }
  char* end() noexcept { return data_ + size_; }
  char const* begin() const noexcept { return data_; }
  char const* end() const noexcept { return data_ + size_; }
  char const* cbegin() const noexcept { return data_; }
  char const* cend() const noexcept { return data_ + size_; }

  size_type size() const noexcept { return size_; }
  size_type length() const noexcept { return size_; }
  size_type capacity() const noexcept { return reserved_capacity_ - 1; }
  size_type max_size() const noexcept { return reserved_capacity_ - 1; }
  bool empty() const noexcept { return size_ == 0; }
  void clear() noexcept { size_ = 0; data_[0] = '\0'; }
  
  
  bool reserve(size_type new_capacity) noexcept {
    
    ++new_capacity; // include terminating '\0'
    
    if(new_capacity <= committed_capacity_)
      return true;
    
    if(committed_capacity_ == reserved_capacity_)
      return false;
    
    new_capacity = nearest_power_of_2(new_capacity);
    if(new_capacity > reserved_capacity_)
      return false;
    
    auto committed = commit_pages(&data_[committed_capacity_],
                                new_capacity - committed_capacity_);
    if(committed == nullptr)
      return false;
    
    committed_capacity_ = new_capacity;
    return true;   
  }
  
  
  bool resize(size_type n) noexcept {
    if(n + 1 > committed_capacity_)
      if(!reserve(n))
        return false;
    size_ = n;
    data_[n] = '\0';
    return true;
  }
  
  
  char& operator [] (size_type i) noexcept {
    return data_[i];
  }


  char const& operator [] (size_type i) const noexcept {
    return data_[i];
  }


  char& at (size_type i) {
    if(i >= size_)
      throw std::out_of_range("invalid string position");
    return data_[i];
  }


  char const& at (size_type i) const {
    if(i >= size_)
      throw std::out_of_range("invalid string position");
    return data_[i];
  }

  
  char& back() noexcept { return data_[size_ - 1]; }
  char const& back() const noexcept { return data_[size_ - 1]; }
  char& front() noexcept { return data_[0]; }
  char const& front() const noexcept { return data_[0]; }

  
  continuous_string& operator += (char c) noexcept {
    push_back(c);
    return *this;
  }


  continuous_string& operator += (continuous_string const& rhs) noexcept {
    return append(rhs.begin(), rhs.end());
  }


  continuous_string& operator += (char const* cc) noexcept {
    return append(cc);
  }


  continuous_string& operator += (std::string const& rhs) noexcept {
    return append(rhs.data(), rhs.data() + rhs.size());
  }

  continuous_string& operator += (std::string_view const& rhs) noexcept {
    return append(rhs.data(), rhs.data() + rhs.size());
  }
  

  continuous_string& append(continuous_string const& rhs) noexcept {
    return append(rhs.begin(), rhs.end());
  }

  continuous_string& append(std::string const& rhs) noexcept {
    return append(rhs.data(), rhs.data() + rhs.size());
  }

  continuous_string& append(std::string_view const& rhs) noexcept {
    return append(rhs.data(), rhs.data() + rhs.size());
  }

  continuous_string& append(char const* cc, size_type n) noexcept {
    return append(cc, cc + n);
  }

  continuous_string& append(wchar_t const* cc, size_type n) noexcept {
    return append(cc, cc + n);
  }

  continuous_string& append(char const* cc) noexcept {
    if(cc == nullptr)
      return *this;
    while(*cc != '\0') {
      if(!push_back(*cc))
        return *this;
      ++cc;
    }
    return *this;
  }

  continuous_string& append(wchar_t const* cc) noexcept {
    if(cc == nullptr)
      return *this;
    while(*cc != '\0') {
      if(!push_back(*cc))
        return *this;
      ++cc;
    }
    return *this;
  }

  continuous_string& append(char const* b, char const* e) noexcept {
    if(b == nullptr || e == nullptr || b == e)
      return *this;
    size_type const size_to_append = static_cast<size_type>(e - b);
    if(size_to_append > committed_capacity_ - size_ - 1 
       && !commit(committed_capacity_ + size_to_append))
      return *this;      
    for(char* p = data_ + size_; b != e; ++p, ++b)
      *p = *b;
    size_ += size_to_append;
    data_[size_] = '\0';
    return *this;
  }

  continuous_string& append(wchar_t const* b, wchar_t const* e) noexcept {
    if(b == nullptr || e == nullptr || b == e)
      return *this;
    size_type const size_to_append = static_cast<size_type>(e - b);
    if(size_to_append > committed_capacity_ - size_ - 1 
       && !commit(committed_capacity_ + size_to_append))
      return *this;      
    for(char* p = data_ + size_; b != e; ++p, ++b)
      // TODO: utf-8 conversion
      *p = *b;
    size_ += size_to_append;
    data_[size_] = '\0';
    return *this;
  }

  bool push_back(char c) noexcept {
    if(size_ + 1 == committed_capacity_ && !commit(committed_capacity_ + 1))
      return false;
    data_[size_] = c;
    ++size_;
    return true;
  }
  
  
  bool push_back(wchar_t c) noexcept {
    if(size_ + 1 == committed_capacity_ && !commit(committed_capacity_ + 1))
      return false;
    // TODO: utf-8 conversion
    data_[size_] = c;    
    ++size_;
    return true;
  }
  
  
  void pop_back() noexcept {
    if(size_ == 0) return;
    data_[--size_] = '\0';
  }


  continuous_string& assign(continuous_string const& rhs) noexcept {
    return assign(rhs.data(), rhs.data() + rhs.size());
  }


  continuous_string& assign(std::string const& rhs) noexcept {
    return assign(rhs.data(), rhs.data() + rhs.size());
  }


  continuous_string& assign(std::string_view const& rhs) noexcept {
    return assign(rhs.data(), rhs.data() + rhs.size());
  }


  continuous_string& assign(char const* data) noexcept {
    size_ = 0; data_[0] = '\0';
    return append(data);
  }


  continuous_string& assign(wchar_t const* data) noexcept {
    size_ = 0; data_[0] = '\0';
    return append(data);
  }


  continuous_string& assign(char const* b, char const* e) noexcept {
    size_ = 0; data_[0] = '\0';
    return append(b, e);
  }


  continuous_string& assign(wchar_t const* b, wchar_t const* e) noexcept {
    size_ = 0; data_[0] = '\0';
    return append(b, e);
  }
  
  
  char const* c_str() const noexcept { return data_; }
  char const* data() const noexcept { return data_; }

  size_type find_first_of(char c) const noexcept {
    return find_first_of(c, 0);
  }


  size_type find_first_of(char c, size_type i) const noexcept {
    if(i >= size_)
      return npos;
    for(char const *p = data_ + i, *e = data_ + size_; p != e; ++p)
      if(*p == c)
        return static_cast<size_type>(p - p_);
    return npos;
  }


  size_type find_last_of(char c) const noexcept {
    return find_last_of(c, size_ - 1);
  }


  size_type find_last_of(char c, size_type i) const noexcept {
    for(char const* p = data_ + i; p != data_ - 1; --p)
      if(*p == c)
        return static_cast<size_type>(p - data_);
    return npos;
  }
  
  
  std::string_view substr(size_type pos, size_type n) const noexcept {
    return std::string_view{p_ + pos, n};
  }
  
    
  int compare(continuous_string const& rhs) const noexcept {
    return compare(rhs.begin(), rhs.end());
  }


  int compare(std::string const& rhs) const noexcept {
    return compare(rhs.begin(), rhs.end());
  }


  int compare(std::string_view const& rhs) const noexcept {
    return compare(rhs.begin(), rhs.end());
  }


  int compare(char const* b, char const* e) const noexcept {
    if(b == e)
      return size_ == 0 ? 0 : 1;
    char const* c = data_;
    while(*c == *b && b != e) {
      ++c; ++b;
    }
    if(b == e) return 0;
    else if(*c > *b) return 1;
    else return -1;
  }

  int compare(char const* s) const noexcept {
    if(s == nullptr)
      return size_ == 0 ? 0 : 1;
    char const* c = data_;
    while(*c == *s && *s != '\0') {
      ++c; ++s;
    }
    if(*c == *s) return 0;
    else if(*c > *s) return 1;
    else return -1;
  }
  

private:

  size_type reserved_capacity_{0};
  size_type committed_capacity_{0};
  size_type size_{0};
  char* data_{nullptr};
  
  
  static std::uint64_t nearest_power_of_2(std::uint64_t n) noexcept {
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
  
  
  static size_type get_page_size() noexcept {
#if defined(_WIN32)    
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;    
#endif 
  }
  
  
  static char* reserve_pages(size_type capacity) noexcept {
#if defined(_WIN32)
    return static_cast<char*>(VirtualAlloc(nullptr, capacity, MEM_RESERVE, PAGE_NOACCESS));
#endif
  }
  
  
  static char* commit_pages(char* address, size_type size) noexcept {
#if defined(_WIN32)
  return static_cast<char*>(VirtualAlloc(address, size, MEM_COMMIT, PAGE_READWRITE));
#endif    
  }
  
  
  static void release_pages(char* address, size_type size) noexcept {
#if defined(_WIN32)
    VirtualFree(address, size, MEM_RELEASE);
#endif    
  }
  
  
  void dispose() noexcept {
    if(data_ == nullptr)
      return;
    release_pages(data_, reserved_capacity_);
    data_ = nullptr;
    reserved_capacity_ = 0;
    committed_capacity_ = 0;
    size_ = 0;
  }
  
  
  bool reserve() noexcept {
    auto const page_size = get_page_size();
    auto pages_count = MCAP / page_size;
    if(MCAP % page_size != 0 || pages_count == 0)
      ++pages_count;
    auto const reserved_capacity = nearest_power_of_2(pages_count * page_size);     
    auto const reserved = reserve_pages(reserved_capacity);
    if(reserved == nullptr)
      return false;
    auto const committed = commit_pages(reserved, page_size);
    if(committed == nullptr)
      return false;
    reserved_capacity_ = reserved_capacity;
    committed_capacity_ = page_size;
    data_ = committed;
    data_[0] = '\0';
    return true;
  }
  
  
  bool commit(size_type new_capacity) noexcept {
    
    if(new_capacity <= committed_capacity_)
      return true;
    
    if(committed_capacity_ == reserved_capacity_)
      return false;
    
    new_capacity = nearest_power_of_2(new_capacity);
    if(new_capacity > reserved_capacity_)
      return false;
    
    auto committed = commit_pages(&data_[size_], new_capacity - committed_capacity_);
    if(committed == nullptr)
      return false;
    
    committed_capacity_ = new_capacity;
    return true;   
  }

};


template<std::size_t N>
std::string& operator += (std::string const& x, continuous_string<N> const& y) {
  return x.append(y.data(), y.size());
}


template<std::size_t N>
bool operator == (continuous_string<N> const& x, continuous_string<N> const& y) {
  return x.compare(y) == 0;
}


template<std::size_t N>
bool operator == (continuous_string<N> const& x, char const* y) {
  return x.compare(y) == 0;
}


template<std::size_t N>
bool operator == (char const* x, continuous_string<N> const& y) {
  return y.compare(x) == 0;
}


template<std::size_t N>
bool operator == (continuous_string<N> const& x, std::string const& y) {
  return x.compare(y) == 0;
}


template<std::size_t N>
bool operator == (std::string const& x, continuous_string<N> const& y) {
  return y.compare(x) == 0;
}


template<std::size_t N>
bool operator == (continuous_string<N> const& x, std::string_view const& y) {
  return x.compare(y) == 0;
}


template<std::size_t N>
bool operator == (std::string_view const& x, continuous_string<N> const& y) {
  return y.compare(x) == 0;
}


template<std::size_t N>
bool operator != (continuous_string<N> const& x, continuous_string<N> const& y) {
  return x.compare(y) != 0;
}


template<std::size_t N>
bool operator != (continuous_string<N> const& x, char const* y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator != (char const* x, continuous_string<N> const& y) {
  return y.compare(x) != 0;
}

template<std::size_t N>
bool operator != (continuous_string<N> const& x, std::string const& y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator != (std::string const& x, continuous_string<N> const& y) {
  return y.compare(x) != 0;
}

template<std::size_t N>
bool operator != (continuous_string<N> const& x, std::string_view const& y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator != (std::string_view const& x, continuous_string<N> const& y) {
  return y.compare(x) != 0;
}

template<std::size_t N>
bool operator < (continuous_string<N> const& x, continuous_string<N> const& y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator < (continuous_string<N> const& x, char const* y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator < (char const* x, continuous_string<N> const& y) {
  return y.compare(x) != 0;
}

template<std::size_t N>
bool operator < (continuous_string<N> const& x, std::string const& y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator < (std::string const& x, continuous_string<N> const& y) {
  return y.compare(x) != 0;
}

template<std::size_t N>
bool operator < (continuous_string<N> const& x, std::string_view const& y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator < (std::string_view const& x, continuous_string<N> const& y) {
  return y.compare(x) != 0;
}

template<std::size_t N>
bool operator <= (continuous_string<N> const& x, continuous_string<N> const& y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator <= (continuous_string<N> const& x, char const* y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator <= (char const* x, continuous_string<N> const& y) {
  return y.compare(x) != 0;
}

template<std::size_t N>
bool operator <= (continuous_string<N> const& x, std::string const& y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator <= (std::string const& x, continuous_string<N> const& y) {
  return y.compare(x) != 0;
}

template<std::size_t N>
bool operator <= (continuous_string<N> const& x, std::string_view const& y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator <= (std::string_view const& x, continuous_string<N> const& y) {
  return y.compare(x) != 0;
}

template<std::size_t N>
bool operator > (continuous_string<N> const& x, continuous_string<N> const& y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator > (continuous_string<N> const& x, char const* y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator > (char const* x, continuous_string<N> const& y) {
  return y.compare(x) != 0;
}

template<std::size_t N>
bool operator > (continuous_string<N> const& x, std::string const& y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator > (std::string const& x, continuous_string<N> const& y) {
  return y.compare(x) != 0;
}

template<std::size_t N>
bool operator > (continuous_string<N> const& x, std::string_view const& y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator > (std::string_view const& x, continuous_string<N> const& y) {
  return y.compare(x) != 0;
}

template<std::size_t N>
bool operator >= (continuous_string<N> const& x, continuous_string<N> const& y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator >= (continuous_string<N> const& x, char const* y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator >= (char const* x, continuous_string<N> const& y) {
  return y.compare(x) != 0;
}

template<std::size_t N>
bool operator >= (continuous_string<N> const& x, std::string const& y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator >= (std::string const& x, continuous_string<N> const& y) {
  return y.compare(x) != 0;
}

template<std::size_t N>
bool operator >= (continuous_string<N> const& x, std::string_view const& y) {
  return x.compare(y) != 0;
}

template<std::size_t N>
bool operator >= (std::string_view const& x, continuous_string<N> const& y) {
  return y.compare(x) != 0;
}


template<typename OS, std::size_t N>
OS& operator << (OS& stream, continuous_string<N> const& fs) {
  stream << std::string_view{ fs.data(), fs.size() };
  return stream;
}



}
