#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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
const int START_UP_SIZE = 4.5 * 1024 * 1024;
const int TOTAL_SPACE = 16 * 1024 * 1024;
const int WRITE_NUM_OF_CHUNK = 2000;

// 6.5MB
const int MAX_FREE_MEMORY = TOTAL_SPACE - START_UP_SIZE;
const int MAX_FREE_BITS = MAX_FREE_MEMORY * 8;
const int MAX_SEARCH_PATTERN_LEN = 5000;
const int MAX_RESULT_NUM = 5000;
const int FIRST_BIT = (1 << 7);
const size_t MAX_LEN_SEARCH = 520;

struct String {
  char* buffer_;
  size_t size_;
  String() : buffer_{new char[1]}, size_{0} { buffer_[0] = '\0'; };
  String(const char* string);
  String(const String& other);
  String(size_t i) : buffer_{new char[i + 1]} { buffer_[i] = '\0'; };
  String& operator=(const String& other);
  String& operator+=(const String& other);
  String& operator+=(const char* other);
  void resize(size_t size);
  friend String operator+(const String& lhs, const String& rhs) {
    String new_string{lhs};
    new_string += rhs;
    return new_string;
  }
  inline size_t size() const { return size_; };
  friend String operator+(const String& lhs, const char* rhs) {
    String new_string{lhs};
    new_string += rhs;
    return new_string;
  }
  friend String operator+(const char* lhs, const String& rhs) {
    String new_string{lhs};
    new_string += rhs;
    return new_string;
  }
  char& operator[](int i) { return buffer_[i]; }
  char operator[](int i) const { return buffer_[i]; }
  char* c_str() const { return buffer_; }
  ~String();
};

struct MyCArray {
  char* buffer_;
  MyCArray(char* ptr) : buffer_{ptr} {};
  MyCArray(int size) : buffer_{new char[size]} {};
  inline char& operator[](int i) { return buffer_[i]; }
  inline char operator[](int i) const { return buffer_[i]; }
  MyCArray& operator=(char* ptr) {
    delete[] buffer_;
    buffer_ = ptr;
    return *this;
  }
  inline char* get() { return buffer_; }
  ~MyCArray() { delete[] buffer_; };
};

struct MyIArray {
  int32_t* buffer_;
  MyIArray(int32_t* ptr) : buffer_{ptr} {};
  MyIArray(int32_t size) : buffer_{new int32_t[size]} {};
  inline int32_t& operator[](int32_t i) { return buffer_[i]; }
  inline int32_t operator[](int32_t i) const { return buffer_[i]; }
  MyIArray& operator=(int32_t* ptr) {
    delete[] buffer_;
    buffer_ = ptr;
    return *this;
  }
  inline int32_t* get() { return buffer_; }
  ~MyIArray() { delete[] buffer_; }
};

struct MySArray {
  size_t* buffer_;
  MySArray(size_t* ptr) : buffer_{ptr} {};
  MySArray(size_t size) : buffer_{new size_t[size]} {};
  inline size_t& operator[](size_t i) { return buffer_[i]; }
  inline size_t operator[](size_t i) const { return buffer_[i]; }
  MySArray& operator=(size_t* ptr) {
    delete[] buffer_;
    buffer_ = ptr;
    return *this;
  }
  inline size_t* get() { return buffer_; }
  ~MySArray() { delete[] buffer_; }
};

// in work_sm.cpp
int Rank_Sm_Md_Function(const MyCArray& buff, const MyIArray& occ, int index,
                        int step_record = 4);

int Occ_Function_Sm_Md(int c, int index_s, const MyIArray& occ,
                       const MyCArray& buff, const MyIArray& map_table,
                       int num_of_char, int step_size);

int binary_search_char(int index, int num_of_char, const MyIArray& c_table,
                       const MyIArray& rev_map);

int Select_Sm_Md(int index, const MyIArray& select_table,
                 const MyCArray& f_buff, int interval = 4);

int find_pre_1(int index, const MyCArray& buff);

int Compare(const void* a, const void* b);

