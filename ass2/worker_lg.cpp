#include "worker.h"

const int STEP_LG_B = 8;

static int rank_lg_function(const std::unique_ptr<char[]>& buff,
                            const std::unique_ptr<int32_t[]>& occ, int index,
                            int step_record, bool load_occ, bool load_f,
                            int b_f, int b_f_i) {
  // bit position, every 32 bits(4 byte * 8) get one record;
  const int step_bits = step_record * 8;
  int location_b_chunk = index / step_bits;
  int b_start_place = 0, occ_b = 0, byte = 0;
  if (location_b_chunk > 0) {
    if (load_occ) {
      occ_b = occ[location_b_chunk - 1];
    } else {
      lseek(b_f_i, 4 * (location_b_chunk - 1), SEEK_SET);
      read(b_f_i, &occ_b, 4);
    }
    b_start_place = location_b_chunk * step_bits;
  }
  int b_start_byte_pos = b_start_place / 8;
  int end_b_byte_pos = index / 8;
  int end_b_bit_pos = index % 8;
  if (index % step_bits != 0) {
    if (load_f) {
      while (true) {
        byte = buff[b_start_byte_pos];
        if (b_start_byte_pos == end_b_byte_pos) {
          for (int i = 0; i < end_b_bit_pos; ++i) {
            if ((byte << i) & FIRST_BIT) {
              ++occ_b;
            }
          }
          break;
        } else {
          for (int i = 0; i != 8; ++i) {
            if ((byte << i) & FIRST_BIT) {
              ++occ_b;
            }
          }
          ++b_start_byte_pos;
        }
      }
    } else {
      lseek(b_f, b_start_byte_pos, SEEK_SET);
      std::array<char, STEP_LG_B> buff2{'\0'};
      read(b_f, buff2.data(), STEP_LG_B);
      for (int j = 0; j != STEP_LG_B; ++j) {
        byte = buff2[j];
        if (b_start_byte_pos + j == end_b_byte_pos) {
          for (int i = 0; i < end_b_bit_pos; ++i) {
            if ((byte << i) & FIRST_BIT) {
              ++occ_b;
            }
          }
          break;
        } else {
          for (int i = 0; i != 8; ++i) {
            if ((byte << i) & FIRST_BIT) {
              ++occ_b;
            }
          }
        }
      }
    }
  }
  return occ_b;
}

int Occ_Function_Lg(int c, int index_s, const std::unique_ptr<int32_t[]>& occ,
                    const std::unique_ptr<char[]>& buff,
                    const std::unique_ptr<int32_t[]>& map_table,
                    int num_of_char, int step_size, bool load_s, bool load_r_s,
                    int s_f, int s_f_i) {
  int chunk_s_location = index_s / step_size;
  int occ_s = 0;
  int start_s_place = 0;
  if (chunk_s_location > 0) {
    if (load_r_s) {
      occ_s = occ[(chunk_s_location - 1) * num_of_char + map_table[c]];
    } else {
      lseek(s_f_i, ((chunk_s_location - 1) * num_of_char + map_table[c]) * 4,
            SEEK_SET);
      read(s_f_i, &occ_s, 4);
    }
    start_s_place = chunk_s_location * step_size;
  }
  if (start_s_place != index_s) {
    if (load_s) {
      for (int i = start_s_place; i < index_s; ++i) {
        if (buff[i] == c) ++occ_s;
      }
    } else {
      lseek(s_f, start_s_place, SEEK_SET);
      std::array<char, CHUNK_SIZE> buff2{'\0'};
      read(s_f, buff2.data(), CHUNK_SIZE);
      int end = index_s - start_s_place;
      for (int i = 0; i < end; ++i) {
        if (buff2[i] == c) ++occ_s;
      }
    }
  }
  return occ_s;
}

