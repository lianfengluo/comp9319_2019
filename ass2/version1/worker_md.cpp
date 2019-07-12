#include "worker.h"

int find_pre_1(int index, const std::unique_ptr<char[]>& buff) {
  int pos = index / 8;
  int bit_pos = index % 8;
  int c = 0;
  while (true) {
    c = buff[pos];
    for (int i = bit_pos; i != -1; --i) {
      if ((c << i) & FIRST_BIT) {
        return index;
      }
      --index;
    }
    --pos;
    bit_pos = 7;
  }
  return index;
}

static int binary_select_bb(int pre_1_pos, int start, int end,
                            const std::unique_ptr<int32_t[]>& select_table,
                            const std::unique_ptr<char[]>& buff, int interval) {
  int mid = 0, select_index = 0;
  // pre_1_pos is index of c_table
  while (true) {
    if (start > end) break;
    mid = (start + end) / 2;
    select_index = Select_Sm_Md(mid, select_table, buff, interval);
    if (select_index == pre_1_pos) {
      return mid;
    } else if (select_index > pre_1_pos) {
      end = mid - 1;
    } else {
      start = mid + 1;
    }
  }
  return -1;
}

int RLEBWT::search_n_md(std::unique_ptr<char[]>& result) {
  ++len_of_pattern_;
  search_pattern_[len_of_pattern_ - 1] = ']';
  int c = ']', curr_index = 0, count = 0,
      max_index = c_table_[NUMBER_OF_CHAR - 1];
  auto reverse_map = std::make_unique<int32_t[]>(num_of_char_);
  for (int i = 0; i != NUMBER_OF_CHAR; ++i) {
    if (mapping_table_[i] != -1) {
      reverse_map[mapping_table_[i]] = i;
    }
  }
  int occurrence_1 = 0, occurrence_2 = 0;
  int search_index = len_of_pattern_ - 1, padding_0_1 = 0, padding_0_2 = 0,
      pre_lower = 0, pre_upper = 0, upper_index = 0, lower_index = 0,
      lower_bound = c_table_[']' - 1], upper_bound = c_table_[']'] - 1;
  while (search_index > -1) {
    if (--search_index == -1) {
      c = '[';
    } else {
      c = search_pattern_[search_index];
    }
    pre_lower = lower_bound;
    pre_upper = upper_bound;
    lower_index =
        Rank_Sm_Md_Function(b_f_buff_, occ_b_table_, lower_bound + 1, 8);
    upper_index =
        Rank_Sm_Md_Function(b_f_buff_, occ_b_table_, upper_bound + 1, 8);
    occurrence_1 =
        Occ_Function_Sm_Md(c, lower_index - 1, occ_s_table_, s_f_buff_,
                           mapping_table_, num_of_char_, step_s_size_);
    occurrence_2 =
        Occ_Function_Sm_Md(c, upper_index, occ_s_table_, s_f_buff_,
                           mapping_table_, num_of_char_, step_s_size_);
    lower_bound = c_s_table_[c - 1] + occurrence_1 + 1;
    upper_bound = c_s_table_[c - 1] + occurrence_2;
    if (lower_bound > upper_bound) {
      return -1;
    }
    padding_0_1 = (s_f_buff_[lower_index - 1] == c)
                      ? pre_lower - Select_Sm_Md(lower_index, select_b_table_,
                                                 b_f_buff_, interval_b_)
                      : 0;
    padding_0_2 = (s_f_buff_[upper_index - 1] == c)
                      ? pre_upper - Select_Sm_Md(upper_index, select_b_table_,
                                                 b_f_buff_, interval_b_)
                      : 0;
    if (s_f_buff_[lower_index - 1] != c) {
      lower_bound =
          Select_Sm_Md(lower_bound, select_bb_table_, bb_f_buff_, interval_bb_);
    } else {
      lower_bound = Select_Sm_Md(lower_bound, select_bb_table_, bb_f_buff_,
                                 interval_bb_) +
                    padding_0_1;
    }
    if (s_f_buff_[upper_index - 1] != c) {
      if (upper_bound == static_cast<int>(s_f_size_)) {
        upper_bound = max_index - 1;
      } else {
        upper_bound = Select_Sm_Md(upper_bound + 1, select_bb_table_,
                                   bb_f_buff_, interval_bb_) -
                      1;
      }
    } else {
      upper_bound = Select_Sm_Md(upper_bound, select_bb_table_, bb_f_buff_,
                                 interval_bb_) +
                    padding_0_2;
    }
    if (c == '[') {
      curr_index = lower_bound;
      // std::cout << static_cast<char>(c) << '\n';
      break;
    }
  }
  // start forward search
  int pre_i_th_1 = 0, pos_c = 0, pos_s_1 = 0, record = false;
  int pre_1_pos = 0;
  while (true) {
    // for no select table
    pre_1_pos = find_pre_1(curr_index, bb_f_buff_);
    pre_i_th_1 =
        binary_select_bb(pre_1_pos, c_s_table_[c - 1] + 1, c_s_table_[c],
                         select_bb_table_, bb_f_buff_, interval_bb_);
    if (pre_i_th_1 == -1) return -1;
    padding_0_1 = curr_index - Select_Sm_Md(pre_i_th_1, select_bb_table_,
                                            bb_f_buff_, interval_bb_);
    pos_c = pre_i_th_1 - c_s_table_[c - 1];
    pos_s_1 = binary_search_s_sm(pos_c, c);
    if (pos_s_1 == -1) return -1;
    curr_index =
        Select_Sm_Md(pos_s_1, select_b_table_, b_f_buff_, interval_b_) +
        padding_0_1;
    c = binary_search_char(curr_index, num_of_char_, c_table_, reverse_map);
    if (c == ']') {
      record = true;
    } else if (c == '[') {
      break;
    } else if (record) {
      result[count] = c;
      ++count;
    }
  }
  return count;
}

