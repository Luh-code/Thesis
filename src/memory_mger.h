#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <memory>
#include <iostream>
#include "logger.h"

// Credit to TheCherno (https://www.youtube.com/channel/UCQ-W1KE9EYfdxhL6S4twUNw) for the memory management code shown here

// struct MemoryMetrics
// {
//   size_t totalAllocated = 0;
//   size_t totalFreed = 0;
//   inline size_t currentUsage() { return this->totalAllocated - this->totalFreed; }
// };

// static MemoryMetrics memoryMetrics;

// inline void* operator new(size_t size)
// {
//   memoryMetrics.totalAllocated += size;
//   //std::cout << "Allocating " << size << " bytes\n";
//   return malloc(size);
// }

// inline void operator delete(void* p, size_t size)
// {
//   memoryMetrics.totalFreed+=size;
//   //std::cout << "Deleting " << size << " bytes\n";
//   free(p);
// }

// inline void printMemoryUsage()
// {
//   LOG_DEBUG("Current Heap Memory Usage: ", memoryMetrics.currentUsage(), " Bytes");
//   //std::cout << "Current Heap Memory Usage: " << memoryMetrics.currentUsage() << " Bytes\n";
// }

// ----------------------------------------------------

#endif // __MEMORY_H__