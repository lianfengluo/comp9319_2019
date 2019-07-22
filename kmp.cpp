#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#define CHECK_TABLE

// method 1
// std::unique_ptr<int32_t[]> buildFailureTable(const char pattern[], const int len)
// {
//     auto failure = std::make_unique<int32_t[]>(len + 1);
//     int i = 1, j = 0;
//     failure[0] = -1;
//     while (i < len) {
//         if (pattern[i] == pattern[j]) {
//             failure[i] = failure[j];
//         }
//         else {
//             failure[i] = j;
//             j = failure[j];
//             while (j != -1 and pattern[i] != pattern[j]) {
//                 j = failure[j];
//             }
//         }
//         ++i; ++j;
//     }
//     failure[i] = j;
//     return failure;
// }

// int countMatch(const char T[], const char pattern[], const int &t_len, const int &p_len,
//     const std::unique_ptr<int32_t[]>&failure)
// {
//     int i = 0, j = 0, count = 0;
//     while (i != t_len) {
//         if (T[i] == pattern[j]) {
//             ++i; ++j;
//             if (j == p_len) {
//                 ++count;
//                 j = failure[j];
//             }
//         } else {
//             j = failure[j];
//             if (j == -1) {
//                 ++j; ++i;
//             }
//         }
//     }
//     return count;
// }

// method 2
std::unique_ptr<int32_t[]> buildFailureTable(const char pattern[], const int len)
{
    auto failure = std::make_unique<int32_t[]>(len);
    for (int i = 0; i < len; ++i)
        failure[i] = 0;
    int i = 1, j = 0;
    while (i < len)
    {
        if (pattern[i] == pattern[j])
        {
            failure[i++] = ++j;
        }
        else if (j > 0)
        {
            j = failure[j - 1];
        }
        else
        {
            ++i;
        }
    }
    return failure;
}

int countMatch(const char T[], const char pattern[], const int &t_len, const int &p_len,
               const std::unique_ptr<int32_t[]> &failure)
{
    int i = 0, j = 0, count = 0;
    int p_len_minus1 = p_len - 1;
    while (i != t_len)
    {
        if (T[i] == pattern[j])
        {
            if (j == p_len_minus1)
            {
                ++count;
                j = failure[j];
            }
            else
            {
                ++j;
            }
            ++i;
        }
        else if (j > 0)
        {
            j = failure[j - 1];
        }
        else
        {
            ++i;
        }
    }
    return count;
}

int main(int argv, char *argc[])
{
    if (argv != 3)
        exit(1);
    int p_len = strlen(argc[1]);
    int t_len = strlen(argc[2]);
    auto failure = buildFailureTable(argc[1], p_len);
#ifdef CHECK_TABLE
    std::cout << argc[1] << std::endl;
    for (int i = 0; i < p_len; ++i)
    {
        std::cout << failure[i] << ' ';
    }
    std::cout << std::endl;
#endif
    std::cout << countMatch(argc[2], argc[1], t_len, p_len, failure) << std::endl;
    return 0;
}