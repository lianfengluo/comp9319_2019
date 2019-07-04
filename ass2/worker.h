#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>

const size_t MAX_LEN_SEARCH = 520;

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
  // void BuildbbIndex();
  // void BuildsIndex();
  // void BuildbIndex();

 private:
  std::string filepath_;
  std::string filename_;
  int s_f_ = 0;
  int b_f_ = 0;
  int bb_f_ = 0;
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
  std::unique_ptr<int32_t[]> select_b_table_{nullptr};
  std::unique_ptr<int32_t[]> select_bb_table_{nullptr};
  void Build_S_B_Index_LG(int s_f_, int b_f_);
  void Build_S_B_Index_SM(int s_f_, int b_f_);
  void Build_BB_Index_LG(std::string& bb_i_f_name);
  void Build_BB_Index_SM();
  void Search_Sm();
  void Search_Lg();
  void Sum_C_Table();
  void get_lower_uppder_bound(int& lower_bound, int& upper_bound, int c);
  int search_m_sm();
  // int Select_Sm(int index, const std::unique_ptr<int[]>& select_table,
  //               const std::unique_ptr<char[]>& f_buff);
  // bool large_threshold;
};