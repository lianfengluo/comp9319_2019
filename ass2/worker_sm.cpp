#include "worker.h"

static constexpr uint8_t B2 = (1 << 4) - 1;
static constexpr uint8_t B1 = B2 ^ (B2 << 2);
static constexpr uint8_t B0 = B1 ^ (B1 << 1);

void RLEBWT::Build_BB_Index_SM() {
  bb_f_buff_ = new char[b_f_size_];
  int write_pos = 0, bit_index = 0, num_of_1 = 0, c = 0, write_pos_bb = 0;
  select_bb_table_ = new int[s_f_size_];
  occ_bb_table_ = new int[(b_f_size_ - 1) / 4 + 1];
  int max = c_s_table_[NUMBER_OF_CHAR - 1], count_4 = 0;
  int readByte = read(bb_f_, bb_f_buff_.get(), b_f_size_);
  for (int i = 0; i != readByte; ++i) {
    c = bb_f_buff_[i];
    for (int j = 0; j != 8; ++j) {
      if ((c << j) & FIRST_BIT) {
        ++num_of_1;
        if (num_of_1 % 4 == 0) {
          select_bb_table_[write_pos] = bit_index;
          ++write_pos;
        }
        if (max == num_of_1) {
          occ_bb_table_[write_pos_bb] = num_of_1;
          return;
        }
      }
      ++bit_index;
    }
    if (++count_4 == 4) {
      count_4 = 0;
      occ_bb_table_[write_pos_bb] = num_of_1;
      ++write_pos_bb;
    }
  }
}

void RLEBWT::Build_S_B_Index_SM(int s_f, int b_f) {
  if (s_f_size_ == 0) {
    fprintf(stderr, "Input file is empty.");
    return;
  }
  // s_f: s_f_size + s_f_i: s_f_size + b_f, bb_f: 2 * b_f_size + o_b, o_bb: 2 *
  // b_f_size + select_b, select_bb: 2 * s_f_size
  int space_can_be_use = MAX_FREE_MEMORY - 4 * b_f_size_ - 4 * s_f_size_;
  int chunk_size = num_of_char_ * sizeof(int32_t);
  int max_chunks_nums = space_can_be_use / chunk_size;
  int step_size = (s_f_size_ / max_chunks_nums == 0)
                      ? 1
                      : ((s_f_size_ - 1) / max_chunks_nums + 1);
  step_s_size_ = step_size;
  int real_chunks_nums = s_f_size_ / step_size;
  s_f_buff_ = new char[s_f_size_];
  b_f_buff_ = new char[b_f_size_];
  MyArray<int32_t> temp_table = new int32_t[num_of_char_];
  for (int i = 0; i != num_of_char_; ++i) {
    temp_table[i] = 0;
  }
  s_i_f_size_ = real_chunks_nums * num_of_char_ * 4;
  occ_s_table_ = new int32_t[s_i_f_size_ / 4];
  occ_b_table_ = new int32_t[((b_f_size_ - 1) / 4) + 1];
  // every 4's 1 build the index
  select_b_table_ = new int32_t[s_f_size_];
  int s_index = 0, num_of_1 = 0, write_pos_b = 0, step_count = 0, write_pos = 0,
      count_4 = 0, bit_index = 0, write_pos_s_b = 0;
  int c = 0, byte = 0;
  bool done = false;
  int s_f_r = read(s_f, s_f_buff_.get(), s_f_size_);
  int readByte = read(b_f, b_f_buff_.get(), b_f_size_);
  for (int i = 0; i != readByte; ++i) {
    byte = b_f_buff_[i];
    for (int j = 0; j != 8; ++j) {
      if (((byte << j) & FIRST_BIT) == 0) {
        // 0-bit
        ++c_table_[c];
      } else {
        // 1-bit
        if (done) break;
        c = s_f_buff_[s_index++];
        ++temp_table[mapping_table_[c]];
        ++c_table_[c];
        ++num_of_1;
        if (num_of_1 % 4 == 0) {
          select_b_table_[write_pos_s_b] = bit_index;
          ++write_pos_s_b;
        }
        ++step_count;
        if (step_size == step_count) {
          step_count = 0;
          auto base = write_pos * num_of_char_;
          for (int k = 0; k != num_of_char_; ++k) {
            occ_s_table_[base + k] = temp_table[k];
          }
          ++write_pos;
        }
        if (s_index == s_f_r) {
          done = true;
        }
      }
      ++bit_index;
    }
    if (done) {
      occ_b_table_[write_pos_b] = num_of_1;
      break;
    }
    if (++count_4 == 4) {
      count_4 = 0;
      occ_b_table_[write_pos_b] = num_of_1;
      ++write_pos_b;
    }
  }
  Sum_C_Table();
}