void RLEBWT::get_lower_uppder_bound_md(int& lower_bound, int& upper_bound,
                                       int c) {
  int occurrence_1 = 0, occurrence_2 = 0;
  int search_index = len_of_pattern_ - 1, padding_0_1 = 0, padding_0_2 = 0,
      pre_lower = 0, pre_upper = 0, upper_index = 0, lower_index = 0,
      max_index = c_table_[NUMBER_OF_CHAR - 1];
  while (search_index > 0) {
    c = search_pattern_[--search_index];
    pre_lower = lower_bound;
    pre_upper = upper_bound;
    lower_index =
        Rank_Sm_Md_Function(b_f_buff_, occ_b_table_, lower_bound + 1, 8);
    upper_index =
        Rank_Sm_Md_Function(b_f_buff_, occ_b_table_, upper_bound + 1, 8);
    occurrence_1 =
        Occ_Function_Sm_Md(c, lower_index - 1, occ_s_table_, s_f_buff_,
                           mapping_table_, num_of_char_, step_s_size_);
    occurrence_2 =
        Occ_Function_Sm_Md(c, upper_index, occ_s_table_, s_f_buff_,
                           mapping_table_, num_of_char_, step_s_size_);
    lower_bound = c_s_table_[c - 1] + occurrence_1 + 1;
    upper_bound = c_s_table_[c - 1] + occurrence_2;
    if (lower_bound > upper_bound) {
      break;
    }
    padding_0_1 = (s_f_buff_[lower_index - 1] == c)
                      ? pre_lower - Select_Sm_Md(lower_index, select_b_table_,
                                                 b_f_buff_, interval_b_)
                      : 0;
    padding_0_2 = (s_f_buff_[upper_index - 1] == c)
                      ? pre_upper - Select_Sm_Md(upper_index, select_b_table_,
                                                 b_f_buff_, interval_b_)
                      : 0;
    if (s_f_buff_[lower_index - 1] != c) {
      lower_bound =
          Select_Sm_Md(lower_bound, select_bb_table_, bb_f_buff_, interval_bb_);
    } else {
      lower_bound = Select_Sm_Md(lower_bound, select_bb_table_, bb_f_buff_,
                                 interval_bb_) +
                    padding_0_1;
    }
    if (s_f_buff_[upper_index - 1] != c) {
      if (upper_bound == static_cast<int>(s_f_size_)) {
        upper_bound = max_index - 1;
      } else {
        upper_bound = Select_Sm_Md(upper_bound + 1, select_bb_table_,
                                   bb_f_buff_, interval_bb_) -
                      1;
      }
    } else {
      upper_bound = Select_Sm_Md(upper_bound, select_bb_table_, bb_f_buff_,
                                 interval_bb_) +
                    padding_0_2;
    }
  }
}

int RLEBWT::search_m_md() {
  int c = search_pattern_[len_of_pattern_ - 1], count = 0;
  int lower_bound = c_table_[c - 1], upper_bound = c_table_[c] - 1;
  get_lower_uppder_bound_md(lower_bound, upper_bound, c);
  if (upper_bound >= lower_bound) count = upper_bound - lower_bound + 1;
  return count;
}

