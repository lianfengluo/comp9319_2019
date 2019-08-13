CC = gcc-9
# CFLAGS = -std=c++14 -O3 -Wall -Wextra -pedantic
CFLAGS = -lsupc++ -fno-rtti -fno-exceptions -lc -O3 -std=c++17 
# CFLAGS = -lsupc++ -fno-rtti -fno-exceptions -lc -g -std=c++14 
#CFLAGS = -lsupc++ -fno-rtti -fno-exceptions -lc -O3 -std=c++14 -lgcc_s 
ODIR = ./
#rlebwt: rlebwt.cpp worker.o worker_lg.o worker_md.o worker_sm.o
#	$(CC) $^ -o $@ $(CFLAGS) 

rlebwt: rlebwt.cpp worker.o worker_lg.o worker_md.o worker_sm.o
	$(CC)  $^ -o $@ $(CFLAGS) 


worker.o: worker.cpp 
	$(CC) -c $^ -o $@ $(CFLAGS) 

worker_lg.o: worker_lg.cpp
	$(CC) -c $^ -o $@ $(CFLAGS)

worker_md.o: worker_md.cpp
	$(CC) -c $^ -o $@ $(CFLAGS)

worker_sm.o: worker_sm.cpp
	$(CC) -c $^ -o $@ $(CFLAGS)
	
clean:
	rm -f $(ODIR)/*.o