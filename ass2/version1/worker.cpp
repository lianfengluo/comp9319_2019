#include "worker.h"

static bool IsPathExist(const std::string& s) {
  struct stat buffer;
  return (stat(s.c_str(), &buffer) == 0);
}

static size_t fetch_new_bits(int file, std::unique_ptr<char[]>& r_buff2) {
  return read(file, r_buff2.get(), READ_BUFF_SIZE);
}

RLEBWT::RLEBWT(char* argv[])
    : filepath_{argv[2]},
      c_table_{std::make_unique<int32_t[]>(NUMBER_OF_CHAR)},
      c_s_table_{std::make_unique<int32_t[]>(NUMBER_OF_CHAR)},
      mapping_table_{std::make_unique<int32_t[]>(NUMBER_OF_CHAR)} {
  for (int j = 0; j != NUMBER_OF_CHAR; ++j) {
    c_table_[j] = 0;
    c_s_table_[j] = 0;
    mapping_table_[j] = -1;
  }
  mode = argv[1][1];
  index_folder_ = argv[3];
  strcpy(search_pattern_, argv[4]);
  len_of_pattern_ = strlen(search_pattern_);
  int i = 0;
  for (i = filepath_.size() - 1; i != -1; --i) {
    if (filepath_[i] == '/') break;
  }
  if (i == 0) {
    filename_ = filepath_;
  } else {
    filename_.resize(filepath_.size() - (i + 1), ' ');
    for (size_t j = 0; j != filepath_.size() - (i + 1); ++j) {
      filename_[j] = filepath_[i + 1 + j];
    }
  }
  std::string s_f_n{filepath_}, b_f_n{filepath_};
  s_f_n.reserve(s_f_n.size() + 4);
  b_f_n.reserve(b_f_n.size() + 4);
  s_f_n += ".s";
  b_f_n += ".b";
  s_f_ = open(s_f_n.c_str(), O_RDONLY);
  b_f_ = open(b_f_n.c_str(), O_RDONLY);
  s_f_size_ = lseek(s_f_, 0, SEEK_END);
  lseek(s_f_, 0, SEEK_SET);
  b_f_size_ = lseek(b_f_, 0, SEEK_END);
  lseek(b_f_, 0, SEEK_SET);
  if (b_f_size_ > BUILD_SIZE && s_f_size_ > 1024) {
    large_file_ = true;
    if (b_f_size_ * 4 + s_f_size_ * 2 <= MAX_FREE_MEMORY) {
      medium_file_ = true;
    }
  }
}

RLEBWT::~RLEBWT() {
  close(s_f_);
  close(b_f_);
  close(bb_f_);
}

void RLEBWT::Sum_C_Table() {
  for (size_t i = 1; i != NUMBER_OF_CHAR; ++i) {
    c_table_[i] += c_table_[i - 1];
    c_s_table_[i] += c_s_table_[i - 1];
  }
}

bool RLEBWT::Existsbb() {
  auto bb_f_name = filepath_ + ".bb";
  if (IsPathExist(bb_f_name)) {
    // std::cout << "test\n";
    bb_f_ = open(bb_f_name.c_str(), O_RDONLY);
    return true;
  } else {
    return false;
  }
}