int64_t pow(int base, int power);

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
  String filepath_;
  String filename_;
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
  String index_folder_;
  MyIArray c_table_{nullptr};
  MyIArray c_s_table_{nullptr};
  MyIArray mapping_table_{nullptr};
  char search_pattern_[MAX_LEN_SEARCH + 1] = {'\0'};
  int len_of_pattern_ = 0;
  int step_s_size_ = 0;
  char mode;
  MyCArray s_f_buff_{nullptr};
  MyCArray b_f_buff_{nullptr};
  MyCArray bb_f_buff_{nullptr};
  MyIArray occ_s_table_{nullptr};
  MyIArray occ_b_table_{nullptr};
  MyIArray occ_bb_table_{nullptr};
  MyIArray select_b_table_{nullptr};
  MyIArray select_bb_table_{nullptr};
  void Build_S_B_Index_LG(int s_f_, int b_f_);
  void Build_S_B_Index_SM(int s_f_, int b_f_);
  void Build_BB_Index_LG(String& bb_i_f_name);
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
  int search_a_sm(MySArray& results);
  int search_n_sm(MyCArray& result);
  int search_m_md();
  int search_r_md();
  int search_a_md(MySArray& results);
  int search_n_md(MyCArray& result);
  int search_m_lg();
  int search_r_lg();
  int search_a_lg(MySArray& results);
  int search_n_lg(MyCArray& result);
  int binary_search_s_sm(int pos_c, int c);
  int binary_search_lg(int pos_c, int c);
  int interval_b_ = 0, interval_bb_ = 0;
  bool load_s_ = false, load_b_ = false, load_s_b_ = false, load_r_b_ = false,
       load_bb_ = false, load_s_bb_ = false, load_r_s_ = false;
};

int main(int argc, char* argv[]) {
  if (argc != 5 || strlen(argv[1]) != 2) {
    exit(1);
  }
  RLEBWT rlebwt_worker{argv};
  rlebwt_worker.Build_S_B_Index();
  if (!rlebwt_worker.Existsbb()) {
    // If the bb not exists we can assure that the c_table and c_s_table is
    // loaded on the class, because the bb not exists means the index also not
    // exists and we will get the c_table and c_s_table from the building index
    // part.
    rlebwt_worker.Createbb();
  }
  rlebwt_worker.Build_BB_Index();
  rlebwt_worker.Search();
  return 0;
}

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
  char* new_buff = new char[len1 + 1];
  strcpy(new_buff, buffer_);
  new_buff[len1] = '\0';
  delete[] buffer_;
  buffer_ = new char[len + 1];
  strcpy(buffer_, new_buff);
  buffer_[len1] = '\0';
  delete[] new_buff;
  strcat(buffer_, other.buffer_);
  buffer_[len] = '\0';
  size_ = len;
  return *this;
}