static int Select_Lg(int index, const std::unique_ptr<int[]>& select_table,
                     const std::unique_ptr<char[]>& f_buff, int interval,
                     bool load_f, bool load_i_f, int file, int index_file) {
  int pos = index / interval;
  int rest_of_1 = index % interval;
  int bit_index = 0, byte = 0;
  if (pos > 0) {
    if (load_i_f) {
      bit_index = select_table[pos - 1];
    } else {
      lseek(index_file, (pos - 1) * 4, SEEK_SET);
      read(index_file, &bit_index, 4);
    }
  } else {
    int start = 0;
    if (load_f) {
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
    } else {
      lseek(file, 0, SEEK_SET);
      std::array<char, MIN_READ_BUFF> r_buff;
      read(file, r_buff.data(), MIN_READ_BUFF);
      while (true) {
        byte = r_buff[start];
        for (int i = 0; i != 8; ++i) {
          if ((byte << i) & FIRST_BIT) {
            --index;
            if (index == 0) return bit_index;
          }
          ++bit_index;
        }
        ++start;
        if (start == MIN_READ_BUFF) {
          start = 0;
          read(file, r_buff.data(), MIN_READ_BUFF);
        }
      }
    }
  }
  if (rest_of_1 == 0) {
    return bit_index;
  }
  int start = (bit_index + 1) / 8;
  int start_bit = (bit_index + 1) % 8;
  if (load_f) {
    while (true) {
      byte = f_buff[start];
      for (int i = start_bit; i != 8; ++i) {
        ++bit_index;
        if ((byte << i) & FIRST_BIT) {
          --rest_of_1;
          if (rest_of_1 == 0) return bit_index;
        }
      }
      start_bit = 0;
      ++start;
    }
  } else {
    lseek(file, start, SEEK_SET);
    std::array<char, MIN_READ_BUFF> r_buff;
    read(file, r_buff.data(), MIN_READ_BUFF);
    start = 0;
    while (true) {
      byte = r_buff[start];
      for (int i = start_bit; i != 8; ++i) {
        ++bit_index;
        if ((byte << i) & FIRST_BIT) {
          --rest_of_1;
          if (rest_of_1 == 0) return bit_index;
        }
      }
      ++start;
      start_bit = 0;
      if (start == MIN_READ_BUFF) {
        start = 0;
        read(file, r_buff.data(), MIN_READ_BUFF);
      }
    }
  }
  return bit_index;
}

void RLEBWT::get_lower_uppder_bound_lg(int& lower_bound, int& upper_bound,
                                       int c) {
  int occurrence_1 = 0, occurrence_2 = 0;
  int search_index = len_of_pattern_ - 1, padding_0_1 = 0, padding_0_2 = 0,
      pre_lower = 0, pre_upper = 0, upper_index = 0, lower_index = 0,
      max_index = c_table_[NUMBER_OF_CHAR - 1];
  bool lower_want = false, upper_want = false;
  while (search_index > 0) {
    c = search_pattern_[--search_index];
    pre_lower = lower_bound;
    pre_upper = upper_bound;
    lower_index = rank_lg_function(b_f_buff_, occ_b_table_, lower_bound + 1, 8,
                                   load_r_b_, load_b_, b_f_, b_i_f_);
    upper_index = rank_lg_function(b_f_buff_, occ_b_table_, upper_bound + 1, 8,
                                   load_r_b_, load_b_, b_f_, b_i_f_);
    if (load_s_ && load_r_s_) {
      occurrence_1 =
          Occ_Function_Sm_Md(c, lower_index - 1, occ_s_table_, s_f_buff_,
                             mapping_table_, num_of_char_, step_s_size_);
      occurrence_2 =
          Occ_Function_Sm_Md(c, upper_index, occ_s_table_, s_f_buff_,
                             mapping_table_, num_of_char_, step_s_size_);
    } else {
      occurrence_1 = Occ_Function_Lg(
          c, lower_index - 1, occ_s_table_, s_f_buff_, mapping_table_,
          num_of_char_, step_s_size_, load_s_, load_r_s_, s_f_, s_i_f_);
      occurrence_2 = Occ_Function_Lg(c, upper_index, occ_s_table_, s_f_buff_,
                                     mapping_table_, num_of_char_, step_s_size_,
                                     load_s_, load_r_s_, s_f_, s_i_f_);
    }
    lower_bound = c_s_table_[c - 1] + occurrence_1 + 1;
    upper_bound = c_s_table_[c - 1] + occurrence_2;
    if (lower_bound > upper_bound) {
      break;
    }
    if (!load_s_) {
      int temp_c = '\0';
      lseek(s_f_, upper_index - 1, SEEK_SET);
      read(s_f_, &temp_c, 1);
      if (temp_c == c) {
        upper_want = true;
      }
      lseek(s_f_, lower_index - 1, SEEK_SET);
      read(s_f_, &temp_c, 1);
      if (temp_c == c) {
        lower_want = true;
      }
    } else {
      lower_want = (s_f_buff_[lower_index - 1] == c);
      upper_want = (s_f_buff_[upper_index - 1] == c);
    }
    padding_0_1 = (lower_want)
                      ? pre_lower - Select_Lg(lower_index, select_b_table_,
                                              b_f_buff_, interval_b_, load_b_,
                                              load_s_b_, b_f_, bs_i_f_)
                      : 0;
    padding_0_2 = (upper_want)
                      ? pre_upper - Select_Lg(upper_index, select_b_table_,
                                              b_f_buff_, interval_b_, load_b_,
                                              load_s_b_, b_f_, bs_i_f_)
                      : 0;
    if (!lower_want) {
      lower_bound =
          Select_Lg(lower_bound, select_bb_table_, bb_f_buff_, interval_bb_,
                    load_bb_, load_s_bb_, bb_f_, bb_i_f_);
    } else {
      lower_bound =
          Select_Lg(lower_bound, select_bb_table_, bb_f_buff_, interval_bb_,
                    load_bb_, load_s_bb_, bb_f_, bb_i_f_) +
          padding_0_1;
    }
    if (!upper_want) {
      if (upper_bound == static_cast<int>(s_f_size_)) {
        upper_bound = max_index - 1;
      } else {
        upper_bound =
            Select_Lg(upper_bound + 1, select_bb_table_, bb_f_buff_,
                      interval_bb_, load_bb_, load_s_bb_, bb_f_, bb_i_f_) -
            1;
      }
    } else {
      upper_bound =
          Select_Lg(upper_bound, select_bb_table_, bb_f_buff_, interval_bb_,
                    load_bb_, load_s_bb_, bb_f_, bb_i_f_) +
          padding_0_2;
    }
    lower_want = false;
    upper_want = false;
  }
}

