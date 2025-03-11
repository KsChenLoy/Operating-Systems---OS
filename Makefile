libMemoryManager.a: MemoryManager.o MyBitMap.o LinkedList.o
	ar cr libMemoryManager.a MemoryManager.o MyBitMap.o LinkedList.o

MemoryManager.o: MemoryManager.cpp
	c++ -std=c++17 -Wall -g -c MemoryManager.cpp -o MemoryManager.o

MyBitMap.o: MyBitMap.cpp
	c++ -std=c++17 -Wall -g -c MyBitMap.cpp -o MyBitMap.o

LinkedList.o: LinkedList.cpp
	c++ -std=c++17 -Wall -g -c LinkedList.cpp -o LinkedList.o
