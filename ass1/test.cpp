#include <cstdint>
#include <iostream>
#include <iterator>
#include <string>
int main() {
  // std::string s;
  // while ((std::cin >> s))
  //    std::cout << s << std::endl;
  // char a = 'a';
  // std::cout << static_cast<int32_t>(a) << std::endl;
  std::string s((std::istreambuf_iterator<char>(std::cin.rdbuf())),
                std::istreambuf_iterator<char>());
  std::cout << s;
  return 0;
}
