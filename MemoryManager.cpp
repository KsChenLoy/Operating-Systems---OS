#include <iostream>
#include <utility>
#include "MemoryManager.h"

//Constructor; sets native word size (in bytes, for alignment) and default allocator for finding a memory hole.
MemoryManager::MemoryManager(unsigned wordSize, std::function<int(int, void *)> allocator) {
    wSize = wordSize;
    alloc = std::move(allocator);
    memLinkedlist = new LinkedList;
    bMap = new MyBitMap;
    valid = false;

}

//Releases all memory allocated by this object without leaking memory.
MemoryManager::~MemoryManager() {
    if (memLinkedlist)
        delete(memLinkedlist);

    if (valid) {
        delete[] memoryChunk;
    }
    if (bMap) {
        if (valid){
            bMap->clear();
        }
        delete [] bMap;
    }
}

//Instantiates block of requested size, no larger than 65536 words; cleans up previous block if applicable.
void MemoryManager::initialize(size_t sizeInWords) {

    if (valid && bMap){
        bMap->clear();
    }

    if(sizeInWords >= 0 && sizeInWords <= 65536){
        //need to keep a track of the memory chunk
        bMap->setMyBitmap(sizeInWords);
        memoryChunkCap = wSize *sizeInWords;
        memoryChunk = new char [memoryChunkCap];
        valid = true;
    }
    else
        std::cout << "Invalid sizeInWords" << endl;
}

//Releases memory block acquired during initialization, if any.
void MemoryManager::shutdown() {
    if (memoryChunk){
        delete[] memoryChunk;
    }

    memLinkedlist->clear();
    bMap->clear();
    valid = false;

}

//Allocates a memory using the allocator function. If no memory is available or size is invalid, returns nullptr.
void *MemoryManager::allocate(size_t sizeInBytes) {
    void * myList =  getList();
    int output = alloc((int)ceil((double)sizeInBytes / wSize), myList);
    delete static_cast<uint16_t*> (myList);

    if (output == -1) {
        return nullptr;
    }

    memLinkedlist->addList((int)ceil((double)sizeInBytes / wSize), output);
    bMap->append((int)ceil((double)sizeInBytes / wSize), output);

    //location
    return output * wSize + (char *) getMemoryStart();
}

//Frees the memory block within the memory manager so that it can be reused.
void MemoryManager::free(void *address) {
    char *index = (char *) address;
    int wordOffset = (int) (index - (char *) getMemoryStart()) / wSize;
    int length = memLinkedlist->getSizeOffset(wordOffset);
    memLinkedlist->deleteList(wordOffset);
    bMap->release(length, wordOffset);

}

//Changes the allocation algorithm to identifying the memory hole to use for allocation.
void MemoryManager::setAllocator(std::function<int(int, void *)> allocator) {
    alloc = allocator;
}

//Uses standard POSIX calls to write hole list to filename as text, returning -1 on error and 0 if successful.
//Format: "[START, LENGTH] - [START, LENGTH] ...", e.g., "[0, 10] - [12, 2] - [20, 6]"
int MemoryManager::dumpMemoryMap(char *filename) {
    string outputMap = bMap->getMemmap();
    char *buf = new char[outputMap.length()+1];
    strcpy(buf, outputMap.c_str());
    size_t bytes = strlen(buf);
    remove(filename);
    int myFile = open(filename, O_RDWR|O_CREAT|O_TRUNC, 777);
    if(myFile == -1)
        return -1;
    write(myFile, buf, bytes);
    close(myFile);
    delete[] buf;
    return 0;
}

//Returns a byte-stream of information (in decimal) about holes for use by the allocator function (little-Endian).
//Offset and length are in words. If no memory has been allocated, the function should return a NULL pointer.
void *MemoryManager::getList() {
    return !bMap ? nullptr : bMap->ToList();
}

//Returns a bit-stream of bits representing whether words are used (1) or free (0). The first two bytes are the
//size of the bMap (little-Endian); the rest is the bMap, word-wise.
void *MemoryManager::getBitmap() {
    return bMap->formatOutput();
}

//Returns the word size used for alignment.
unsigned MemoryManager::getWordSize() {
    return wSize;
}

