#
# Makefile for SharkBatch
#
# Dillon Bostwick for Tufts Univ.
#
# Note: check if stable for both clang++ and g++ because I'm not sure what Tufts is using
#

CXX      = clang++
CXXFLAGS = -Wall -Wextra
LDFLAGS  = -g
SRCS     = *.cpp
OBJS     = Scheduler.o main.o Job.o JobHashTable.o IntBST.o JobHeap.o

sharkbatch: ${OBJS}
	${CXX} ${LDFLAGS} -o sharkbatch ${OBJS}
	
clean:
	rm -rf sharkbatch *.o *~ *.dSYM core.*

# MUST SPECIFY A "LOCATION" WHEN PROVIDING!!!
provide:
	provide comp15 LOCATION *
	
	
Scheduler.o:  Scheduler.cpp Scheduler.h Job.h JobHeap.h JobHashTable.h IntBST.h
Job.o: Job.h Job.cpp IntBST.h
JobHashTable.o: JobHashTable.h JobHashTable.cpp Job.h
IntBST.o: IntBST.h IntBST.cpp
JobHeap.o: JobHeap.h JobHeap.cpp Job.h
main.o: main.cpp Scheduler.h Job.h IntBST.h JobHashTable.h JobHeap.h