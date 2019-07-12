CC = g++
CFLAGS = -std=c++14 -O3 -Wall -Wextra -pedantic
ODIR = ./

rlebwt: rlebwt.cpp worker.o worker_sm.o worker_md.o worker_lg.o
	$(CC) $(CFLAGS) $^ -o $@ 

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