int Select_Sm_Md(int index, const MyArray<int32_t>& select_table,
                 const MyArray<char>& f_buff, int interval) {
  int pos = index / interval;
  int rest_of_1 = index % interval;
  int bit_index = 0, byte = 0;
  if (pos > 0) {
    bit_index = select_table[pos - 1];
  } else {
    int start = 0;
    while (true) {
      byte = f_buff[start];
      for (int i = 0; i != 8; ++i) {
        if ((byte << i) & FIRST_BIT) {
          --index;
          if (index == 0) return bit_index;
        }
        ++bit_index;
      }
      ++start;
    }
  }
  if (rest_of_1 == 0) {
    return bit_index;
  }
  int start = (bit_index + 1) / 8;
  int start_bit = (bit_index + 1) % 8;
  while (true) {
    byte = f_buff[start];
    for (int i = start_bit; i != 8; ++i) {
      ++bit_index;
      if ((byte << i) & FIRST_BIT) {
        --rest_of_1;
        if (rest_of_1 == 0) {
          return bit_index;
        }
      }
    }
    start_bit = 0;
    ++start;
  }
  return bit_index;
}

int Rank_Sm_Md_Function(const MyArray<char>& buff, const MyArray<int32_t>& occ,
                        int index, int step_record) {
  // bit position, every 32 bits(4 byte * 8) get one record;
  const int step_bits = step_record * 8;
  int location_b_chunk = index / step_bits;
  int b_start_place = 0, occ_b = 0, byte = 0;
  if (location_b_chunk > 0) {
    occ_b = occ[location_b_chunk - 1];
    b_start_place = location_b_chunk * step_bits;
  }
  int b_start_byte_pos = (b_start_place) / 8;
  int end_b_byte_pos = index / 8;
  int end_b_bit_pos = index % 8;
  if (index % step_bits != 0) {
    while (true) {
      byte = buff[b_start_byte_pos];
      if (b_start_byte_pos == end_b_byte_pos) {
        for (int i = 0; i < end_b_bit_pos; ++i) {
          if ((byte << i) & FIRST_BIT) {
            ++occ_b;
          }
        }
        return occ_b;
      } else {
        // adding number of bit 1 to the occ_b
        byte = ((byte >> 1) & B0) + (byte & B0);
        byte = ((byte >> 2) & B1) + (byte & B1);
        byte = ((byte >> 4) + byte) & B2;
        occ_b += byte;
        ++b_start_byte_pos;
      }
    }
  }
  return occ_b;
}

int Occ_Function_Sm_Md(int c, int index_s, const MyArray<int32_t>& occ,
                       const MyArray<char>& buff,
                       const MyArray<int32_t>& map_table, int num_of_char,
                       int step_size, size_t s_i_f_size) {
  int chunk_s_location = index_s / step_size;
  int occ_s = 0;
  int start_s_place = 0;
  if (chunk_s_location > 0) {
    occ_s = occ[(chunk_s_location - 1) * num_of_char + map_table[c]];
    if ((chunk_s_location * num_of_char + map_table[c]) * 4 < s_i_f_size) {
      int occ_s2 = occ[chunk_s_location * num_of_char + map_table[c]];
      if (occ_s == occ_s2) {
        return occ_s;
      }
    }
    start_s_place = chunk_s_location * step_size;
  }
  for (int i = start_s_place; i < index_s; ++i) {
    if (buff[i] == c) ++occ_s;
  }
  return occ_s;
}

