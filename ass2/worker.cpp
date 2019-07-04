#include "worker.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>

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
const int CHUNK_SIZE = 127 * sizeof(int32_t);
// 9.5MB
const int START_UP_SIZE = 9.5 * 1024 * 1024;
const int TOTAL_SPACE = 16 * 1024 * 1024;
const int WRITE_NUM_OF_CHUNK = 2000;
const int NUMBER_OF_CHAR = 127;
// 6.5MB
const int MAX_FREE_MEMORY = TOTAL_SPACE - START_UP_SIZE;
const int MAX_FREE_BITS = MAX_FREE_MEMORY * 8;
const int _4 = sizeof(int32_t);
const int FIRST_BIT = (1 << 7);

static bool IsPathExist(const std::string& s) {
  struct stat buffer;
  return (stat(s.c_str(), &buffer) == 0);
}

RLEBWT::RLEBWT(char* argv[])
    : filepath_{argv[2]},
      c_table_{std::make_unique<int32_t[]>(NUMBER_OF_CHAR)},
      c_s_table_{std::make_unique<int32_t[]>(NUMBER_OF_CHAR)},
      mapping_table_{std::make_unique<int32_t[]>(NUMBER_OF_CHAR)} {
  for (int j = 0; j != NUMBER_OF_CHAR; ++j) {
    c_table_[j] = 0;
    c_s_table_[j] = 0;
    mapping_table_[j] = 0;
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
    if (b_f_size_ * 2 + s_f_size_ <= MAX_FREE_MEMORY) {
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
    bb_f_ = open(bb_f_name.c_str(), O_RDONLY);
    return true;
  } else {
    return false;
  }
}

static size_t fetch_new_bits(int file, std::unique_ptr<char[]>& r_buff2) {
  return read(file, r_buff2.get(), READ_BUFF_SIZE);
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

void RLEBWT::Build_BB_Index_LG(std::string& bb_i_f_name) {
  int bb_i_f = open(bb_i_f_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  int readByte = 0;
  auto r_buff = std::make_unique<char[]>(READ_BUFF_SIZE);
  auto w_buff = std::make_unique<int32_t[]>(WRITE_BUFF_CHUNK);
  int interval = std::ceil(static_cast<double>(s_f_size_) / b_f_size_ * 4);
  int write_pos = 0, num_of_1 = 0, bit_index = 0;
  int c;
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
              write(bb_i_f, w_buff.get(), WRITE_BUFF_SIZE);
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
  close(bb_f_);
}

void RLEBWT::Build_BB_Index_SM() {
  bb_f_buff_ = std::make_unique<char[]>(b_f_size_);
  int write_pos = 0, bit_index = 0, num_of_1 = 0;
  int c;
  select_bb_table_ =
      std::make_unique<int32_t[]>(s_f_size_ * sizeof(int32_t) * 2);
  int max = c_s_table_[NUMBER_OF_CHAR - 1];
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
        if (max == num_of_1) break;
      }
      ++bit_index;
    }
  }
}

