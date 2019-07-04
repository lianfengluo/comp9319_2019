#include <array>
#include <cstdint>
#include <iostream>
#include <string>

const size_t MAX_ENTRIES = 16384;

const std::string GetSubStr(const std::string &input, const size_t &i,
                            size_t &len)
{
  while (i + len < input.size())
  {
    if (input[i + len] == ' ')
      break;
    ++len;
  }
  return input.substr(i, len);
}

int32_t main(int32_t argc, char *argv[])
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
  std::array<std::string, MAX_ENTRIES> entries;
  for (int32_t i = 0; i != 256; ++i)
    entries[i] = std::string{static_cast<int8_t>(i)};
  std::string pre{"NIL"}, income_str;
  int32_t cur{256}, income_index{0}, pre_index{0};
  std::string input;
  getline(std::cin, input);
  size_t len{0}; // substring length
  if (!listing)
  {
    // Not listing the step
    for (size_t i = 0; i < input.size();)
    {
      income_str = GetSubStr(input, i, len);
      if (len == 0)
      {
        // income string is a space
        income_str = " ";
        ++len;
      }
      i += len + 1;
      len = 0;
      if (income_str.size() == 1)
      {
        income_index = static_cast<uint8_t>(income_str[0]);
      }
      else
      {
        income_index = std::stoi(income_str);
      }
      if (i != 2)
      {
        if (pre.size() == 1)
        {
          pre_index = static_cast<uint8_t>(pre[0]);
        }
        else
        {
          pre_index = std::stoi(pre);
        }
      }
      if (income_index == cur)
      {
        // if the income_index is undefined
        if (cur != MAX_ENTRIES)
        {
          entries[cur].reserve(entries[pre_index].size() + 1);
          entries[cur] = entries[pre_index] + entries[pre_index][0];
          std::cout << entries[income_index];
          ++cur;
        }
        else
        {
          std::cout << entries[pre_index] + entries[pre_index][0];
        }
        pre = income_str;
        continue;
      }
      else
      {
        std::cout << entries[income_index];
      }
      if (i != 2)
      {
        if (cur != MAX_ENTRIES)
        {
          entries[cur].reserve(entries[pre_index].size() + 1);
          entries[cur] = entries[pre_index] + entries[income_index][0];
          ++cur;
        }
      }
      pre = income_str;
    }
    std::cout << std::endl;
  }
  else
  {
    // listing all the steps
    for (size_t i = 0; i < input.size();)
    {
      income_str = GetSubStr(input, i, len);
      if (len == 0)
      {
        income_str = " ";
        len = 1;
      }
      i += len + 1;
      len = 0;
      if (income_str.size() == 1)
      {
        income_index = static_cast<uint8_t>(income_str[0]);
      }
      else
      {
        income_index = std::stoi(income_str);
      }
      if (i != 2)
      {
        if (pre.size() == 1)
        {
          pre_index = static_cast<uint8_t>(pre[0]);
        }
        else
        {
          pre_index = std::stoi(pre);
        }
      }
      if (income_index == cur)
      {
        // if the income_index is undefined
        if (cur != MAX_ENTRIES)
        {
          entries[cur].reserve(entries[pre_index].size() + 1);
          entries[cur] = entries[pre_index] + entries[pre_index][0];
          std::cout << pre << ' ' << income_str << ' ' << entries[income_index]
                    << ' ' << cur << ' ' << entries[cur] << '\n';
          ++cur;
        }
        else
        {
          std::cout << pre << ' ' << income_str << ' ' << entries[income_index]
                    << ' ' << cur << ' '
                    << (entries[pre_index] + entries[pre_index][0]) << '\n';
        }
        pre = income_str;
        continue;
      }
      else
      {
        std::cout << pre << ' ' << income_str << ' ' << entries[income_index];
      }
      if (i != 2 && cur != MAX_ENTRIES)
      {
        std::cout << ' ' << cur << ' ';
        entries[cur].reserve(entries[pre_index].size() + 1);
        entries[cur] = entries[pre_index] + entries[income_index][0];
        std::cout << entries[cur];
        ++cur;
      }
      std::cout << '\n';
      pre = income_str;
    }
  }
  return 0;
}
