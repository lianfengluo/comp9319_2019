#include "worker.h"

String::String(const char* string) {
  size_ = strlen(string);
  buffer_ = new char[size_ + 1];
  strcpy(buffer_, string);
  buffer_[size_] = '\0';
}

String::String(const String& other) {
  size_ = other.size();
  buffer_ = new char[size_ + 1];
  strcpy(buffer_, other.buffer_);
  buffer_[size_] = '\0';
}

String& String::operator+=(const String& other) {
  size_t len1 = size();
  size_t len2 = other.size();
  size_t len = len1 + len2;
  char* new_buff = new char[len + 1];
  new_buff[len] = '\0';
  strcpy(new_buff, buffer_);
  delete[] buffer_;
  strcat(new_buff, other.buffer_);
  buffer_ = new_buff;
  size_ = len;
  return *this;
}

String& String::operator+=(const char* other) {
  size_t len1 = size();
  size_t len2 = strlen(other);
  size_t len = len1 + len2;
  char* new_buff = new char[len + 1];
  new_buff[len] = '\0';
  strcpy(new_buff, buffer_);
  delete[] buffer_;
  strcat(new_buff, other);
  buffer_ = new_buff;
  size_ = len;
  return *this;
}

String& String::operator=(const String& other) {
  delete[] buffer_;
  buffer_ = new char[other.size() + 1];
  strcpy(buffer_, other.buffer_);
  size_ = other.size();
  buffer_[size_] = '\0';
  return *this;
}

void String::resize(size_t size) {
  delete[] buffer_;
  buffer_ = new char[size + 1];
  size_ = size;
  buffer_[size] = '\0';
}

String::~String() { delete[] buffer_; }

int64_t pow(int base, int power) {
  int64_t result = 1;
  for (int i = 0; i < power; ++i) {
    result *= base;
  }
  return result;
}

int Compare(const void* a, const void* b) {
  return (*(size_t*)a - *(size_t*)b);
}

static bool IsPathExist(const String& s) {
  struct stat buffer;
  return (stat(s.c_str(), &buffer) == 0);
}

static size_t fetch_new_bits(int file, MyArray<char>& r_buff2) {
  return read(file, r_buff2.get(), READ_BUFF_SIZE);
}

RLEBWT::RLEBWT(char* argv[])
    : filepath_{argv[2]},
      c_table_{new int32_t[NUMBER_OF_CHAR]},
      c_s_table_{new int32_t[NUMBER_OF_CHAR]},
      mapping_table_{new int32_t[NUMBER_OF_CHAR]} {
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
    filename_.resize(filepath_.size() - (i + 1));
    for (size_t j = 0; j != filepath_.size() - (i + 1); ++j) {
      filename_[j] = filepath_[i + 1 + j];
    }
  }
  String s_f_n{filepath_}, b_f_n{filepath_};
  s_f_n += ".s";
  b_f_n += ".b";
  if (access(s_f_n.c_str(), F_OK) == -1 || access(b_f_n.c_str(), F_OK) == -1) {
    fprintf(stderr, "file not exists\n");
    exit(1);
  }
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
    bb_f_ = open(bb_f_name.c_str(), O_RDONLY);
    return true;
  } else {
    return false;
  }
}