void RLEBWT::get_lower_uppder_bound_sm(int& lower_bound, int& upper_bound,
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
        Rank_Sm_Md_Function(b_f_buff_, occ_b_table_, lower_bound + 1, 4);
    upper_index =
        Rank_Sm_Md_Function(b_f_buff_, occ_b_table_, upper_bound + 1, 4);
    occurrence_1 = Occ_Function_Sm_Md(c, lower_index - 1, occ_s_table_,
                                      s_f_buff_, mapping_table_, num_of_char_,
                                      step_s_size_, s_i_f_size_);
    occurrence_2 = Occ_Function_Sm_Md(c, upper_index, occ_s_table_, s_f_buff_,
                                      mapping_table_, num_of_char_,
                                      step_s_size_, s_i_f_size_);
    lower_bound = c_s_table_[c - 1] + occurrence_1 + 1;
    upper_bound = c_s_table_[c - 1] + occurrence_2;
    if (lower_bound > upper_bound) {
      break;
    }
    padding_0_1 =
        (s_f_buff_[lower_index - 1] == c)
            ? pre_lower - Select_Sm_Md(lower_index, select_b_table_, b_f_buff_)
            : 0;
    padding_0_2 =
        (s_f_buff_[upper_index - 1] == c)
            ? pre_upper - Select_Sm_Md(upper_index, select_b_table_, b_f_buff_)
            : 0;
    if (s_f_buff_[lower_index - 1] != c) {
      lower_bound = Select_Sm_Md(lower_bound, select_bb_table_, bb_f_buff_);
    } else {
      lower_bound =
          Select_Sm_Md(lower_bound, select_bb_table_, bb_f_buff_) + padding_0_1;
    }
    if (s_f_buff_[upper_index - 1] != c) {
      if (upper_bound == static_cast<int>(s_f_size_)) {
        upper_bound = max_index - 1;
      } else {
        upper_bound =
            Select_Sm_Md(upper_bound + 1, select_bb_table_, bb_f_buff_) - 1;
      }
    } else {
      upper_bound =
          Select_Sm_Md(upper_bound, select_bb_table_, bb_f_buff_) + padding_0_2;
    }
  }
}

int RLEBWT::search_m_sm() {
  int c = search_pattern_[len_of_pattern_ - 1], count = 0;
  int lower_bound = c_table_[c - 1], upper_bound = c_table_[c] - 1;
  get_lower_uppder_bound_sm(lower_bound, upper_bound, c);
  if (upper_bound >= lower_bound) count = upper_bound - lower_bound + 1;
  return count;
}