int RLEBWT::search_r_md() {
  int c = search_pattern_[len_of_pattern_ - 1], count = 0;
  int lower_bound = c_table_[c - 1], upper_bound = c_table_[c] - 1;
  get_lower_uppder_bound_md(lower_bound, upper_bound, c);
  if (upper_bound >= lower_bound) {
    int occ = 0, curr_index = 0, padding = 0, cc = 0;
    int max_index = c_table_[NUMBER_OF_CHAR - 1], pre_index = 0, rank_index = 0;
    // std::cout << lower_bound << ' ' << upper_bound << '\n';
    for (int i = lower_bound; i <= upper_bound; ++i) {
      curr_index = i;
      while (true) {
        pre_index = curr_index;
        rank_index =
            Rank_Sm_Md_Function(b_f_buff_, occ_b_table_, curr_index + 1, 8);
        cc = s_f_buff_[rank_index - 1];
        if (cc == ']') {
          ++count;
          break;
        }
        occ = Occ_Function_Sm_Md(cc, rank_index, occ_s_table_, s_f_buff_,
                                 mapping_table_, num_of_char_, step_s_size_);
        curr_index = c_s_table_[cc - 1] + occ;
        padding = (s_f_buff_[rank_index - 1] == cc)
                      ? pre_index - Select_Sm_Md(rank_index, select_b_table_,
                                                 b_f_buff_, interval_b_)
                      : 0;
        if (s_f_buff_[rank_index - 1] != cc) {
          if (curr_index == static_cast<int>(s_f_size_)) {
            curr_index = max_index - 1;
          } else {
            curr_index = Select_Sm_Md(curr_index + 1, select_bb_table_,
                                      bb_f_buff_, interval_bb_) -
                         1;
          }
        } else {
          curr_index = Select_Sm_Md(curr_index, select_bb_table_, bb_f_buff_,
                                    interval_bb_) +
                       padding;
        }
        if (lower_bound <= curr_index && curr_index <= upper_bound) {
          break;
        }
      }
    }
  }
  return count;
}

int RLEBWT::search_a_md(std::unique_ptr<size_t[]>& results) {
  int c = search_pattern_[len_of_pattern_ - 1], count = 0;
  int lower_bound = c_table_[c - 1], upper_bound = c_table_[c] - 1;
  get_lower_uppder_bound_md(lower_bound, upper_bound, c);
  if (upper_bound >= lower_bound) {
    int occ = 0, curr_index = 0, padding = 0, cc = 0;
    int max_index = c_table_[NUMBER_OF_CHAR - 1], pre_index = 0, rank_index = 0;
    for (int i = lower_bound; i <= upper_bound; ++i) {
      size_t result = 0;
      int result_len = 0;
      curr_index = i;
      bool record = false;
      while (true) {
        pre_index = curr_index;
        rank_index =
            Rank_Sm_Md_Function(b_f_buff_, occ_b_table_, curr_index + 1, 8);
        cc = s_f_buff_[rank_index - 1];
        if (cc == '[') {
          results[count] = result;
          ++count;
          break;
        }
        if (record) {
          result += (cc - '0') * static_cast<size_t>(std::powl(10, result_len));
          ++result_len;
        }
        if (cc == ']') {
          record = true;
        }
        occ = Occ_Function_Sm_Md(cc, rank_index, occ_s_table_, s_f_buff_,
                                 mapping_table_, num_of_char_, step_s_size_);
        curr_index = c_s_table_[cc - 1] + occ;
        padding = (s_f_buff_[rank_index - 1] == cc)
                      ? pre_index - Select_Sm_Md(rank_index, select_b_table_,
                                                 b_f_buff_, interval_b_)
                      : 0;
        if (s_f_buff_[rank_index - 1] != cc) {
          if (curr_index == static_cast<int>(s_f_size_)) {
            curr_index = max_index - 1;
          } else {
            curr_index = Select_Sm_Md(curr_index + 1, select_bb_table_,
                                      bb_f_buff_, interval_bb_) -
                         1;
          }
        } else {
          curr_index = Select_Sm_Md(curr_index, select_bb_table_, bb_f_buff_,
                                    interval_bb_) +
                       padding;
        }
        if (lower_bound <= curr_index && curr_index <= upper_bound) {
          break;
        }
      }
    }
  }
  std::sort(results.get(), results.get() + count);
  return count;
}

