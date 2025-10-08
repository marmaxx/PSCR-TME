#include "PrString.h"

namespace pr {

  // TODO: Implement constructor e.g. using initialization list
  String::String (const char *s) {
    if (s) {
        data = newcopy(s);
    } else {
        data = nullptr;
    }
    std::cout << "String constructor called for: " << s << std::endl;
  }

  String::~String () {
    std::cout << "String destructor called for: " << (data ? data : "(null)")
        << std::endl;
    delete[] data;
  }

  // TODO : add other operators and functions

  std::ostream& operator<<(std::ostream& os, const String& str) {
    os << (str.data ? str.data : "(null)");
    return os;
  }

  String::String(const String& other) {
    data = new char[length(other.data) + 1];  
    data = newcopy(other.data);
    std::cout << "String copy constructor called for: " << data << std::endl;
  }

  bool operator==(const String& a, const String& b){
    return compare(a.data, b.data) == 0;
  }
  
  char* newcat (const char *a, const char *b) {
    size_t len_a = length(a);
    size_t len_b = length(b);
    char* cat = new char[len_a + len_b + 1];
    for (size_t i = 0; i < len_a; ++i) {
        cat[i] = a[i];
    }
    for (size_t j = 0; j <= len_b; ++j) {
        cat[len_a + j] = b[j];
    }
    return cat;
  }

  String operator+(const String& a, const String& b) {
    char* cat = newcat(a.data, b.data);
    String result(cat);
    delete[] cat;
    return result;
  }

  String::String(String&& other) noexcept {
    data = other.data;
    other.data = nullptr;
    std::cout << "String move constructor called for: " << data << std::endl;
  }

  String& String::operator=(const String& other) {
    if (this != &other) {
        delete[] data;  
        data = newcopy(other.data); 
        std::cout << "String copy assignment operator called for: " << data << std::endl;
    }
    return *this;
  }

  bool String::operator<(const String& other) const {
    return compare(data, other.data) < 0;
  }

  String& String::operator=(String&& other) noexcept {
    if (this != &other) {
        delete[] data;  
        data = other.data; 
        other.data = nullptr;
        std::cout << "String move assignment operator called for: " << data << std::endl;
    }
    return *this;
  }

}// namespace pr