int RLEBWT::search_r_sm() {
  int c = search_pattern_[len_of_pattern_ - 1], count = 0;
  int lower_bound = c_table_[c - 1], upper_bound = c_table_[c] - 1;
  get_lower_uppder_bound_sm(lower_bound, upper_bound, c);
  if (upper_bound >= lower_bound) {
    int occ = 0, curr_index = 0, padding = 0, cc = 0;
    int max_index = c_table_[NUMBER_OF_CHAR - 1], pre_index = 0, rank_index = 0;
    for (int i = lower_bound; i <= upper_bound; ++i) {
      curr_index = i;
      while (true) {
        pre_index = curr_index;
        rank_index =
            Rank_Sm_Md_Function(b_f_buff_, occ_b_table_, curr_index + 1, 4);
        cc = s_f_buff_[rank_index - 1];
        if (cc == ']') {
          ++count;
          break;
        }
        occ = Occ_Function_Sm_Md(cc, rank_index, occ_s_table_, s_f_buff_,
                                 mapping_table_, num_of_char_, step_s_size_,
                                 s_i_f_size_);
        curr_index = c_s_table_[cc - 1] + occ;
        padding = (s_f_buff_[rank_index - 1] == cc)
                      ? pre_index -
                            Select_Sm_Md(rank_index, select_b_table_, b_f_buff_)
                      : 0;
        if (s_f_buff_[rank_index - 1] != cc) {
          if (curr_index == static_cast<int>(s_f_size_)) {
            curr_index = max_index - 1;
          } else {
            curr_index =
                Select_Sm_Md(curr_index + 1, select_bb_table_, bb_f_buff_) - 1;
          }
        } else {
          curr_index =
              Select_Sm_Md(curr_index, select_bb_table_, bb_f_buff_) + padding;
        }
        if (lower_bound <= curr_index && curr_index <= upper_bound) {
          break;
        }
      }
    }
  }
  return count;
}

int RLEBWT::search_a_sm(MyArray<size_t>& results) {
  int c = search_pattern_[len_of_pattern_ - 1], count = 0;
  int lower_bound = c_table_[c - 1], upper_bound = c_table_[c] - 1;
  get_lower_uppder_bound_sm(lower_bound, upper_bound, c);
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
            Rank_Sm_Md_Function(b_f_buff_, occ_b_table_, curr_index + 1, 4);
        cc = s_f_buff_[rank_index - 1];
        if (cc == '[') {
          results[count] = result;
          ++count;
          break;
        }
        if (record) {
          result += (cc - '0') * static_cast<size_t>(pow(10, result_len));
          ++result_len;
        }
        if (cc == ']') {
          record = true;
        }
        occ = Occ_Function_Sm_Md(cc, rank_index, occ_s_table_, s_f_buff_,
                                 mapping_table_, num_of_char_, step_s_size_,
                                 s_i_f_size_);
        curr_index = c_s_table_[cc - 1] + occ;
        padding = (s_f_buff_[rank_index - 1] == cc)
                      ? pre_index -
                            Select_Sm_Md(rank_index, select_b_table_, b_f_buff_)
                      : 0;
        if (s_f_buff_[rank_index - 1] != cc) {
          if (curr_index == static_cast<int>(s_f_size_)) {
            curr_index = max_index - 1;
          } else {
            curr_index =
                Select_Sm_Md(curr_index + 1, select_bb_table_, bb_f_buff_) - 1;
          }
        } else {
          curr_index =
              Select_Sm_Md(curr_index, select_bb_table_, bb_f_buff_) + padding;
        }
        if (lower_bound <= curr_index && curr_index <= upper_bound) {
          break;
        }
      }
    }
  }
  qsort(results.get(), count, sizeof(size_t), Compare);
  return count;
}

int RLEBWT::binary_search_s_sm(int pos_c, int c) {
  int start = pos_c, end = s_f_size_, mid = 0, occ = 0;
  while (true) {
    mid = (start + end) / 2;
    if (start > end) return -1;
    occ = Occ_Function_Sm_Md(c, mid, occ_s_table_, s_f_buff_, mapping_table_,
                             num_of_char_, step_s_size_, s_i_f_size_);
    if (occ == pos_c) {
      if (s_f_buff_[mid - 1] == c) {
        return mid;
      } else {
        end = mid - 1;
      }
    } else if (occ < pos_c) {
      start = mid + 1;
    } else {
      end = mid - 1;
    }
  }
  return -1;
}