static void write_bb(MyArray<char>& w_buff, char r_s_buff[MIN_READ_BUFF],
                     char r_b_buff[MIN_READ_BUFF], int bb_f, int s_f, int b_f,
                     int64_t base, const MyArray<int32_t>& c_table, size_t size,
                     size_t buff_size) {
  int64_t max =
      (8 * size - base > buff_size * 8) ? base + (buff_size * 8) : 8 * size;
  bool processing_table[NUMBER_OF_CHAR];
  int start_write[NUMBER_OF_CHAR];
  int write_pos[NUMBER_OF_CHAR];
  int count_c[NUMBER_OF_CHAR];
  for (int i = 0; i != NUMBER_OF_CHAR; ++i) {
    processing_table[i] = false;
    count_c[i] = 0;
    start_write[i] = 0;
    write_pos[i] = 0;
  }
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
  int s_f_r = read(s_f, r_s_buff, MIN_READ_BUFF);
  for (int i = 0; i != buff_size; ++i) {
    w_buff[i] = -1;
  }
  int readByte = 0;
  int c = 0, byte = 0;
  int s_index = 0, w_byte = 0, w_bit = 0;
  bool done = false;
  int bit_index = 0;
  const int writing_size =
      (8 * size - base > buff_size * 8) ? buff_size : size - (base / 8);
  const int64_t writing_size_bits = writing_size * 8;
  while ((readByte = read(b_f, r_b_buff, MIN_READ_BUFF))) {
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
            s_f_r = read(s_f, r_s_buff, MIN_READ_BUFF);
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
  size_t buff_size =
      (b_f_size_ > MAX_FREE_MEMORY) ? MAX_FREE_MEMORY : b_f_size_;
  MyArray<char> w_buff = new char[buff_size];
  char r_s_buff[MIN_READ_BUFF];
  char r_b_buff[MIN_READ_BUFF];
  int repeat = ((b_f_size_ - 1) / buff_size + 1);
  int64_t base = 0;
  lseek(s_f_, 0, SEEK_SET);
  lseek(b_f_, 0, SEEK_SET);
  write_bb(w_buff, r_s_buff, r_b_buff, bb_f, s_f_, b_f_, base, c_table_,
           b_f_size_, buff_size);
  for (int i = 1; i < repeat; ++i) {
    lseek(s_f_, 0, SEEK_SET);
    lseek(b_f_, 0, SEEK_SET);
    base += MAX_FREE_BITS;
    write_bb(w_buff, r_s_buff, r_b_buff, bb_f, s_f_, b_f_, base, c_table_,
             b_f_size_, buff_size);
  }
  close(bb_f);
  bb_f_ = open(bb_f_n.c_str(), O_RDONLY);
}

void RLEBWT::Build_BB_Index() {
  auto bb_i_f_name = index_folder_ + "/" + filename_ + ".bb";
  // #ifndef DEBUG_
  if (IsPathExist(bb_i_f_name)) return;
  // #endif
  lseek(bb_f_, 0, SEEK_SET);
  if (!large_file_) {
    Build_BB_Index_SM();
  } else {
    Build_BB_Index_LG(bb_i_f_name);
  }
}

static void build_s_b_index(
    MyArray<char>& r_buff, MyArray<char>& r_buff2, MyArray<int32_t>& c_table,
    size_t size, MyArray<int32_t>& w_buff, MyArray<int32_t>& w_buff2,
    int s_f_index_f, int b_f_index_f, int step_size, int& real_chunks_nums,
    int& write_pos, int s_f, size_t& s_index, int& num_of_1, int& write_pos_b,
    size_t& s_f_r, int& c, int& step_count,
    const MyArray<int32_t>& mapping_table, int num_of_char,
    MyArray<int32_t>& tmp_table, MyArray<int32_t>& w_buff3, int& write_pos_b_s,
    int b_f_select_index_f, int interval, int& bit_index, int chunk_size) {
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
  const int interval = ((s_f_size_ * 4 - 1) / (b_f_size_ / 2) + 1);
  int c_table_f = open(c_table_f_n.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  int chunk_size = sizeof(int32_t) * num_of_char_;
  MyArray<char> r_buff = new char[READ_BUFF_SIZE];
  MyArray<char> r_buff2 = new char[READ_BUFF_SIZE];
  MyArray<int32_t> w_buff = new int32_t[chunk_size * WRITE_NUM_OF_CHUNK];
  MyArray<int32_t> w_buff2 = new int32_t[WRITE_BUFF_CHUNK];
  MyArray<int32_t> w_buff3 = new int32_t[WRITE_BUFF_CHUNK];
  // giving the chunk for the c_table and c_s_table. Therefore, I minus 2.
  int max_chunks_nums = (s_f_size_ - 2 * CHUNK_SIZE) / chunk_size;
  int step_size = 0, bit_index = 0;
  if (max_chunks_nums > 0) {
    step_size = ((s_f_size_ - 1) / max_chunks_nums + 1);
  } else {
    step_size = s_f_size_ + 1;
  }
  int real_chunks_nums = s_f_size_ / step_size;
  int write_pos = 0, write_pos_b = 0, num_of_1 = 0, step_count = 0,
      write_pos_b_s = 0;
  size_t s_f_r = fetch_new_bits(s_f, r_buff), s_index = 0;
  int c = 0;
  MyArray<int32_t> temp_table = new int32_t[num_of_char_];
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

static void create_mapping(MyArray<int32_t>& mapping_table,
                           MyArray<int32_t>& c_s_table, int s_f,
                           int& num_of_char_) {
  char r_buff[MIN_READ_BUFF];
  int readByte;
  int count = 0;
  int c;
  while ((readByte = read(s_f, r_buff, MIN_READ_BUFF))) {
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
                       const MyArray<int32_t>& c_table,
                       const MyArray<int32_t>& rev_map) {
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

void RLEBWT::Build_BB_Index_LG(String& bb_i_f_name) {
  int bb_i_f = open(bb_i_f_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  int readByte = 0;
  MyArray<char> r_buff = new char[READ_BUFF_SIZE];
  MyArray<int32_t> w_buff = new int32_t[WRITE_BUFF_CHUNK];
  const int interval = ((s_f_size_ * 4 - 1) / (b_f_size_) + 1);
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