void RLEBWT::Build_BB_Index() {
  auto bb_i_f_name = index_folder_ + "/" + filename_ + ".bb";
#ifndef DEBUG_
  if (IsPathExist(bb_i_f_name)) return;
#endif
  if (large_file_) {
    Build_BB_Index_LG(bb_i_f_name);
  } else {
    Build_BB_Index_SM();
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

void RLEBWT::Build_S_B_Index_SM(int s_f, int b_f) {
  if (s_f_size_ == 0) throw std::runtime_error("Input file is empty.");
  int space_can_be_use =
      MAX_FREE_MEMORY - s_f_size_ - (sizeof(int32_t) * s_f_size_ * 2) * 2;
  int chunk_size = num_of_char_ * sizeof(int32_t);
  int max_chunks_nums = space_can_be_use / chunk_size;
  int step_size =
      (s_f_size_ / max_chunks_nums == 0)
          ? 1
          : (std::ceil(static_cast<double>(s_f_size_) / max_chunks_nums));
  step_s_size_ = step_size;
  int real_chunks_nums = s_f_size_ / step_size;
  s_f_buff_ = std::make_unique<char[]>(s_f_size_);
  b_f_buff_ = std::make_unique<char[]>(b_f_size_);
  auto temp_table = std::make_unique<int32_t[]>(num_of_char_);
  std::fill(temp_table.get(), temp_table.get() + num_of_char_, 0);
  occ_s_table_ = std::make_unique<int32_t[]>(real_chunks_nums * num_of_char_);
  occ_b_table_ = std::make_unique<int32_t[]>(b_f_size_);
  // every 4's 1 build the index
  select_b_table_ =
      std::make_unique<int32_t[]>(s_f_size_ * sizeof(int32_t) * 2);
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
      occ_b_table_[write_pos_b++] = num_of_1;
      break;
    }
    if (++count_4 == _4) {
      count_4 = 0;
      occ_b_table_[write_pos_b++] = num_of_1;
    }
  }
  // #ifdef DEBUG_
  //   for (int i = 0; i != NUMBER_OF_CHAR; ++i) {
  //     if (c_table_[i] != 0) {
  //       std::cout << static_cast<char>(i) << " : " << c_table_[i] << '\n';
  //     }
  //   }
  // #endif
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
#ifndef DEBUG_
  if (IsPathExist(index_folder_ + "/" + filename_ + ".s")) return;
#endif
  create_mapping(mapping_table_, c_s_table_, s_f_, num_of_char_);
  if (large_file_) {
    Build_S_B_Index_LG(s_f_, b_f_);
  } else {
    Build_S_B_Index_SM(s_f_, b_f_);
  }
  lseek(s_f_, 0, SEEK_SET);
  lseek(b_f_, 0, SEEK_SET);
}

void RLEBWT::Search_Lg() {
  std::string s_f_i_name = index_folder_ + "/" + filename_ + ".s";
  std::string ct_f_name = index_folder_ + "/" + filename_ + ".ct";
  std::string b_f_i_name = index_folder_ + "/" + filename_ + ".b";
  std::string bs_f_i_name = index_folder_ + "/" + filename_ + ".bs";
  std::string bb_f_i_name = index_folder_ + "/" + filename_ + ".bb";
  int s_f_i = open(s_f_i_name.c_str(), O_RDONLY);
  int ct_f = open(ct_f_name.c_str(), O_RDONLY);
  int b_f_i = open(b_f_i_name.c_str(), O_RDONLY);
  int bs_f_i = open(bs_f_i_name.c_str(), O_RDONLY);
  int bb_f_i = open(bb_f_i_name.c_str(), O_RDONLY);
  // int s_f_i_size = lseek(s_f_i, 0, SEEK_END);
  // lseek(s_f_i, 0, SEEK_SET);
}