int RLEBWT::search_a_lg(std::unique_ptr<size_t[]>& results) {
  int c = search_pattern_[len_of_pattern_ - 1], count = 0;
  int lower_bound = c_table_[c - 1], upper_bound = c_table_[c] - 1;
  get_lower_uppder_bound_lg(lower_bound, upper_bound, c);
  if (upper_bound >= lower_bound) {
    int occ = 0, curr_index = 0, padding = 0, cc = 0;
    int max_index = c_table_[NUMBER_OF_CHAR - 1], pre_index = 0, rank_index = 0;
    bool index_want = false;
    for (int i = lower_bound; i <= upper_bound; ++i) {
      size_t result = 0;
      int result_len = 0;
      curr_index = i;
      bool record = false;
      while (true) {
        pre_index = curr_index;
        rank_index = rank_lg_function(b_f_buff_, occ_b_table_, curr_index + 1,
                                      8, load_r_b_, load_b_, b_f_, b_i_f_);
        if (load_s_) {
          cc = s_f_buff_[rank_index - 1];
        } else {
          lseek(s_f_, rank_index - 1, SEEK_SET);
          read(s_f_, &cc, 1);
        }
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
        if (load_s_ && load_r_s_) {
          occ = Occ_Function_Sm_Md(cc, rank_index, occ_s_table_, s_f_buff_,
                                   mapping_table_, num_of_char_, step_s_size_);
        } else {
          occ = Occ_Function_Lg(cc, rank_index, occ_s_table_, s_f_buff_,
                                mapping_table_, num_of_char_, step_s_size_,
                                load_s_, load_r_s_, s_f_, s_i_f_);
        }
        if (load_s_) {
          index_want = (s_f_buff_[rank_index - 1] == cc);
        } else {
          char temp_c = 0;
          lseek(s_f_, rank_index - 1, SEEK_SET);
          read(s_f_, &temp_c, 1);
          if (temp_c == cc) {
            index_want = true;
          } else {
            index_want = false;
          }
        }
        curr_index = c_s_table_[cc - 1] + occ;
        padding = (index_want)
                      ? pre_index - Select_Lg(rank_index, select_b_table_,
                                              b_f_buff_, interval_b_, load_b_,
                                              load_s_b_, b_f_, bs_i_f_)
                      : 0;
        if (!index_want) {
          if (curr_index == static_cast<int>(s_f_size_)) {
            curr_index = max_index - 1;
          } else {
            curr_index =
                Select_Lg(curr_index + 1, select_bb_table_, bb_f_buff_,
                          interval_bb_, load_bb_, load_s_bb_, bb_f_, bb_i_f_) -
                1;
          }
        } else {
          curr_index =
              Select_Lg(curr_index, select_bb_table_, bb_f_buff_, interval_bb_,
                        load_bb_, load_s_bb_, bb_f_, bb_i_f_) +
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

int RLEBWT::search_r_lg() {
  int c = search_pattern_[len_of_pattern_ - 1], count = 0;
  int lower_bound = c_table_[c - 1], upper_bound = c_table_[c] - 1;
  bool index_want = false;
  get_lower_uppder_bound_lg(lower_bound, upper_bound, c);
  if (upper_bound >= lower_bound) {
    int occ = 0, curr_index = 0, padding = 0, cc = 0;
    int max_index = c_table_[NUMBER_OF_CHAR - 1], pre_index = 0, rank_index = 0;
    for (int i = lower_bound; i <= upper_bound; ++i) {
      curr_index = i;
      while (true) {
        pre_index = curr_index;
        rank_index = rank_lg_function(b_f_buff_, occ_b_table_, curr_index + 1,
                                      8, load_r_b_, load_b_, b_f_, b_i_f_);
        cc = s_f_buff_[rank_index - 1];
        if (cc == ']') {
          ++count;
          break;
        }
        if (load_s_ && load_r_s_) {
          occ = Occ_Function_Sm_Md(cc, rank_index, occ_s_table_, s_f_buff_,
                                   mapping_table_, num_of_char_, step_s_size_);
        } else {
          occ = Occ_Function_Lg(cc, rank_index, occ_s_table_, s_f_buff_,
                                mapping_table_, num_of_char_, step_s_size_,
                                load_s_, load_r_s_, s_f_, s_i_f_);
        }
        if (load_s_) {
          index_want = (s_f_buff_[rank_index - 1] == cc);
        } else {
          char temp_c = 0;
          lseek(s_f_, rank_index - 1, SEEK_SET);
          read(s_f_, &temp_c, 1);
          if (temp_c == cc) {
            index_want = true;
          } else {
            index_want = false;
          }
        }
        curr_index = c_s_table_[cc - 1] + occ;
        padding = (index_want)
                      ? pre_index - Select_Lg(rank_index, select_b_table_,
                                              b_f_buff_, interval_b_, load_b_,
                                              load_s_b_, b_f_, bs_i_f_)
                      : 0;
        if (!index_want) {
          if (curr_index == static_cast<int>(s_f_size_)) {
            curr_index = max_index - 1;
          } else {
            curr_index =
                Select_Lg(curr_index + 1, select_bb_table_, bb_f_buff_,
                          interval_bb_, load_bb_, load_s_bb_, bb_f_, bb_i_f_) -
                1;
          }
        } else {
          curr_index =
              Select_Lg(curr_index, select_bb_table_, bb_f_buff_, interval_bb_,
                        load_bb_, load_s_bb_, bb_f_, bb_i_f_) +
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

int RLEBWT::search_m_lg() {
  int c = search_pattern_[len_of_pattern_ - 1], count = 0;
  int lower_bound = c_table_[c - 1], upper_bound = c_table_[c] - 1;
  get_lower_uppder_bound_lg(lower_bound, upper_bound, c);
  if (upper_bound >= lower_bound) count = upper_bound - lower_bound + 1;
  return count;
}

static int find_pre_1_lg(int index, int bb_f) {
  int pos = index / 8;
  int bit_pos = index % 8;
  int c = 0;
  char buff[MIN_READ_BUFF] = {'\0'};
  int low_pos = (pos - MIN_READ_BUFF + 1 < 0) ? 0 : pos - MIN_READ_BUFF + 1;
  lseek(bb_f, low_pos, SEEK_SET);
  read(bb_f, buff, MIN_READ_BUFF);
  while (true) {
    c = buff[pos - low_pos];
    for (int i = bit_pos; i != -1; --i) {
      if ((c << i) & FIRST_BIT) {
        return index;
      }
      --index;
    }
    --pos;
    bit_pos = 7;
    if (pos < low_pos) {
      low_pos = (pos - MIN_READ_BUFF + 1 < 0) ? 0 : pos - MIN_READ_BUFF + 1;
      lseek(bb_f, low_pos, SEEK_SET);
      read(bb_f, buff, MIN_READ_BUFF);
    }
  }
  return index;
}

static int binary_select_bb_lg(int pre_1_pos, int start, int end,
                               const std::unique_ptr<int32_t[]>& select_table,
                               const std::unique_ptr<char[]>& buff,
                               int interval, bool load_bb, bool load_bb_i,
                               int bb_f, int bb_i_f) {
  int mid = 0, select_index = 0;
  // pre_1_pos is index of c_table
  while (true) {
    if (start > end) break;
    mid = (start + end) / 2;
    select_index = Select_Lg(mid, select_table, buff, interval, load_bb,
                             load_bb_i, bb_f, bb_i_f);
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

int RLEBWT::binary_search_lg(int pos_c, int c) {
  int start = 1, end = s_f_size_, mid = 0, occ = 0;
  while (true) {
    mid = (start + end) / 2;
    if (start > end) return -1;
    occ = Occ_Function_Lg(c, mid, occ_s_table_, s_f_buff_, mapping_table_,
                          num_of_char_, step_s_size_, load_s_, load_r_s_, s_f_,
                          s_i_f_);
    if (occ == pos_c) {
      if (load_s_) {
        if (s_f_buff_[mid - 1] == c) {
          return mid;
        } else {
          end = mid - 1;
        }
      } else {
        int temp_c = 0;
        lseek(s_f_, mid - 1, SEEK_SET);
        read(s_f_, &temp_c, 1);
        if (temp_c == c) {
          return mid;
        } else {
          end = mid - 1;
        }
      }
    } else if (occ < pos_c) {
      start = mid + 1;
    } else {
      end = mid - 1;
    }
  }
  return -1;
}

int RLEBWT::search_n_lg(std::unique_ptr<char[]>& result) {
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
  bool lower_want = false, upper_want = false;
  while (search_index > -1) {
    if (--search_index == -1) {
      c = '[';
    } else {
      c = search_pattern_[search_index];
    }
    pre_lower = lower_bound;
    pre_upper = upper_bound;
    lower_index = rank_lg_function(b_f_buff_, occ_b_table_, lower_bound + 1, 8,
                                   load_r_b_, load_b_, b_f_, b_i_f_);
    upper_index = rank_lg_function(b_f_buff_, occ_b_table_, upper_bound + 1, 8,
                                   load_r_b_, load_b_, b_f_, b_i_f_);
    if (load_s_ && load_r_s_) {
      occurrence_1 =
          Occ_Function_Sm_Md(c, lower_index - 1, occ_s_table_, s_f_buff_,
                             mapping_table_, num_of_char_, step_s_size_);
      occurrence_2 =
          Occ_Function_Sm_Md(c, upper_index, occ_s_table_, s_f_buff_,
                             mapping_table_, num_of_char_, step_s_size_);
    } else {
      occurrence_1 = Occ_Function_Lg(
          c, lower_index - 1, occ_s_table_, s_f_buff_, mapping_table_,
          num_of_char_, step_s_size_, load_s_, load_r_s_, s_f_, s_i_f_);
      occurrence_2 = Occ_Function_Lg(c, upper_index, occ_s_table_, s_f_buff_,
                                     mapping_table_, num_of_char_, step_s_size_,
                                     load_s_, load_r_s_, s_f_, s_i_f_);
    }
    lower_bound = c_s_table_[c - 1] + occurrence_1 + 1;
    upper_bound = c_s_table_[c - 1] + occurrence_2;
    if (lower_bound > upper_bound) {
      return -1;
    }
    if (!load_s_) {
      int temp_c = '\0';
      lseek(s_f_, upper_index - 1, SEEK_SET);
      read(s_f_, &temp_c, 1);
      if (temp_c == c) {
        upper_want = true;
      }
      lseek(s_f_, lower_index - 1, SEEK_SET);
      read(s_f_, &temp_c, 1);
      if (temp_c == c) {
        lower_want = true;
      }
    } else {
      lower_want = (s_f_buff_[lower_index - 1] == c);
      upper_want = (s_f_buff_[upper_index - 1] == c);
    }
    padding_0_1 = (lower_want)
                      ? pre_lower - Select_Lg(lower_index, select_b_table_,
                                              b_f_buff_, interval_b_, load_b_,
                                              load_s_b_, b_f_, bs_i_f_)
                      : 0;
    padding_0_2 = (upper_want)
                      ? pre_upper - Select_Lg(upper_index, select_b_table_,
                                              b_f_buff_, interval_b_, load_b_,
                                              load_s_b_, b_f_, bs_i_f_)
                      : 0;
    if (!lower_want) {
      lower_bound =
          Select_Lg(lower_bound, select_bb_table_, bb_f_buff_, interval_bb_,
                    load_bb_, load_s_bb_, bb_f_, bb_i_f_);
    } else {
      lower_bound =
          Select_Lg(lower_bound, select_bb_table_, bb_f_buff_, interval_bb_,
                    load_bb_, load_s_bb_, bb_f_, bb_i_f_) +
          padding_0_1;
    }
    if (!upper_want) {
      if (upper_bound == static_cast<int>(s_f_size_)) {
        upper_bound = max_index - 1;
      } else {
        upper_bound =
            Select_Lg(upper_bound + 1, select_bb_table_, bb_f_buff_,
                      interval_bb_, load_bb_, load_s_bb_, bb_f_, bb_i_f_) -
            1;
      }
    } else {
      upper_bound =
          Select_Lg(upper_bound, select_bb_table_, bb_f_buff_, interval_bb_,
                    load_bb_, load_s_bb_, bb_f_, bb_i_f_) +
          padding_0_2;
    }
    if (c == '[') {
      curr_index = lower_bound;
      break;
    }
    lower_want = false;
    upper_want = false;
  }
  // start forward search
  int pre_i_th_1 = 0, pos_c = 0, pos_s_1 = 0, record = false;
  int pre_1_pos = 0;
  while (true) {
    // for no select table
    if (load_bb_) {
      pre_1_pos = find_pre_1(curr_index, bb_f_buff_);
    } else {
      pre_1_pos = find_pre_1_lg(curr_index, bb_f_);
    }
    pre_i_th_1 = binary_select_bb_lg(
        pre_1_pos, c_s_table_[c - 1] + 1, c_s_table_[c], select_bb_table_,
        bb_f_buff_, interval_bb_, load_bb_, load_s_bb_, bb_f_, bb_i_f_);
    if (pre_i_th_1 == -1) return -1;
    padding_0_1 = curr_index - Select_Lg(pre_i_th_1, select_bb_table_,
                                         bb_f_buff_, interval_bb_, load_bb_,
                                         load_s_bb_, bb_f_, bb_i_f_);
    pos_c = pre_i_th_1 - c_s_table_[c - 1];
    pos_s_1 = binary_search_lg(pos_c, c);
    if (pos_s_1 == -1) return -1;
    curr_index = Select_Lg(pos_s_1, select_b_table_, b_f_buff_, interval_b_,
                           load_b_, load_s_b_, b_f_, bs_i_f_) +
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

void RLEBWT::Search_Lg() {
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
  // int free_space = MAX_FREE_MEMORY;
  int free_space = MAX_FREE_MEMORY + 6 * 1024 * 1024;
  const auto s_i_f_name = index_folder_ + "/" + filename_ + ".s";
  const auto b_i_f_name = index_folder_ + "/" + filename_ + ".b";
  const auto bs_i_f_name = index_folder_ + "/" + filename_ + ".bs";
  const auto bb_i_f_name = index_folder_ + "/" + filename_ + ".bb";
  s_i_f_ = open(s_i_f_name.c_str(), O_RDONLY);
  int s_i_f_size = lseek(s_i_f_, 0, SEEK_END);
  lseek(s_i_f_, 0, SEEK_SET);
  b_i_f_ = open(b_i_f_name.c_str(), O_RDONLY);
  int b_i_f_size = lseek(b_i_f_, 0, SEEK_END);
  lseek(b_i_f_, 0, SEEK_SET);
  bs_i_f_ = open(bs_i_f_name.c_str(), O_RDONLY);
  int bs_i_f_size = lseek(bs_i_f_, 0, SEEK_END);
  lseek(bs_i_f_, 0, SEEK_SET);
  bb_i_f_ = open(bb_i_f_name.c_str(), O_RDONLY);
  int bb_i_f_size = lseek(bb_i_f_, 0, SEEK_END);
  lseek(bb_i_f_, 0, SEEK_SET);
  if (free_space >= static_cast<int>(b_f_size_)) {
    bb_f_buff_ = std::make_unique<char[]>(b_f_size_);
    free_space -= b_f_size_;
    read(bb_f_, bb_f_buff_.get(), b_f_size_);
    load_bb_ = true;
  }
  if (mode == 'n') {
    // if it's a n mode load bb index first
    if (free_space >= static_cast<int>(bb_i_f_size)) {
      free_space -= bb_i_f_size;
      select_bb_table_ = std::make_unique<int32_t[]>(bb_i_f_size / 4);
      read(bb_i_f_, select_bb_table_.get(), bb_i_f_size);
      load_s_bb_ = true;
    }
    if (free_space >= static_cast<int>(b_f_size_)) {
      free_space -= b_f_size_;
      b_f_buff_ = std::make_unique<char[]>(b_f_size_);
      read(b_f_, b_f_buff_.get(), b_f_size_);
      load_b_ = true;
    }
  } else {
    if (free_space >= static_cast<int>(b_f_size_)) {
      free_space -= b_f_size_;
      b_f_buff_ = std::make_unique<char[]>(b_f_size_);
      read(b_f_, b_f_buff_.get(), b_f_size_);
      load_b_ = true;
    }
    if (free_space >= static_cast<int>(bb_i_f_size)) {
      free_space -= bb_i_f_size;
      select_bb_table_ = std::make_unique<int32_t[]>(bb_i_f_size / 4);
      read(bb_i_f_, select_bb_table_.get(), bb_i_f_size);
      load_s_bb_ = true;
    }
  }
  if (free_space >= static_cast<int>(b_i_f_size)) {
    free_space -= b_i_f_size;
    occ_b_table_ = std::make_unique<int32_t[]>(b_i_f_size / 4);
    read(b_i_f_, occ_b_table_.get(), b_i_f_size);
    load_r_b_ = true;
  }
  if (free_space >= static_cast<int>(bs_i_f_size)) {
    free_space -= bs_i_f_size;
    select_b_table_ = std::make_unique<int32_t[]>(bs_i_f_size / 4);
    read(bs_i_f_, select_b_table_.get(), bs_i_f_size);
    load_s_b_ = true;
  }
  if (free_space >= static_cast<int>(s_f_size_)) {
    free_space -= s_f_size_;
    s_f_buff_ = std::make_unique<char[]>(s_f_size_);
    read(s_f_, s_f_buff_.get(), s_f_size_);
    load_s_ = true;
  }
  if (free_space >= static_cast<int>(s_i_f_size)) {
    free_space -= s_i_f_size;
    occ_s_table_ = std::make_unique<int32_t[]>(s_i_f_size / 4);
    read(s_i_f_, occ_s_table_.get(), s_i_f_size);
    load_r_s_ = true;
  }
  ////////// load all the stuff in memory
  // interval getting
  // 1 + ((x - 1) / y)
  // interval_b_ = std::ceil(static_cast<double>(s_f_size_) / (b_f_size_ / 2) *
  // 4);
  interval_b_ = 1 + ((s_f_size_ * 4 - 1) / (b_f_size_ / 2));
  interval_bb_ = 1 + ((s_f_size_ * 4 - 1) / b_f_size_);
  int chunk_size = sizeof(int32_t) * num_of_char_;
  int max_chunks_nums = (s_f_size_ - 2 * CHUNK_SIZE) / chunk_size;
  if (max_chunks_nums > 0) {
    step_s_size_ = ((s_f_size_ - 1) / max_chunks_nums) + 1;
  } else {
    step_s_size_ = s_f_size_ + 1;
  }
  if (mode == 'm') {
    std::cout << search_m_lg() << '\n';
  } else if (mode == 'a') {
    auto results = std::make_unique<size_t[]>(MAX_RESULT_NUM);
    auto count = search_a_lg(results);
    for (int i = 0; i != count; ++i) {
      std::cout << '[' << results[i] << ']' << '\n';
    }
  } else if (mode == 'r') {
    std::cout << search_r_lg() << '\n';
  } else if (mode == 'n') {
    auto result = std::make_unique<char[]>(MAX_SEARCH_PATTERN_LEN);
    auto count = search_n_lg(result);
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