void RLEBWT::Search_Medium() {
  if (!load_index_) {
    const auto c_f_name = index_folder_ + "/" + filename_ + ".ct";
    int c_t_file = open(c_f_name.c_str(), O_RDONLY);
    read(c_t_file, c_s_table_.get(), CHUNK_SIZE);
    read(c_t_file, c_table_.get(), CHUNK_SIZE);
    close(c_t_file);
    int count = 0;
    for (int i = 0; i != NUMBER_OF_CHAR; ++i) {
      if (c_s_table_[i]) {
        mapping_table_[i] = count++;
      }
    }
    Sum_C_Table();
    num_of_char_ = count;
  }
  // load all the stuff to the buffer
  const auto s_i_f_n = index_folder_ + "/" + filename_ + ".s";
  const auto b_i_f_n = index_folder_ + "/" + filename_ + ".b";
  const auto bs_i_f_n = index_folder_ + "/" + filename_ + ".bs";
  const auto bb_i_f_n = index_folder_ + "/" + filename_ + ".bb";
  s_f_buff_ = std::make_unique<char[]>(s_f_size_);
  read(s_f_, s_f_buff_.get(), s_f_size_);
  b_f_buff_ = std::make_unique<char[]>(b_f_size_);
  read(b_f_, b_f_buff_.get(), b_f_size_);
  bb_f_buff_ = std::make_unique<char[]>(b_f_size_);
  read(bb_f_, bb_f_buff_.get(), b_f_size_);
  int s_i_f = open(s_i_f_n.c_str(), O_RDONLY);
  int s_i_f_size = lseek(s_i_f, 0, SEEK_END);
  lseek(s_i_f, 0, SEEK_SET);
  occ_s_table_ = std::make_unique<int32_t[]>(s_i_f_size / 4);
  read(s_i_f, occ_s_table_.get(), s_i_f_size);
  int b_i_f = open(b_i_f_n.c_str(), O_RDONLY);
  int b_i_f_size = lseek(b_i_f, 0, SEEK_END);
  lseek(b_i_f, 0, SEEK_SET);
  occ_b_table_ = std::make_unique<int32_t[]>(b_i_f_size / 4);
  read(b_i_f, occ_b_table_.get(), b_i_f_size);
  int bs_i_f = open(bs_i_f_n.c_str(), O_RDONLY);
  int bs_i_f_size = lseek(bs_i_f, 0, SEEK_END);
  select_b_table_ = std::make_unique<int32_t[]>(bs_i_f_size / 4);
  lseek(bs_i_f, 0, SEEK_SET);
  read(bs_i_f, select_b_table_.get(), bs_i_f_size);
  int bb_i_f = open(bb_i_f_n.c_str(), O_RDONLY);
  int bb_i_f_size = lseek(bb_i_f, 0, SEEK_END);
  lseek(bb_i_f, 0, SEEK_SET);
  select_bb_table_ = std::make_unique<int32_t[]>(bb_i_f_size / 4);
  read(bb_i_f, select_bb_table_.get(), bb_i_f_size);
  close(s_i_f);
  close(b_i_f);
  close(bs_i_f);
  close(bb_i_f);
  // interval getting
  interval_b_ = std::ceil(static_cast<double>(s_f_size_) / (b_f_size_ / 2) * 4);
  interval_bb_ = std::ceil(static_cast<double>(s_f_size_) / b_f_size_ * 4);
  int chunk_size = sizeof(int32_t) * num_of_char_;
  int max_chunks_nums = (s_f_size_ - 2 * CHUNK_SIZE) / chunk_size;
  if (max_chunks_nums > 0) {
    step_s_size_ = std::ceil(static_cast<double>(s_f_size_) / max_chunks_nums);
  } else {
    step_s_size_ = s_f_size_ + 1;
  }
  if (mode == 'm') {
    std::cout << search_m_md() << '\n';
  } else if (mode == 'a') {
    auto results = std::make_unique<size_t[]>(MAX_RESULT_NUM);
    auto count = search_a_md(results);
    for (int i = 0; i != count; ++i) {
      std::cout << '[' << results[i] << ']' << '\n';
    }
  } else if (mode == 'r') {
    std::cout << search_r_md() << '\n';
  } else if (mode == 'n') {
    auto result = std::make_unique<char[]>(MAX_SEARCH_PATTERN_LEN);
    auto count = search_n_md(result);
    if (count == -1) {
      return;
    }
    for (int i = 0; i != count; ++i) {
      std::cout << result[i];
    }
    std::cout << '\n';
  } else {
    std::cerr << "Invalid search flag.\n";
  }
}
