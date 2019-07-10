#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

#define DEBUG_
// 1MB
const int READ_BUFF_SIZE = 1024 * 1024;
// 10K
const int MIN_READ_BUFF = 20 * 1024;
// 1MB
const int WRITE_BUFF_SIZE = 1024 * 1024;
const int WRITE_BUFF_CHUNK = WRITE_BUFF_SIZE / sizeof(int32_t);
// 100KB
const int BUILD_SIZE = 100 * 1024;
const int NUMBER_OF_CHAR = 127;
// NUMBER OF CHAR * sizeof(int32_t)
const int CHUNK_SIZE = NUMBER_OF_CHAR * sizeof(int32_t);
// 9.5MB
const int START_UP_SIZE = 9.5 * 1024 * 1024;
const int TOTAL_SPACE = 16 * 1024 * 1024;
// const int TOTAL_SPACE = 20 * 1024 * 1024;
const int WRITE_NUM_OF_CHUNK = 2000;

// 6.5MB
const int MAX_FREE_MEMORY = TOTAL_SPACE - START_UP_SIZE;
const int MAX_FREE_BITS = MAX_FREE_MEMORY * 8;
const int MAX_SEARCH_PATTERN_LEN = 5000;
const int MAX_RESULT_NUM = 5000;
const int FIRST_BIT = (1 << 7);
const size_t MAX_LEN_SEARCH = 520;

// in work_sm.cpp
int Rank_Sm_Md_Function(const std::unique_ptr<char[]>& buff,
                        const std::unique_ptr<int32_t[]>& occ, int index,
                        int step_record = 4);

int Occ_Function_Sm_Md(int c, int index_s,
                       const std::unique_ptr<int32_t[]>& occ,
                       const std::unique_ptr<char[]>& buff,
                       const std::unique_ptr<int32_t[]>& map_table,
                       int num_of_char, int step_size);

int binary_search_char(int index, int num_of_char,
                       const std::unique_ptr<int32_t[]>& c_table,
                       const std::unique_ptr<int32_t[]>& rev_map);

int Select_Sm_Md(int index, const std::unique_ptr<int[]>& select_table,
                 const std::unique_ptr<char[]>& f_buff, int interval = 4);

int find_pre_1(int index, const std::unique_ptr<char[]>& buff);

class RLEBWT {
 public:
  RLEBWT(char* argv[]);
  bool Existsbb();
  void Build_S_B_Index();
  void Build_BB_Index();
  bool ExistsIndex();
  void Build_bb();
  void Createbb();
  void Search();
  ~RLEBWT();

 private:
  std::string filepath_;
  std::string filename_;
  bool load_index_ = false;
  int s_f_ = 0;
  int b_f_ = 0;
  int bb_f_ = 0;
  int s_i_f_ = 0;
  int b_i_f_ = 0;
  int bs_i_f_ = 0;
  int bb_i_f_ = 0;
  size_t s_f_size_ = 0;
  size_t b_f_size_ = 0;
  int num_of_char_ = 0;
  bool large_file_ = false;
  bool medium_file_ = false;
  std::string index_folder_;
  std::unique_ptr<int32_t[]> c_table_{nullptr};
  std::unique_ptr<int32_t[]> c_s_table_{nullptr};
  std::unique_ptr<int32_t[]> mapping_table_{nullptr};
  char search_pattern_[MAX_LEN_SEARCH + 1] = {'\0'};
  int len_of_pattern_ = 0;
  int step_s_size_ = 0;
  char mode;
  std::unique_ptr<char[]> s_f_buff_{nullptr};
  std::unique_ptr<char[]> b_f_buff_{nullptr};
  std::unique_ptr<char[]> bb_f_buff_{nullptr};
  std::unique_ptr<int32_t[]> occ_s_table_{nullptr};
  std::unique_ptr<int32_t[]> occ_b_table_{nullptr};
  std::unique_ptr<int32_t[]> occ_bb_table_{nullptr};
  std::unique_ptr<int32_t[]> select_b_table_{nullptr};
  std::unique_ptr<int32_t[]> select_bb_table_{nullptr};
  void Build_S_B_Index_LG(int s_f_, int b_f_);
  void Build_S_B_Index_SM(int s_f_, int b_f_);
  void Build_BB_Index_LG(std::string& bb_i_f_name);
  void Build_BB_Index_SM();
  void Search_Sm();
  void Search_Medium();
  void Search_Lg();
  void Sum_C_Table();
  void get_lower_uppder_bound_sm(int& lower_bound, int& upper_bound, int c);
  void get_lower_uppder_bound_md(int& lower_bound, int& upper_bound, int c);
  void get_lower_uppder_bound_lg(int& lower_bound, int& upper_bound, int c);
  int search_m_sm();
  int search_r_sm();
  int search_a_sm(std::unique_ptr<size_t[]>& results);
  int search_n_sm(std::unique_ptr<char[]>& result);
  int search_m_md();
  int search_r_md();
  int search_a_md(std::unique_ptr<size_t[]>& results);
  int search_n_md(std::unique_ptr<char[]>& result);
  int search_m_lg();
  int search_r_lg();
  int search_a_lg(std::unique_ptr<size_t[]>& results);
  int search_n_lg(std::unique_ptr<char[]>& result);
  int binary_search_s_sm(int pos_c, int c);
  int binary_search_lg(int pos_c, int c);
  int interval_b_ = 0, interval_bb_ = 0;
  bool load_s_ = false, load_b_ = false, load_s_b_ = false, load_r_b_ = false,
       load_bb_ = false, load_s_bb_ = false, load_r_s_ = false;
};