//Returns the byte-wise memory address of the beginning of the memory block.
void *MemoryManager::getMemoryStart() {
    return memoryChunk == nullptr ? nullptr : memoryChunk;

}

//Returns the byte limit of the current memory block.
unsigned MemoryManager::getMemoryLimit() {
   // return bMap->getRange()*wSize;
    return memoryChunkCap;
}

//Algorithms

//Returns word offset of hole selected by the best fit memory allocation algorithm, and -1 if there is no fit.
int bestFit(int sizeInWords, void *list) {

    auto *hList = (uint16_t *) list;
    uint16_t hListrange = *hList++;

    if (sizeInWords > 0) {
        int minEmpty = pow(2,31)-1;
        int minIndex = -1;
        int i =1;
        while (i < hListrange * 2) {
            if (hList[i] >= sizeInWords && hList[i] < minEmpty) {
                minEmpty = hList[i];
                minIndex = hList[i - 1];
            }
            i+=2;
        }
        return minIndex;

    } else
        return -1;
}

//Returns word offset of hole selected by the worst fit memory allocation algorithm, and -1 if there is no fit.
int worstFit(int sizeInWords, void *list) {
    auto *hList = (uint16_t *) list;
    uint16_t hListrange = *hList++;

    if (sizeInWords > 0) {
        int maxEmpty = 0;
        int maxIndex = -1;
        int i = 1;
        while(i < hListrange * 2) {
            if (hList[i] >= sizeInWords && hList[i] > maxEmpty) {
                maxEmpty = hList[i];
                maxIndex = hList[i - 1];
            }
            i+=2;
        }
        return maxIndex;

    } else
        return -1;
}
/*

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
*/

