#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>

const size_t MAX_ENTRIES = 16384;

int main(int argc, char *argv[])
{
  bool listing = false;
  if (argc == 2)
  {
    if (std::string(argv[1]) == "-l")
    {
      listing = true;
    }
    else
    {
      std::cerr << "Error flag!" << std::endl;
      exit(1);
    }
  }
  std::string input;
  getline(std::cin, input);
  std::unordered_map<std::string, int32_t> entries;
  entries.reserve(MAX_ENTRIES);
  for (int i = 0; i != 256; ++i)
    entries.emplace(std::string{static_cast<int8_t>(i)}, i);
  int32_t cur = 256;
  std::string pre, combine, out_s;
  if (!listing)
  {
    for (size_t i = 0;; ++i)
    {
      if (i != input.size())
      {
        combine = pre + input[i];
        if (entries.find(combine) != entries.end())
        {
          // in the entries
          pre = combine;
          continue;
        }
        if (pre.size() == 1)
        {
          // output visible char
          std::cout << pre;
        }
        else
        {
          // string in dictionary
          std::cout << entries.find(pre)->second;
        }
        std::cout << ' ';
      }
      else
      {
        if (pre.size() == 1)
        {
          // output visible char
          std::cout << pre;
        }
        else
        {
          // string in dictionary
          std::cout << entries.find(pre)->second;
        }
        break;
      }
      pre = std::string{combine.back()};
      if (cur != MAX_ENTRIES)
      {
        entries.emplace(combine, cur++);
      }
    }
    std::cout << std::endl;
  }
  else
  {
    for (size_t i = 0;; ++i)
    {
      if (i != input.size())
      {
        combine = pre + input[i];
        if (entries.find(combine) != entries.end())
        {
          // in the entries
          if (pre.size() == 0)
          {
            std::cout << "NIL " << input[i];
          }
          else
          {
            std::cout << pre << ' ' << input[i];
          }
          std::cout << '\n';
          pre = combine;
          continue;
        }
        if (entries.find(pre)->second > 255)
        {
          out_s = std::to_string(entries.find(pre)->second);
        }
        else
        {
          out_s = std::string{static_cast<int8_t>(pre[0])};
        }
      }
      else
      {
        if (entries.find(pre)->second > 255)
        {
          out_s = std::to_string(entries.find(pre)->second);
        }
        else
        {
          out_s = std::string{static_cast<int8_t>(pre[0])};
        }
        break;
      }
      std::cout << pre << ' ' << input[i] << ' ' << out_s;
      pre = std::string{combine.back()};
      if (cur != MAX_ENTRIES)
      {
        entries.emplace(combine, cur);
        std::cout << ' ' << cur << ' ' << combine;
        ++cur;
      }
      std::cout << '\n';
    }
    std::cout << pre << " EOF " << out_s << std::endl;
  }
  return 0;
}