static void write_bb(std::unique_ptr<char[]>& w_buff,
                     std::array<char, MIN_READ_BUFF>& r_s_buff,
                     std::array<char, MIN_READ_BUFF>& r_b_buff, int bb_f,
                     int s_f, int b_f, int base,
                     const std::unique_ptr<int32_t[]>& c_table, size_t size) {
  int max = (8 * size - base > MAX_FREE_BITS) ? base + MAX_FREE_BITS : 8 * size;
  std::array<bool, NUMBER_OF_CHAR> processing_table;
  std::array<int, NUMBER_OF_CHAR> start_write;
  std::array<int64_t, NUMBER_OF_CHAR> write_pos;
  std::array<int, NUMBER_OF_CHAR> count_c;
  processing_table.fill(false);
  count_c.fill(0);
  start_write.fill(0);
  write_pos.fill(0);
  // getting the char we need to consider
  for (int i = 1; i != NUMBER_OF_CHAR; ++i) {
    if (c_table[i - 1] == c_table[i]) {
      continue;
    } else {
      if (c_table[i - 1] <= base && c_table[i] > base) {
        processing_table[i] = true;
        start_write[i] = base - c_table[i - 1];
      } else if (c_table[i - 1] > base && c_table[i - 1] < max) {
        processing_table[i] = true;
        write_pos[i] = c_table[i - 1] - base;
      }
    }
  }
  int s_f_r = read(s_f, r_s_buff.data(), MIN_READ_BUFF);
  // fill all the write buff to 1;
  std::fill(w_buff.get(), w_buff.get() + MAX_FREE_MEMORY, -1);
  int readByte = 0;
  int c = 0, byte = 0;
  int s_index = 0, w_byte = 0, w_bit = 0;
  bool done = false;
  int bit_index = 0;
  const int writing_size =
      (8 * size - base > MAX_FREE_BITS) ? MAX_FREE_MEMORY : size - (base / 8);
  const int writing_size_bits = writing_size * 8;
  while ((readByte = read(b_f, r_b_buff.data(), MIN_READ_BUFF))) {
    for (int i = 0; i != readByte; ++i) {
      byte = r_b_buff[i];
      for (int j = 0; j != 8; ++j) {
        if (((byte << j) & FIRST_BIT) == 0) {
          if (processing_table[c]) {
            if (count_c[c] >= start_write[c]) {
              w_byte = write_pos[c] / 8;
              w_bit = 7 - (write_pos[c] % 8);
              if (w_byte < writing_size) {
                w_buff[w_byte] &= ~(1UL << w_bit);
                ++write_pos[c];
                ++bit_index;
                if (bit_index == writing_size_bits) {
                  done = true;
                  break;
                }
              }
            }
            ++count_c[c];
          }
        } else {
          if (done) break;
          c = r_s_buff[s_index++];
          if (processing_table[c]) {
            if (count_c[c] >= start_write[c]) {
              w_byte = write_pos[c] / 8;
              if (w_byte < writing_size) {
                ++write_pos[c];
                ++bit_index;
                if (bit_index == writing_size_bits) {
                  done = true;
                  break;
                }
              }
            }
            ++count_c[c];
          }
          if (s_index == s_f_r) {
            s_index = 0;
            s_f_r = read(s_f, r_s_buff.data(), MIN_READ_BUFF);
            if (s_f_r == 0) {
              done = true;
            }
          }
        }
      }
      if (done) break;
    }
  }
  write(bb_f, w_buff.get(), writing_size);
}

