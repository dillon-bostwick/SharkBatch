#
# Makefile for SharkBatch
#
# Dillon Bostwick for Tufts Univ.
#
# Note: check if stable for both clang++ and g++ because I'm not sure what Tufts is using
#

CXX      = clang++
CXXFLAGS = -Wall -Wextra
LDFLAGS  = -g -lncurses
SRCS     = *.cpp
OBJS     = Scheduler.o main.o Job.o JobHashTable.o JobQueue.o CursesHandler.o

sharkbatch: ${OBJS}
	${CXX} ${LDFLAGS} -o sharkbatch ${OBJS}
	
clean:
	rm -rf sharkbatch *.o *~ *.dSYM core.*

# MUST SPECIFY A LOCATION WHEN PROVIDING!!!
provide:
	provide comp15 LOCATION *
	
	
Scheduler.o: Scheduler.cpp Scheduler.h Job.h JobHashTable.h JobQueue.h CursesHandler.h
Job.o: Job.h Job.cpp JobHashTable.h
JobHashTable.o: JobHashTable.h JobHashTable.cpp Job.h
main.o: main.cpp Scheduler.h Job.h JobHashTable.h JobQueue.h CursesHandler.h
JobQueue.o: JobQueue.h JobQueue.cpp
CursesHandler.o: CursesHandler.h CursesHandler.cpp