int RLEBWT::search_n_sm(MyArray<char>& result) {
  ++len_of_pattern_;
  search_pattern_[len_of_pattern_ - 1] = ']';
  int c = ']', curr_index = 0, count = 0,
      max_index = c_table_[NUMBER_OF_CHAR - 1];
  MyArray<int32_t> reverse_map = new int32_t[num_of_char_];
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
        Rank_Sm_Md_Function(b_f_buff_, occ_b_table_, lower_bound + 1, 4);
    upper_index =
        Rank_Sm_Md_Function(b_f_buff_, occ_b_table_, upper_bound + 1, 4);
    occurrence_1 = Occ_Function_Sm_Md(c, lower_index - 1, occ_s_table_,
                                      s_f_buff_, mapping_table_, num_of_char_,
                                      step_s_size_, s_i_f_size_);
    occurrence_2 = Occ_Function_Sm_Md(c, upper_index, occ_s_table_, s_f_buff_,
                                      mapping_table_, num_of_char_,
                                      step_s_size_, s_i_f_size_);
    lower_bound = c_s_table_[c - 1] + occurrence_1 + 1;
    upper_bound = c_s_table_[c - 1] + occurrence_2;
    if (lower_bound > upper_bound) {
      return -1;
    }
    padding_0_1 =
        (s_f_buff_[lower_index - 1] == c)
            ? pre_lower - Select_Sm_Md(lower_index, select_b_table_, b_f_buff_)
            : 0;
    padding_0_2 =
        (s_f_buff_[upper_index - 1] == c)
            ? pre_upper - Select_Sm_Md(upper_index, select_b_table_, b_f_buff_)
            : 0;
    if (s_f_buff_[lower_index - 1] != c) {
      lower_bound = Select_Sm_Md(lower_bound, select_bb_table_, bb_f_buff_);
    } else {
      lower_bound =
          Select_Sm_Md(lower_bound, select_bb_table_, bb_f_buff_) + padding_0_1;
    }
    if (s_f_buff_[upper_index - 1] != c) {
      if (upper_bound == static_cast<int>(s_f_size_)) {
        upper_bound = max_index - 1;
      } else {
        upper_bound =
            Select_Sm_Md(upper_bound + 1, select_bb_table_, bb_f_buff_) - 1;
      }
    } else {
      upper_bound =
          Select_Sm_Md(upper_bound, select_bb_table_, bb_f_buff_) + padding_0_2;
    }
    if (c == '[') {
      curr_index = lower_bound;
      break;
    }
  }
  // start forward search
  int pre_i_th_1 = 0, pos_c = 0, pos_s_1 = 0, record = false;
  // int pre_1_pos = 0;
  while (true) {
    pre_i_th_1 =
        Rank_Sm_Md_Function(bb_f_buff_, occ_bb_table_, curr_index + 1, 4);
    if (pre_i_th_1 == -1) return -1;
    padding_0_1 =
        curr_index - Select_Sm_Md(pre_i_th_1, select_bb_table_, bb_f_buff_);
    pos_c = pre_i_th_1 - c_s_table_[c - 1];
    pos_s_1 = binary_search_s_sm(pos_c, c);
    if (pos_s_1 == -1) return -1;
    curr_index =
        Select_Sm_Md(pos_s_1, select_b_table_, b_f_buff_) + padding_0_1;
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

void RLEBWT::Search_Sm() {
  if (mode == 'm') {
    printf("%d\n", search_m_sm());
  } else if (mode == 'a') {
    MyArray<size_t> results = new size_t[MAX_RESULT_NUM];
    auto count = search_a_sm(results);
    for (int i = 0; i != count; ++i) {
      printf("[%zu]\n", results[i]);
    }
  } else if (mode == 'r') {
    printf("%d\n", search_r_sm());
  } else if (mode == 'n') {
    MyArray<char> result = new char[MAX_SEARCH_PATTERN_LEN];
    auto count = search_n_sm(result);
    if (count == -1) {
      return;
    }
    for (int i = 0; i != count; ++i) {
      putchar(result[i]);
    }
    putchar('\n');
  } else {
    fprintf(stderr, "Invalid search flag.\n");
  }
}