/*
 unsigned int testMemoryLeaksNoShutdown();
 unsigned int testSimpleFirstFit();
 unsigned int testSimpleBestFit();
 unsigned int testComplexBestFit();
 unsigned int testNewAllocator();
 unsigned int testInvalidAllocate();
 unsigned int testRepeatedShutdown();
 unsigned int testMaxInitialization();
 unsigned int testGetters();
 unsigned int testReadingUsingGetMemoryStart();


 // helper functions
 std::string vectorToString(const std::vector<uint16_t>& vector);
 unsigned int testGetBitmap(MemoryManager& memoryManager, uint16_t correctBitmapLength, std::vector<uint8_t> correctBitmap);
 unsigned int testGetList(MemoryManager& memoryManager, uint16_t correctListLength, std::vector<uint16_t> correctList);
 unsigned int testGetWordSize(MemoryManager& memoryManager, size_t correctWordSize);
 unsigned int testGetMemoryLimit(MemoryManager& memoryManager, size_t correctMemoryLimit);
 unsigned int testDumpMemoryMap(MemoryManager& memoryManager, std::string fileName, std::string correctFileContents);

 int hopesAndDreamsAllocator(int sizeInWords, void* list)
 {
     static int wordOffsetIndex = 2;

     int largestWordOffsets[] = {-1,-1,-1};
     int largestWordOffsetsLength[] = {-1,-1,-1};


     uint16_t* holeList = static_cast<uint16_t*>(list);
     uint16_t holeListlength = *holeList++;

     for(uint16_t i = 1; i < (holeListlength) * 2; i += 2) {
         if(holeList[i] >= sizeInWords) {
             if(holeList[i] > largestWordOffsetsLength[2]) {
                 largestWordOffsets[0] = largestWordOffsets[1];
                 largestWordOffsets[1] = largestWordOffsets[2];
                 largestWordOffsets[2] = holeList[i - 1];

                 largestWordOffsetsLength[0] = largestWordOffsetsLength[1];
                 largestWordOffsetsLength[1] = largestWordOffsetsLength[2];
                 largestWordOffsetsLength[2] = holeList[i];

             }
             else if(holeList[i] > largestWordOffsetsLength[1]) {
                 largestWordOffsets[0] = largestWordOffsets[1];
                 largestWordOffsets[1] = holeList[i - 1];

                 largestWordOffsetsLength[0] = largestWordOffsetsLength[1];
                 largestWordOffsetsLength[1] = holeList[i];
             }
             else if(holeList[i] > largestWordOffsetsLength[0]) {
                 largestWordOffsets[0] = holeList[i - 1];
                 largestWordOffsetsLength[0] = holeList[i];
             }
         }
     }
     int wordOffset = largestWordOffsets[wordOffsetIndex];

     --wordOffsetIndex;
     if(wordOffsetIndex == -1) {
         wordOffsetIndex = 2;
     }

     return wordOffset;
 }

 int main()
 {
     unsigned int maxScore = 38;
     unsigned int score = 0;

     score += testMemoryLeaksNoShutdown(); // 0

     score += testSimpleFirstFit(); // 3
     std::cout << "Score: " << score << " / " <<  maxScore << std::endl;

     score += testSimpleBestFit(); // 3
     std::cout << "Score: " << score << " / " <<  maxScore << std::endl;

     score += testComplexBestFit(); // 13
     std::cout << "Score: " << score << " / " <<  maxScore << std::endl;

     score += testNewAllocator(); // 7
     std::cout << "Score: " << score << " / " <<  maxScore << std::endl;

     score += testInvalidAllocate(); // 1
     std::cout << "Score: " << score << " / " <<  maxScore << std::endl;

     score += testRepeatedShutdown(); // 3
     std::cout << "Score: " << score << " / " <<  maxScore << std::endl;

     score += testMaxInitialization(); // 1
     std::cout << "Score: " << score << " / " <<  maxScore << std::endl;

     score += testGetters(); // 2
     std::cout << "Score: " << score << " / " <<  maxScore << std::endl;

     score += 5 * testReadingUsingGetMemoryStart(); // 1 * 5

     std::cout << "Score: " << score << " / " <<  maxScore << std::endl;
 }



 unsigned int testMemoryLeaksNoShutdown()
 {
     unsigned int score = 0;

     unsigned int wordSize = 8;
     size_t numberOfWords = 100;

     MemoryManager memoryManager(wordSize, bestFit);
     memoryManager.initialize(numberOfWords);
     uint32_t* testArray1 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));
     uint32_t* testArray2 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));
     uint32_t* testArray3 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));
     uint32_t* testArray4 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));
     uint32_t* testArray5 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));
     memoryManager.free(testArray2);

     memoryManager.free(testArray4);
     memoryManager.free(testArray5);
     memoryManager.free(testArray1);
     memoryManager.free(testArray3);
     return score;
 }


 unsigned int testSimpleFirstFit()
 {
     std::cout << "Test Case: First Fit 1" << std::endl;

     unsigned int  wordSize = 8;
     size_t numberOfWords = 26;
     MemoryManager memoryManager(wordSize, bestFit);
     memoryManager.initialize(numberOfWords);

     std::cout << "allocating and freeing memory..." << std::endl;

     uint64_t* testArray1 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 10));
     uint64_t* testArray2 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 2));
     uint64_t* testArray3 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 2));
     uint64_t* testArray4 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 6));

     memoryManager.free(testArray1);
     memoryManager.free(testArray3);

     std::vector<uint8_t> correctBitmap{0x00,0xCC,0x0F,0x00};
     uint16_t correctBitmapLength = correctBitmap.size();

     std::vector<uint16_t> correctList = {0, 10, 12, 2, 20, 6};
     uint16_t correctListLength = correctList.size() * 2;

     std::cout << "Testing Memory Manager state after allocations and frees" << std::endl;


     unsigned int score = 0;
     score += testGetBitmap(memoryManager, correctBitmapLength, correctBitmap);
     score += testGetList(memoryManager, correctListLength, correctList);
     score += testDumpMemoryMap(memoryManager, "testSimpleFirstFit.txt", vectorToString(correctList));

     memoryManager.shutdown();

     return score;
 }


 unsigned int testSimpleBestFit()
 {
     std::cout << "Test Case: Best Fit 1" << std::endl;
     unsigned int wordSize = 8;
     size_t numberOfWords = 96;
     MemoryManager memoryManager(8, worstFit);
     memoryManager.initialize(numberOfWords);
     // allocate
     uint32_t* testArray1 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 4));
     uint32_t* testArray2 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 4));
     uint32_t* testArray3 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));
     uint32_t* testArray4 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 4));
     uint32_t* testArray5 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 4));
     uint32_t* testArray6 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));


     // free specific allocations to create holes
     memoryManager.free(testArray3);
     memoryManager.free(testArray5);
     // change allocator
     memoryManager.setAllocator(bestFit);

     std::cout << "allocating 4 words" << std::endl;

     uint32_t* testArray7 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 4));


     std::vector<uint8_t> correctBitmap{0x0F, 0xFE, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

     std::vector<uint16_t> correctList = {4,5,18,78};
     uint16_t correctListLength = correctList.size() * 2;

     std::string correctFileContents = vectorToString(correctList);

     unsigned int score = 0;

     std::cout << "Testing Memory Manager state after allocation" << std::endl;

     score += testGetBitmap(memoryManager, correctBitmap.size(), correctBitmap);
     score += testGetList(memoryManager, correctListLength, correctList);
     score += testDumpMemoryMap(memoryManager, "testSimpleBestFit.txt", vectorToString(correctList));

     memoryManager.shutdown();

     return score;
 }


 unsigned int testComplexBestFit()
 {
     std::cout << "Test Case: Best Fit 2" << std::endl;
     unsigned int wordSize = 4;
     size_t numberOfWords = 96;
     MemoryManager memoryManager(wordSize, worstFit);
     memoryManager.initialize(numberOfWords);

     uint32_t* testArray1 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));
     uint32_t* testArray2 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 4));
     uint32_t* testArray3 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));
     uint32_t* testArray4 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 3));
     uint32_t* testArray5 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));
     uint32_t* testArray6 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 2));
     uint32_t* testArray7 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));
     uint32_t* testArray8 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 1));
     uint32_t* testArray9 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));


     unsigned int score = 0;

     std::vector<uint8_t> correctBitmapBeforeFree{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00};

     std::vector<uint16_t> correctListBeforeFree = {60, 36};
     uint16_t correctListLengthBeforeFree = correctListBeforeFree.size() * 2;

     std::cout << "Testing Memory Manager state after initial allocations" << std::endl;

     score += testGetBitmap(memoryManager, correctBitmapBeforeFree.size(), correctBitmapBeforeFree);
     score += testGetList(memoryManager, correctListLengthBeforeFree, correctListBeforeFree);



     // free specific allocations to create holes
     memoryManager.free(testArray2);
     memoryManager.free(testArray4);
     memoryManager.free(testArray6);
     memoryManager.free(testArray8);

     std::vector<uint8_t> correctBitmapAfterFree{0xFF, 0xC3, 0xFF, 0xF8, 0x9F, 0xFF, 0xFD, 0x0F, 0x00, 0x00, 0x00, 0x00};

     std::vector<uint16_t> correctListAfterFree = {10, 4, 24, 3, 37, 2, 49, 1, 60, 36};
     uint16_t correctListLengthAfterFree = correctListAfterFree.size() * 2;

     std::cout << "Testing Memory Manager state after freeing specific areas " << std::endl;


     score += testGetBitmap(memoryManager, correctBitmapAfterFree.size(), correctBitmapAfterFree);
     score += testGetList(memoryManager, correctListLengthAfterFree, correctListAfterFree);


     // change allocator
     memoryManager.setAllocator(bestFit);

     std::cout << "Allocating 1 words" <<std::endl;


     uint32_t* testArray10 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 1));

     std::vector<uint8_t> correctBitmapAfter1{0xFF, 0xC3, 0xFF, 0xF8, 0x9F, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00};

     std::vector<uint16_t> correctListAfter1 = {10, 4, 24, 3, 37, 2, 60, 36};
     uint16_t correctListLengthAfter1 = correctListAfter1.size() * 2;

     std::cout << "Testing Memory Manager state\n" << std::endl;


     score += testGetBitmap(memoryManager, correctBitmapAfter1.size(), correctBitmapAfter1);
     score += testGetList(memoryManager, correctListLengthAfter1, correctListAfter1);

     std::cout << "Allocating 2 words" <<std::endl;


     uint32_t* testArray11 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 2));

     std::vector<uint8_t> correctBitmapAfter2{0xFF, 0xC3, 0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00};

     std::vector<uint16_t> correctListAfter2 = {10, 4, 24, 3, 60, 36};
     uint16_t correctListLengthAfter2 = correctListAfter2.size() * 2;

     std::cout << "Testing Memory Manager state\n" << std::endl;


     score += testGetBitmap(memoryManager, correctBitmapAfter2.size(), correctBitmapAfter2);
     score += testGetList(memoryManager, correctListLengthAfter2, correctListAfter2);

     std::cout << "Allocating 3 words" <<std::endl;


     uint32_t* testArray12 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 3));

     std::vector<uint8_t> correctBitmapAfter3{0xFF, 0xC3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00};

     std::vector<uint16_t> correctListAfter3 = {10, 4, 60, 36};
     uint16_t correctListLengthAfter3 = correctListAfter3.size() * 2;

     std::cout << "Testing Memory Manager state\n" << std::endl;


     score += testGetBitmap(memoryManager, correctBitmapAfter3.size(), correctBitmapAfter3);
     score += testGetList(memoryManager, correctListLengthAfter3, correctListAfter3);

     uint32_t* testArray13 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 4));

     std::cout << "Allocating 4 words" <<std::endl;


     std::vector<uint8_t> correctBitmapAfter4{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00};

     std::vector<uint16_t> correctListAfter4 = {60, 36};
     uint16_t correctListLengthAfter4 = correctListAfter4.size() * 2;

     std::cout << "Testing Memory Manager state\n" << std::endl;


     score += testGetBitmap(memoryManager, correctBitmapAfter4.size(), correctBitmapAfter4);
     score += testGetList(memoryManager, correctListLengthAfter4, correctListAfter4);
     score += testDumpMemoryMap(memoryManager, "testComplexBestFit.txt", vectorToString(correctListAfter4));

     memoryManager.shutdown();

     return score;
 }


 unsigned int testNewAllocator()
 {
     std::cout << "Test Case: New allocator";
     unsigned int wordSize = 8;
     size_t numberOfWords = 88;
     MemoryManager memoryManager(wordSize, worstFit);
     memoryManager.initialize(numberOfWords);


     uint64_t* testArray1 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 10));
     uint64_t* testArray2 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 13));
     uint64_t* testArray3 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 10));
     uint64_t* testArray4 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 8));
     uint64_t* testArray5 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 10));
     uint64_t* testArray6 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 4));
     uint64_t* testArray7 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 10));


     memoryManager.free(testArray2);
     memoryManager.free(testArray4);
     memoryManager.free(testArray6);

     unsigned int score = 0;

     // memoryManager.setAllocator(bestFit);
     memoryManager.setAllocator(hopesAndDreamsAllocator);

     // std::cout<<" SIZE: " <<  sizeof(uint64_t) * 10 << std::endl;

     std::cout << "Allocating 4 words" <<std::endl;

     uint64_t* testArray8 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 4));

     std::vector<uint8_t> correctBitmapAfter1{0xFF, 0x03, 0x80, 0xFF, 0x01, 0xFE, 0x87, 0xFF, 0x1F, 0x00, 0x00};

     std::vector<uint16_t> correctListAfter1 = {10, 13, 33, 8, 51, 4, 69, 19};
     uint16_t correctListLengthAfter1 = correctListAfter1.size() * 2;

     std::cout << "Testing Memory Manager state\n" << std::endl;

     score += testGetBitmap(memoryManager, correctBitmapAfter1.size(), correctBitmapAfter1);
     score += testGetList(memoryManager, correctListLengthAfter1, correctListAfter1);

     std::cout << "Allocating 4 words" <<std::endl;


     uint64_t* testArray9 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 4));

     std::vector<uint8_t> correctBitmapAfter2{0xFF, 0x3F, 0x80, 0xFF, 0x01, 0xFE, 0x87, 0xFF, 0x1F, 0x00, 0x00};

     std::vector<uint16_t> correctListAfter2 = {14, 9, 33, 8, 51, 4, 69, 19};
     uint16_t correctListLengthAfter2 = correctListAfter2.size() * 2;

     std::cout << "Testing Memory Manager state\n" << std::endl;


     score += testGetBitmap(memoryManager, correctBitmapAfter2.size(), correctBitmapAfter2);
     score += testGetList(memoryManager, correctListLengthAfter2, correctListAfter2);

     std::cout << "Allocating 4 words" <<std::endl;


     uint64_t* testArray10 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 4));

     std::vector<uint8_t> correctBitmapAfter3{0xFF, 0x3F, 0x80, 0xFF, 0x1F, 0xFE, 0x87, 0xFF, 0x1F, 0x00, 0x00};

     std::vector<uint16_t> correctListAfter3 = {14, 9, 37, 4, 51, 4, 69, 19};
     uint16_t correctListLengthAfter3 = correctListAfter3.size() * 2;

     std::cout << "Testing Memory Manager state\n" << std::endl;


     score += testGetBitmap(memoryManager, correctBitmapAfter3.size(), correctBitmapAfter3);
     score += testGetList(memoryManager, correctListLengthAfter3, correctListAfter3);
     score += testDumpMemoryMap(memoryManager, "testNewAllocator.txt", vectorToString(correctListAfter3));


     memoryManager.shutdown();

     return score;
 }


 unsigned int testInvalidAllocate()
 {
     std::cout << "Test Case: invalid allocation, over the allowed amount" << std::endl;
     unsigned int wordSize = 2;
     size_t numberOfWords = 20;
     MemoryManager memoryManager(wordSize, worstFit);
     memoryManager.initialize(numberOfWords);

     uint16_t* testArray1 = static_cast<uint16_t*>(memoryManager.allocate(sizeof(uint16_t) * numberOfWords));
     uint16_t* testArray2 = static_cast<uint16_t*>(memoryManager.allocate(sizeof(uint16_t) * 1));

     if(testArray2 == nullptr) {
         std::cout << "[CORRECT]\n" << std::endl;
         return 1;
     }
     else {
         std::cout << "[INCORRECT]\n" << std::endl;
         return 0;
     }
 }


 unsigned int testRepeatedShutdown()
 {
     std::cout << "Test Case: repeated shutdown\nGenerating Memory Manager..." << std::endl;
     unsigned int wordSize = 2;
     size_t numberOfWords1 = 10;
     MemoryManager memoryManager(wordSize, worstFit);
     memoryManager.initialize(numberOfWords1);

     uint16_t* testArray1 = static_cast<uint16_t*>(memoryManager.allocate(sizeof(uint16_t) * 1));
     uint16_t* testArray2 = static_cast<uint16_t*>(memoryManager.allocate(sizeof(uint16_t) * 2));
     uint16_t* testArray3 = static_cast<uint16_t*>(memoryManager.allocate(sizeof(uint16_t) * 1));
     uint16_t* testArray4 = static_cast<uint16_t*>(memoryManager.allocate(sizeof(uint16_t) * 2));
     uint16_t* testArray5 = static_cast<uint16_t*>(memoryManager.allocate(sizeof(uint16_t) * 1));

     memoryManager.free(testArray1);

     std::cout << "shutting down Memory Manager..." << std::endl;
     memoryManager.shutdown();

     std::cout << "initializing Memory Manager..." << std::endl;

     size_t numberOfWords2 = 20;

     memoryManager.initialize(numberOfWords2);

     uint16_t* testArray6 = static_cast<uint16_t*>(memoryManager.allocate(sizeof(uint16_t) * 1));
     uint16_t* testArray7 = static_cast<uint16_t*>(memoryManager.allocate(sizeof(uint16_t) * 2));
     uint16_t* testArray8 = static_cast<uint16_t*>(memoryManager.allocate(sizeof(uint16_t) * 1));
     uint16_t* testArray9 = static_cast<uint16_t*>(memoryManager.allocate(sizeof(uint16_t) * 2));
     uint16_t* testArray10 = static_cast<uint16_t*>(memoryManager.allocate(sizeof(uint16_t) * 1));

     memoryManager.free(testArray7);
     memoryManager.free(testArray8);


     std::vector<uint8_t> correctBitmap{0x71, 0x00, 0x00};

     std::vector<uint16_t> correctList = {1,3,7,13};
     uint16_t correctListLength = correctList.size() * 2;

     unsigned int score = 0;

     std::cout << "Testing Memory Manager state\n" << std::endl;
     score += testGetBitmap(memoryManager, correctBitmap.size(), correctBitmap);
     score += testGetList(memoryManager, correctListLength, correctList);
     score += testDumpMemoryMap(memoryManager, "testRepeatedShutdown.txt", vectorToString(correctList));


     memoryManager.shutdown();

     return score;

 }


 unsigned int testMaxInitialization()
 {
     std::cout << "Test Case: max initialization"  << std::endl;
     unsigned int wordSize = 8;
     size_t numberOfWords = 65535;
     MemoryManager memoryManager(wordSize, worstFit);
     memoryManager.initialize(numberOfWords);

     uint64_t* testArray1 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 32768));
     uint64_t* testArray2 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 32767));

     if(testArray1 && testArray2) {
         std::cout << "[CORRECT]\n" << std::endl;
         return 1;
     }
     else {
         std::cout << "[INCORRECT]\n" << std::endl;
         return 0;
     }
 }


 unsigned int testGetters()
 {
     // perfroms simpleFirstFit
     std::cout << "Test Case: testGetters, wordSize = 8, numberOfWords = 26" << std::endl;
     unsigned int wordSize = 8;
     size_t numberOfWords = 26;
     MemoryManager memoryManager(wordSize, bestFit);
     memoryManager.initialize(numberOfWords);

     uint64_t* testArray1 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 10));
     uint64_t* testArray2 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 2));
     uint64_t* testArray3 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 2));
     uint64_t* testArray4 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 6));

     memoryManager.free(testArray1);
     memoryManager.free(testArray3);

     unsigned int score = 0;
     score += testGetMemoryLimit(memoryManager, wordSize * numberOfWords);
     score += testGetWordSize(memoryManager, wordSize);

     memoryManager.shutdown();



     return score;
 }


 unsigned int testReadingUsingGetMemoryStart()
 {
     std::cout << "Test Case: Reading memory using GetMemoryStart" << std::endl;
     unsigned int wordSize = 8;
     size_t numberOfWords = 80;
     MemoryManager memoryManager(wordSize, worstFit);
     memoryManager.initialize(numberOfWords);

     std::vector<uint64_t> arrayContent{1, 12, 13, 14, 15, 2, 22, 23, 24, 25, 3, 32, 33, 34, 35, 4, 42, 43, 44, 45, 5, 52, 53, 54, 55};

     std::vector<uint64_t>::const_iterator arrayContentItr = arrayContent.begin();
     std::vector<uint64_t*> testArrays;

     testArrays.push_back(static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 5)));
     testArrays.push_back(static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 5)));
     testArrays.push_back(static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 5)));
     testArrays.push_back(static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 5)));
     testArrays.push_back(static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 5)));

     for(auto testArray: testArrays) {
         for(uint16_t i = 0; i < 5; ++i) {
             testArray[i] = *arrayContentItr++;
         }
     }

     unsigned int score = 0;

     arrayContentItr = arrayContent.begin();

     uint64_t* MemoryManagerContents = static_cast<uint64_t*>(memoryManager.getMemoryStart());

     for(auto content: arrayContent) {
         if(content != *MemoryManagerContents++) {
             std::cout << "Expected: " << content << std::endl;
             std::cout << "Got: "  << *(MemoryManagerContents - 1) << std::endl;
             std::cout << "[INCORRECT]\n" << std::endl;
             return score;
         }
     }

     ++score;

     memoryManager.shutdown();
     std::cout << "[CORRECT]\n" << std::endl;
     return score;
 }


 std::string vectorToString(const std::vector<uint16_t>& vector)
 {
     std::string vectorString = "";

     vectorString += "[" + std::to_string(vector[0]) + ", " + std::to_string(vector[1]);
     for (int i = 2; i < vector.size(); i +=2)
     {
         vectorString += "] - [" + std::to_string(vector[i]) + ", " + std::to_string(vector[i + 1]);
     }
     vectorString += "]";

     return vectorString;
 }


 unsigned int testGetBitmap(MemoryManager& memoryManager, uint16_t correctBitmapLength, std::vector<uint8_t> correctBitmap)
 {
     unsigned int score = 0;
     uint8_t* bitmap = static_cast<uint8_t*>(memoryManager.getBitmap());
     uint8_t* bitmapEntryPoint = bitmap;

     uint8_t lowerByte = *bitmap++;
     uint8_t higherByte = *bitmap++;
     uint16_t byteStreamLength = (higherByte << 8) | lowerByte;

     std::cout << "Testing getBitmap" << std::endl;
     std::cout << "Expected: " << std::endl;
     std::cout << "[0x" << std::hex << (int)correctBitmap[0];

     for(uint16_t i = 1; i < correctBitmap.size(); ++i) {
         std::cout << ", 0x" << std::hex << (int)correctBitmap[i];
     }
     std::cout << "]" << std::endl;


     std::cout << "\n";
     std::cout << "Got:"  << std::endl;
     std::cout << "[0x" << std::hex << (int)bitmap[0];
     for(uint16_t i = 1; i < byteStreamLength; ++i) {
         std::cout << ", 0x" << std::hex << (int)bitmap[i];
     }
     std::cout << "]" << std::endl;

     for(uint16_t i = 0; i < byteStreamLength; ++i) {
         if(bitmap[i] != correctBitmap[i]) {
             delete [] bitmapEntryPoint;
             std::cout << "[INCORRECT]\n" << std::endl;
             return score;
         }
     }

     ++score;
     delete [] bitmapEntryPoint;
     std::cout << "[CORRECT]\n" << std::endl;
     return score;
 }


 unsigned int testGetList(MemoryManager& memoryManager, uint16_t correctListLength, std::vector<uint16_t> correctList)
 {
     unsigned int score = 0;
     std::cout << std::dec << std::endl;
     uint16_t* list = static_cast<uint16_t*>(memoryManager.getList());
     uint16_t* listEntryPoint = list;

     uint16_t listLength = *list++;

     uint16_t bytesPerEntry = 2;
     uint16_t entriesInList = listLength * bytesPerEntry;

     std::cout << "Testing getList" << std::endl;
     std::cout << "Expected: " << std::endl;
     std::cout << std::dec << "[" << correctList[0];
     for(uint16_t i = 1; i < correctList.size(); ++i) {
         std::cout <<"] - [" << correctList[i];
     }
     std::cout << "]" << std::endl;


     std::cout << "\n";
     std::cout << "Got:"  << std::endl;
     std::cout << std::dec << "[" << list[0];
     for(uint16_t i = 1; i < entriesInList; ++i) {
         std::cout <<"] - [" << list[i];
     }
     std::cout << "]" << std::endl;

     for(uint16_t i = 0; i < entriesInList; ++i) {
         if(list[i] != correctList[i]) {
             delete [] listEntryPoint;
             std::cout << "[INCORRECT]\n" << std::endl;
             return score;
         }
     }

     ++score;

     delete [] listEntryPoint;
     std::cout << "[CORRECT]\n" << std::endl;
     return score;
 }

 unsigned int testGetWordSize(MemoryManager& memoryManager, size_t correctWordSize)
 {
     std::cout << "Testing getWordSize" << std::endl;
     std::cout << "Expected: " << correctWordSize << std::endl;
     std::cout << "Got:" << memoryManager.getWordSize() << std::endl;
     if(memoryManager.getWordSize() == correctWordSize) {
         std::cout << "[CORRECT]\n" << std::endl;
         return 1;
     }
     else {
         std::cout << "[INCORRECT]\n" << std::endl;
         return 0;
     }
 }

 unsigned int testGetMemoryLimit(MemoryManager& memoryManager, size_t correctMemoryLimit)
 {
     std::cout << "Testing getMemoryLimit" << std::endl;
     std::cout << "Expected: " << correctMemoryLimit << std::endl;
     std::cout << "Got:" << memoryManager.getMemoryLimit() << std::endl;
     if(memoryManager.getMemoryLimit() == correctMemoryLimit) {
         std::cout << "[CORRECT]\n" << std::endl;
         return 1;
     }
     else {
         std::cout << "[INCORRECT]\n" << std::endl;
         return 0;
     }
 }

 unsigned int testDumpMemoryMap(MemoryManager& memoryManager, std::string fileName, std::string correctFileContents)
 {
     std::cout << "Testing dumpMemoryMap" << std::endl;

     memoryManager.dumpMemoryMap((char*)fileName.c_str());
     std::ifstream testFile(fileName);
     if (testFile.is_open()) {
         std::string line;
         std::getline(testFile, line);
         testFile.close();
         std::cout << "Expected: " << correctFileContents << std::endl;
         std::cout << "Got:" << line << std::endl;
         if(line == correctFileContents) {
             std::cout << "[CORRECT]\n" << std::endl;
             return 1;
         }
         else {
             std::cout << "[INCORRECT]\n" << std::endl;
             return 0;
         }
     }
     return 0;
 }
 */