void RLEBWT::Createbb() {
  const auto bb_f_n = filepath_ + ".bb";
  int bb_f = open(bb_f_n.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  // auto w_buff = std::make_unique<char[]>(MAX_FREE_MEMORY);
  auto w_buff = std::make_unique<char[]>(MAX_FREE_MEMORY);
  std::array<char, MIN_READ_BUFF> r_s_buff;
  std::array<char, MIN_READ_BUFF> r_b_buff;
  int repeat = std::ceil(static_cast<double>(b_f_size_) / MAX_FREE_MEMORY);
  int base = 0;
  write_bb(w_buff, r_s_buff, r_b_buff, bb_f, s_f_, b_f_, base, c_table_,
           b_f_size_);
  for (int i = 1; i < repeat; ++i) {
    lseek(s_f_, 0, SEEK_SET);
    lseek(b_f_, 0, SEEK_SET);
    base += MAX_FREE_BITS;
    write_bb(w_buff, r_s_buff, r_b_buff, bb_f, s_f_, b_f_, base, c_table_,
             b_f_size_);
  }
  close(bb_f);
  bb_f_ = open(bb_f_n.c_str(), O_RDONLY);
}

void RLEBWT::Build_BB_Index() {
  auto bb_i_f_name = index_folder_ + "/" + filename_ + ".bb";
  // #ifndef DEBUG_
  if (IsPathExist(bb_i_f_name)) return;
  // #endif
  if (!large_file_) {
    Build_BB_Index_SM();
  } else {
    Build_BB_Index_LG(bb_i_f_name);
  }
}

static void build_s_b_index(
    std::unique_ptr<char[]>& r_buff, std::unique_ptr<char[]>& r_buff2,
    std::unique_ptr<int32_t[]>& c_table, size_t size,
    std::unique_ptr<int32_t[]>& w_buff, std::unique_ptr<int32_t[]>& w_buff2,
    int s_f_index_f, int b_f_index_f, int step_size, int& real_chunks_nums,
    int& write_pos, int s_f, size_t& s_index, int& num_of_1, int& write_pos_b,
    size_t& s_f_r, int& c, int& step_count,
    const std::unique_ptr<int32_t[]>& mapping_table, int num_of_char,
    std::unique_ptr<int32_t[]>& tmp_table, std::unique_ptr<int32_t[]>& w_buff3,
    int& write_pos_b_s, int b_f_select_index_f, int interval, int& bit_index,
    int chunk_size) {
  int byte = 0;
  int count_8 = 0;
  bool done = false;
  for (size_t i = 0; i != size; ++i) {
    byte = r_buff2[i];
    for (int j = 0; j != 8; ++j) {
      if (((byte << j) & FIRST_BIT) == 0) {
        // 0-bit
        ++c_table[c];
      } else {
        // 1-bit
        if (done) return;
        c = r_buff[s_index++];
        ++tmp_table[mapping_table[c]];
        ++c_table[c];
        ++num_of_1;
        ++step_count;
        if (num_of_1 % interval == 0) {
          w_buff3[write_pos_b_s] = bit_index;
          ++write_pos_b_s;
          if (write_pos_b_s == WRITE_BUFF_CHUNK) {
            write_pos_b_s = 0;
            write(b_f_select_index_f, w_buff3.get(), WRITE_BUFF_SIZE);
          }
        }
        if (step_size == step_count) {
          step_count = 0;
          auto base = write_pos * num_of_char;
          for (int k = 0; k != num_of_char; ++k) {
            w_buff[base + k] = tmp_table[k];
          }
          ++write_pos;
          if (--real_chunks_nums == 0) {
            write(s_f_index_f, w_buff.get(), write_pos * chunk_size);
            write_pos = 0;
          } else if (write_pos == WRITE_NUM_OF_CHUNK) {
            write_pos = 0;
            write(s_f_index_f, w_buff.get(), chunk_size * WRITE_NUM_OF_CHUNK);
          }
        }
        if (s_index == s_f_r) {
          s_index = 0;
          s_f_r = fetch_new_bits(s_f, r_buff);
          if (s_f_r == 0) {
            w_buff2[write_pos_b++] = num_of_1;
            done = true;
          }
        }
      }
      ++bit_index;
    }
    if (++count_8 == 8) {
      count_8 = 0;
      w_buff2[write_pos_b] = num_of_1;
      if (++write_pos_b == WRITE_BUFF_CHUNK) {
        write_pos_b = 0;
        write(b_f_index_f, w_buff2.get(), WRITE_BUFF_SIZE);
      }
    }
  }
}

void RLEBWT::Build_S_B_Index_LG(int s_f, int b_f) {
  size_t readByte = 0;
  auto s_f_index_f_n = index_folder_ + "/" + filename_ + ".s";
  auto b_f_index_f_n = index_folder_ + "/" + filename_ + ".b";
  auto b_f_select_index_f_n = index_folder_ + "/" + filename_ + ".bs";
  auto c_table_f_n = index_folder_ + "/" + filename_ + ".ct";
  int s_f_index_f =
      open(s_f_index_f_n.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  int b_f_index_f =
      open(b_f_index_f_n.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  int b_f_select_index_f =
      open(b_f_select_index_f_n.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  const int interval =
      std::ceil(static_cast<double>(s_f_size_) / (b_f_size_ / 2) * 4);
  int c_table_f = open(c_table_f_n.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  int chunk_size = sizeof(int32_t) * num_of_char_;
  auto r_buff = std::make_unique<char[]>(READ_BUFF_SIZE);
  auto r_buff2 = std::make_unique<char[]>(READ_BUFF_SIZE);
  auto w_buff = std::make_unique<int32_t[]>(chunk_size * WRITE_NUM_OF_CHUNK);
  auto w_buff2 = std::make_unique<int32_t[]>(WRITE_BUFF_CHUNK);
  auto w_buff3 = std::make_unique<int32_t[]>(WRITE_BUFF_CHUNK);
  // giving the chunk for the c_table and c_s_table. Therefore, I minus 2.
  int max_chunks_nums = (s_f_size_ - 2 * CHUNK_SIZE) / chunk_size;
  int step_size = 0, bit_index = 0;
  if (max_chunks_nums > 0) {
    step_size = std::ceil(static_cast<double>(s_f_size_) / max_chunks_nums);
  } else {
    step_size = s_f_size_ + 1;
  }
  int real_chunks_nums = s_f_size_ / step_size;
  int write_pos = 0, write_pos_b = 0, num_of_1 = 0, step_count = 0,
      write_pos_b_s = 0;
  size_t s_f_r = fetch_new_bits(s_f, r_buff), s_index = 0;
  int c = 0;
  auto temp_table = std::make_unique<int32_t[]>(num_of_char_);
  while ((readByte = read(b_f, r_buff2.get(), READ_BUFF_SIZE))) {
    build_s_b_index(r_buff, r_buff2, c_table_, readByte, w_buff, w_buff2,
                    s_f_index_f, b_f_index_f, step_size, real_chunks_nums,
                    write_pos, s_f, s_index, num_of_1, write_pos_b, s_f_r, c,
                    step_count, mapping_table_, num_of_char_, temp_table,
                    w_buff3, write_pos_b_s, b_f_select_index_f, interval,
                    bit_index, chunk_size);
  }
  if (write_pos != 0) {
    write(s_f_index_f, w_buff.get(), write_pos * chunk_size);
  }
  if (write_pos_b != 0) {
    write(b_f_index_f, w_buff2.get(), write_pos_b * sizeof(int32_t));
  }
  if (write_pos_b != 0) {
    write(b_f_select_index_f, w_buff3.get(), write_pos_b_s * sizeof(int32_t));
  }
  write(c_table_f, c_s_table_.get(), CHUNK_SIZE);
  write(c_table_f, c_table_.get(), CHUNK_SIZE);
  close(c_table_f);
  close(b_f_index_f);
  close(b_f_select_index_f);
  close(s_f_index_f);
  Sum_C_Table();
}

static void create_mapping(std::unique_ptr<int32_t[]>& mapping_table,
                           std::unique_ptr<int32_t[]>& c_s_table, int s_f,
                           int& num_of_char_) {
  std::array<char, MIN_READ_BUFF> r_buff;
  int readByte;
  int count = 0;
  int c;
  while ((readByte = read(s_f, r_buff.data(), MIN_READ_BUFF))) {
    for (int i = 0; i != readByte; ++i) {
      c = r_buff[i];
      ++c_s_table[c];
    }
  }
  for (int i = 1; i != NUMBER_OF_CHAR; ++i) {
    if (c_s_table[i]) {
      mapping_table[i] = count++;
    }
  }
  num_of_char_ = count;
  lseek(s_f, 0, SEEK_SET);
}

void RLEBWT::Build_S_B_Index() {
  if (IsPathExist(index_folder_ + "/" + filename_ + ".s")) return;
  load_index_ = true;
  create_mapping(mapping_table_, c_s_table_, s_f_, num_of_char_);
  if (large_file_) {
    Build_S_B_Index_LG(s_f_, b_f_);
  } else {
    Build_S_B_Index_SM(s_f_, b_f_);
  }
  lseek(s_f_, 0, SEEK_SET);
  lseek(b_f_, 0, SEEK_SET);
}

int binary_search_char(int index, int num_of_char,
                       const std::unique_ptr<int32_t[]>& c_table,
                       const std::unique_ptr<int32_t[]>& rev_map) {
  int start = 0, end = num_of_char - 1, mid = 0;
  while (true) {
    if (start > end) break;
    mid = (start + end) / 2;
    if (mid == 0) {
      if (0 <= index && index < c_table[rev_map[0]]) {
        return rev_map[0];
      }
      start = 1;
    } else {
      int low = c_table[rev_map[mid - 1]], high = c_table[rev_map[mid]];
      if (low <= index && index < high) {
        return rev_map[mid];
      } else if (index < low) {
        end = mid - 1;
      } else {
        start = mid + 1;
      }
    }
  }
  return -1;
}

void RLEBWT::Build_BB_Index_LG(std::string& bb_i_f_name) {
  int bb_i_f = open(bb_i_f_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  int readByte = 0;
  auto r_buff = std::make_unique<char[]>(READ_BUFF_SIZE);
  auto w_buff = std::make_unique<int32_t[]>(WRITE_BUFF_CHUNK);
  int interval = std::ceil(static_cast<double>(s_f_size_) / b_f_size_ * 4);
  int write_pos = 0, num_of_1 = 0, bit_index = 0;
  int c = 0;
  int max = c_s_table_[NUMBER_OF_CHAR - 1];
  while ((readByte = read(bb_f_, r_buff.get(), READ_BUFF_SIZE))) {
    for (int i = 0; i != readByte; ++i) {
      c = r_buff[i];
      for (int j = 0; j != 8; ++j) {
        if (((c << j) & FIRST_BIT)) {
          ++num_of_1;
          if (num_of_1 % interval == 0) {
            w_buff[write_pos] = bit_index;
            if (++write_pos == WRITE_BUFF_CHUNK) {
              write(bb_i_f, w_buff.get(), WRITE_BUFF_CHUNK * 4);
              write_pos = 0;
            }
          }
          if (max == num_of_1) break;
        }
        ++bit_index;
      }
    }
  }
  if (write_pos != 0) {
    write(bb_i_f, w_buff.get(), write_pos * sizeof(int32_t));
  }
  close(bb_i_f);
  lseek(bb_f_, 0, SEEK_SET);
}

void RLEBWT::Search() {
  if (large_file_) {
    if (medium_file_) {
      Search_Medium();
    } else {
      Search_Lg();
    }
  } else {
    Search_Sm();
  }
}