static int Select_Sm(int index, const std::unique_ptr<int[]>& select_table,
                     const std::unique_ptr<char[]>& f_buff) {
  int pos = index / 4;
  int rest_of_1 = index % 4;
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

static int rank_sm_function(const std::unique_ptr<char[]>& buff,
                            const std::unique_ptr<int32_t[]>& occ, int index) {
  // bit position, every 32 bits(4 byte * 8) get one record;
  int location_b_chunk = index / 32;
  int b_start_place = 0, occ_b = 0, byte = 0;
  if (location_b_chunk > 0) {
    occ_b = occ[location_b_chunk - 1];
    b_start_place = location_b_chunk * 32;
  }
  int b_start_byte_pos = (b_start_place) / 8;
  int end_b_byte_pos = index / 8;
  int end_b_bit_pos = index % 8;
  if (index % 32 != 0) {
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
  }
  return occ_b;
}

static int Occ_Function_Sm(int c, int index_s, std::unique_ptr<int32_t[]>& occ,
                           std::unique_ptr<char[]>& buff,
                           std::unique_ptr<int32_t[]>& map_table,
                           int num_of_char, int step_size) {
  int chunk_s_location = index_s / step_size;
  int occ_s = 0;
  int start_s_place = 0;
  if (chunk_s_location > 0) {
    occ_s = occ[(chunk_s_location - 1) * num_of_char + map_table[c]];
    start_s_place = chunk_s_location * step_size;
  }
  for (int i = start_s_place; i < index_s; ++i) {
    if (buff[i] == c) ++occ_s;
  }
  return occ_s;
}

void RLEBWT::get_lower_uppder_bound(int& lower_bound, int& upper_bound, int c) {
  int occurrence_1 = 0, occurrence_2 = 0;
  int search_index = len_of_pattern_ - 1, padding_0_1 = 0, padding_0_2 = 0,
      pre_lower = 0, pre_upper = 0, upper_index = 0, lower_index = 0,
      max_index = c_table_[NUMBER_OF_CHAR - 1];
#ifdef DEBUG_
  // std::string
  // s{"1111223445[[[[[[[]]]]]]]aaaaaaabbbbbbbddddeggiiiinnnnnnnnn"};
  // std::string s{"12[[]]aaaabbnnn"};
  // for (int i = 1; i <= 11; ++i) {
  //   std::cout << Select_Sm(i, select_bb_table_, bb_f_buff_) << '\n';
  // }
#endif
  while (search_index > 0) {
    c = search_pattern_[--search_index];
    pre_lower = lower_bound;
    pre_upper = upper_bound;
    lower_index = rank_sm_function(b_f_buff_, occ_b_table_, lower_bound + 1);
    upper_index = rank_sm_function(b_f_buff_, occ_b_table_, upper_bound + 1);
    occurrence_1 = Occ_Function_Sm(c, lower_index - 1, occ_s_table_, s_f_buff_,
                                   mapping_table_, num_of_char_, step_s_size_);
    occurrence_2 = Occ_Function_Sm(c, upper_index, occ_s_table_, s_f_buff_,
                                   mapping_table_, num_of_char_, step_s_size_);
    lower_bound = c_s_table_[c - 1] + occurrence_1 + 1;
    upper_bound = c_s_table_[c - 1] + occurrence_2;
    if (lower_bound > upper_bound) {
      // std::cout << "ttest\n";
      break;
    }
    // std::cout << s_f_buff_[lower_index] << '\n';
    // bool lower = (s_f_buff_[lower_index - 1] == c);
    // bool upper = (s_f_buff_[upper_index - 1] == c);
    padding_0_1 =
        (s_f_buff_[lower_index - 1] == c)
            ? pre_lower - Select_Sm(lower_index, select_b_table_, b_f_buff_)
            : 0;
    padding_0_2 =
        (s_f_buff_[upper_index - 1] == c)
            ? pre_upper - Select_Sm(upper_index, select_b_table_, b_f_buff_)
            : 0;
    if (s_f_buff_[lower_index - 1] != c) {
      // if (lower_bound == static_cast<int>(s_f_size_)) {
      //   lower_bound = max_index - 1;
      // } else {
      lower_bound = Select_Sm(lower_bound, select_bb_table_, bb_f_buff_);
      // }
    } else {
      lower_bound =
          Select_Sm(lower_bound, select_bb_table_, bb_f_buff_) + padding_0_1;
    }
    if (s_f_buff_[upper_index - 1] != c) {
      if (upper_bound == static_cast<int>(s_f_size_)) {
        upper_bound = max_index - 1;
      } else {
        upper_bound =
            Select_Sm(upper_bound + 1, select_bb_table_, bb_f_buff_) - 1;
      }
    } else {
      upper_bound =
          Select_Sm(upper_bound, select_bb_table_, bb_f_buff_) + padding_0_2;
    }
    // #ifdef DEBUG_
    //     for (int i = lower_bound; i <= upper_bound; ++i) {
    //       std::cout << s[i];
    //     }
    //     std::cout << '\n';
    // #endif
  }
}

int RLEBWT::search_m_sm() {
  int c = search_pattern_[len_of_pattern_ - 1], count = 0;
  int lower_bound = c_table_[c - 1], upper_bound = c_table_[c] - 1;
  get_lower_uppder_bound(lower_bound, upper_bound, c);
  if (upper_bound >= lower_bound) count = upper_bound - lower_bound + 1;
  return count;
}

void RLEBWT::Search_Sm() {
  if (mode == 'm') {
    std::cout << search_m_sm() << '\n';
  } else if (mode == 'a') {
  } else if (mode == 'r') {
  } else if (mode == 'n') {
  } else {
    std::cerr << "Invalid search flag.\n";
  }
}

void RLEBWT::Search() {
  if (large_file_) {
    Search_Lg();
  } else {
    if (medium_file_) {
      ;
    } else {
      Search_Sm();
    }
  }
}