String& String::operator+=(const char* other) {
  size_t len1 = size();
  size_t len2 = strlen(other);
  size_t len = len1 + len2;
  char* new_buff = new char[len1 + 1];
  strcpy(new_buff, buffer_);
  new_buff[len1] = '\0';
  delete[] buffer_;
  buffer_ = new char[len + 1];
  strcpy(buffer_, new_buff);
  buffer_[len1] = '\0';
  delete[] new_buff;
  strcat(buffer_, other);
  buffer_[len] = '\0';
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

static size_t fetch_new_bits(int file, MyCArray& r_buff2) {
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

static void write_bb(MyCArray& w_buff, char r_s_buff[MIN_READ_BUFF],
                     char r_b_buff[MIN_READ_BUFF], int bb_f, int s_f, int b_f,
                     int base, const MyIArray& c_table, size_t size) {
  int max = (8 * size - base > MAX_FREE_BITS) ? base + MAX_FREE_BITS : 8 * size;
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
  for (int i = 0; i != MAX_FREE_MEMORY; ++i) {
    w_buff[i] = -1;
  }
  int readByte = 0;
  int c = 0, byte = 0;
  int s_index = 0, w_byte = 0, w_bit = 0;
  bool done = false;
  int bit_index = 0;
  const int writing_size =
      (8 * size - base > MAX_FREE_BITS) ? MAX_FREE_MEMORY : size - (base / 8);
  const int writing_size_bits = writing_size * 8;
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
  close(bb_f);
}

void RLEBWT::Createbb() {
  const auto bb_f_n = filepath_ + ".bb";
  int bb_f = open(bb_f_n.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  MyCArray w_buff = new char[MAX_FREE_MEMORY];
  char r_s_buff[MIN_READ_BUFF];
  char r_b_buff[MIN_READ_BUFF];
  int repeat = ((b_f_size_ - 1) / MAX_FREE_MEMORY + 1);
  int base = 0;
  lseek(s_f_, 0, SEEK_SET);
  lseek(b_f_, 0, SEEK_SET);
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
  lseek(s_f_, 0, SEEK_SET);
  lseek(b_f_, 0, SEEK_SET);
  if (!large_file_) {
    Build_BB_Index_SM();
  } else {
    Build_BB_Index_LG(bb_i_f_name);
  }
}

static void build_s_b_index(
    MyCArray& r_buff, MyCArray& r_buff2, MyIArray& c_table, size_t size,
    MyIArray& w_buff, MyIArray& w_buff2, int s_f_index_f, int b_f_index_f,
    int step_size, int& real_chunks_nums, int& write_pos, int s_f,
    size_t& s_index, int& num_of_1, int& write_pos_b, size_t& s_f_r, int& c,
    int& step_count, const MyIArray& mapping_table, int num_of_char,
    MyIArray& tmp_table, MyIArray& w_buff3, int& write_pos_b_s,
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
  MyCArray r_buff = new char[READ_BUFF_SIZE];
  MyCArray r_buff2 = new char[READ_BUFF_SIZE];
  MyIArray w_buff = new int32_t[chunk_size * WRITE_NUM_OF_CHUNK];
  MyIArray w_buff2 = new int32_t[WRITE_BUFF_CHUNK];
  MyIArray w_buff3 = new int32_t[WRITE_BUFF_CHUNK];
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
  MyIArray temp_table = new int32_t[num_of_char_];
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

static void create_mapping(MyIArray& mapping_table, MyIArray& c_s_table,
                           int s_f, int& num_of_char_) {
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
}

int binary_search_char(int index, int num_of_char, const MyIArray& c_table,
                       const MyIArray& rev_map) {
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
  MyCArray r_buff = new char[READ_BUFF_SIZE];
  MyIArray w_buff = new int32_t[WRITE_BUFF_CHUNK];
  int interval = ((s_f_size_ * 4 - 1) / (b_f_size_) + 1);
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
  MyIArray temp_table = new int32_t[num_of_char_];
  for (int i = 0; i != num_of_char_; i++) {
    temp_table[i] = 0;
  }
  occ_s_table_ = new int32_t[real_chunks_nums * num_of_char_];
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

int Select_Sm_Md(int index, const MyIArray& select_table,
                 const MyCArray& f_buff, int interval) {
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

int Rank_Sm_Md_Function(const MyCArray& buff, const MyIArray& occ, int index,
                        int step_record) {
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

int Occ_Function_Sm_Md(int c, int index_s, const MyIArray& occ,
                       const MyCArray& buff, const MyIArray& map_table,
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
                                 mapping_table_, num_of_char_, step_s_size_);
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

int RLEBWT::search_a_sm(MySArray& results) {
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
                                 mapping_table_, num_of_char_, step_s_size_);
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
  int start = 1, end = s_f_size_, mid = 0, occ = 0;
  while (true) {
    mid = (start + end) / 2;
    if (start > end) return -1;
    occ = Occ_Function_Sm_Md(c, mid, occ_s_table_, s_f_buff_, mapping_table_,
                             num_of_char_, step_s_size_);
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

int RLEBWT::search_n_sm(MyCArray& result) {
  ++len_of_pattern_;
  search_pattern_[len_of_pattern_ - 1] = ']';
  int c = ']', curr_index = 0, count = 0,
      max_index = c_table_[NUMBER_OF_CHAR - 1];
  MyIArray reverse_map = new int32_t[num_of_char_];
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
    MySArray results = new size_t[MAX_RESULT_NUM];
    auto count = search_a_sm(results);
    for (int i = 0; i != count; ++i) {
      printf("[%zu]\n", results[i]);
    }
  } else if (mode == 'r') {
    printf("%d\n", search_r_sm());
  } else if (mode == 'n') {
    MyCArray result = new char[MAX_SEARCH_PATTERN_LEN];
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

int find_pre_1(int index, const MyCArray& buff) {
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
                            const MyIArray& select_table, const MyCArray& buff,
                            int interval) {
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

int RLEBWT::search_n_md(MyCArray& result) {
  ++len_of_pattern_;
  search_pattern_[len_of_pattern_ - 1] = ']';
  int c = ']', curr_index = 0, count = 0,
      max_index = c_table_[NUMBER_OF_CHAR - 1];
  MyIArray reverse_map = new int32_t[num_of_char_];
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

int RLEBWT::search_a_md(MySArray& results) {
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
          result += (cc - '0') * static_cast<size_t>(pow(10, result_len));
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
  qsort(results.get(), count, sizeof(size_t), Compare);
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
  lseek(s_f_, 0, SEEK_SET);
  lseek(b_f_, 0, SEEK_SET);
  lseek(bb_f_, 0, SEEK_SET);
  s_f_buff_ = new char[s_f_size_];
  read(s_f_, s_f_buff_.get(), s_f_size_);
  b_f_buff_ = new char[b_f_size_];
  read(b_f_, b_f_buff_.get(), b_f_size_);
  bb_f_buff_ = new char[b_f_size_];
  read(bb_f_, bb_f_buff_.get(), b_f_size_);
  int s_i_f = open(s_i_f_n.c_str(), O_RDONLY);
  int s_i_f_size = lseek(s_i_f, 0, SEEK_END);
  lseek(s_i_f, 0, SEEK_SET);
  occ_s_table_ = new int32_t[s_i_f_size / 4];
  read(s_i_f, occ_s_table_.get(), s_i_f_size);
  int b_i_f = open(b_i_f_n.c_str(), O_RDONLY);
  int b_i_f_size = lseek(b_i_f, 0, SEEK_END);
  lseek(b_i_f, 0, SEEK_SET);
  occ_b_table_ = new int32_t[b_i_f_size / 4];
  read(b_i_f, occ_b_table_.get(), b_i_f_size);
  int bs_i_f = open(bs_i_f_n.c_str(), O_RDONLY);
  int bs_i_f_size = lseek(bs_i_f, 0, SEEK_END);
  select_b_table_ = new int32_t[bs_i_f_size / 4];
  lseek(bs_i_f, 0, SEEK_SET);
  read(bs_i_f, select_b_table_.get(), bs_i_f_size);
  int bb_i_f = open(bb_i_f_n.c_str(), O_RDONLY);
  int bb_i_f_size = lseek(bb_i_f, 0, SEEK_END);
  lseek(bb_i_f, 0, SEEK_SET);
  select_bb_table_ = new int32_t[bb_i_f_size / 4];
  read(bb_i_f, select_bb_table_.get(), bb_i_f_size);
  close(s_i_f);
  close(b_i_f);
  close(bs_i_f);
  close(bb_i_f);
  // interval getting
  interval_b_ = ((s_f_size_ * 4 - 1) / (b_f_size_ / 2) + 1);
  interval_bb_ = ((s_f_size_ * 4 - 1) / (b_f_size_) + 1);
  int chunk_size = sizeof(int32_t) * num_of_char_;
  int max_chunks_nums = (s_f_size_ - 2 * CHUNK_SIZE) / chunk_size;
  if (max_chunks_nums > 0) {
    step_s_size_ = ((s_f_size_ - 1) / max_chunks_nums + 1);
  } else {
    step_s_size_ = s_f_size_ + 1;
  }
  if (mode == 'm') {
    printf("%d\n", search_m_md());
  } else if (mode == 'a') {
    MySArray results = new size_t[MAX_RESULT_NUM];
    auto count = search_a_md(results);
    for (int i = 0; i != count; ++i) {
      printf("[%zu]\n", results[i]);
    }
  } else if (mode == 'r') {
    printf("%d\n", search_r_md());
  } else if (mode == 'n') {
    MyCArray result = new char[MAX_SEARCH_PATTERN_LEN];
    auto count = search_n_md(result);
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

const int STEP_LG_B = 8;

static int rank_lg_function(const MyCArray& buff, const MyIArray& occ,
                            int index, int step_record, bool load_occ,
                            bool load_f, int b_f, int b_f_i) {
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
      char buff2[STEP_LG_B] = {'\0'};
      read(b_f, buff2, STEP_LG_B);
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

int Occ_Function_Lg(int c, int index_s, const MyIArray& occ,
                    const MyCArray& buff, const MyIArray& map_table,
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
      char buff2[CHUNK_SIZE] = {'\0'};
      read(s_f, buff2, CHUNK_SIZE);
      int end = index_s - start_s_place;
      for (int i = 0; i < end; ++i) {
        if (buff2[i] == c) ++occ_s;
      }
    }
  }
  return occ_s;
}

static int Select_Lg(int index, const MyIArray& select_table,
                     const MyCArray& f_buff, int interval, bool load_f,
                     bool load_i_f, int file, int index_file) {
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
      char r_buff[MIN_READ_BUFF];
      read(file, r_buff, MIN_READ_BUFF);
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
          read(file, r_buff, MIN_READ_BUFF);
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
    char r_buff[MIN_READ_BUFF];
    read(file, r_buff, MIN_READ_BUFF);
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
        read(file, r_buff, MIN_READ_BUFF);
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

int RLEBWT::search_a_lg(MySArray& results) {
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
          result += (cc - '0') * static_cast<size_t>(pow(10, result_len));
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
  qsort(results.get(), count, sizeof(size_t), Compare);
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
                               const MyIArray& select_table,
                               const MyCArray& buff, int interval, bool load_bb,
                               bool load_bb_i, int bb_f, int bb_i_f) {
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

int RLEBWT::search_n_lg(MyCArray& result) {
  ++len_of_pattern_;
  search_pattern_[len_of_pattern_ - 1] = ']';
  int c = ']', curr_index = 0, count = 0,
      max_index = c_table_[NUMBER_OF_CHAR - 1];
  MyIArray reverse_map = new int32_t[num_of_char_];
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
  lseek(s_f_, 0, SEEK_SET);
  lseek(b_f_, 0, SEEK_SET);
  lseek(bb_f_, 0, SEEK_SET);
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
    bb_f_buff_ = new char[b_f_size_];
    free_space -= b_f_size_;
    read(bb_f_, bb_f_buff_.get(), b_f_size_);
    load_bb_ = true;
  }
  if (mode == 'n') {
    // if it's a n mode load bb index first
    if (free_space >= static_cast<int>(bb_i_f_size)) {
      free_space -= bb_i_f_size;
      select_bb_table_ = new int32_t[bb_i_f_size / 4];
      read(bb_i_f_, select_bb_table_.get(), bb_i_f_size);
      load_s_bb_ = true;
    }
    if (free_space >= static_cast<int>(b_f_size_)) {
      free_space -= b_f_size_;
      b_f_buff_ = new char[b_f_size_];
      read(b_f_, b_f_buff_.get(), b_f_size_);
      load_b_ = true;
    }
  } else {
    if (free_space >= static_cast<int>(b_f_size_)) {
      free_space -= b_f_size_;
      b_f_buff_ = new char[b_f_size_];
      read(b_f_, b_f_buff_.get(), b_f_size_);
      load_b_ = true;
    }
    if (free_space >= static_cast<int>(bb_i_f_size)) {
      free_space -= bb_i_f_size;
      select_bb_table_ = new int32_t[bb_i_f_size / 4];
      read(bb_i_f_, select_bb_table_.get(), bb_i_f_size);
      load_s_bb_ = true;
    }
  }
  if (free_space >= static_cast<int>(b_i_f_size)) {
    free_space -= b_i_f_size;
    occ_b_table_ = new int32_t[b_i_f_size / 4];
    read(b_i_f_, occ_b_table_.get(), b_i_f_size);
    load_r_b_ = true;
  }
  if (free_space >= static_cast<int>(bs_i_f_size)) {
    free_space -= bs_i_f_size;
    select_b_table_ = new int32_t[bs_i_f_size / 4];
    read(bs_i_f_, select_b_table_.get(), bs_i_f_size);
    load_s_b_ = true;
  }
  if (free_space >= static_cast<int>(s_f_size_)) {
    free_space -= s_f_size_;
    s_f_buff_ = new char[s_f_size_];
    read(s_f_, s_f_buff_.get(), s_f_size_);
    load_s_ = true;
  }
  if (free_space >= static_cast<int>(s_i_f_size)) {
    free_space -= s_i_f_size;
    occ_s_table_ = new int32_t[s_i_f_size / 4];
    read(s_i_f_, occ_s_table_.get(), s_i_f_size);
    load_r_s_ = true;
  }
  ////////// load all the stuff in memory
  // interval getting
  // 1 + ((x - 1) / y)
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
    printf("%d\n", search_m_lg());
  } else if (mode == 'a') {
    MySArray results = new size_t[MAX_RESULT_NUM];
    auto count = search_a_lg(results);
    for (int i = 0; i != count; ++i) {
      printf("[%zu]\n", results[i]);
    }
  } else if (mode == 'r') {
    printf("%d\n", search_r_lg());
  } else if (mode == 'n') {
    MyCArray result = new char[MAX_SEARCH_PATTERN_LEN];
    auto count = search_n_lg(result);
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
  close(s_i_f_);
  close(b_i_f_);
  close(bs_i_f_);
  close(bb_i_f_);
}