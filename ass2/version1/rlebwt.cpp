#include <cstring>
#include <iostream>

#include "worker.h"

int main(int argc, char *argv[]) {
  if (argc != 5 || strlen(argv[1]) != 2) {
    std::exit(1);
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