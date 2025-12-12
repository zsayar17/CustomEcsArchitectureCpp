#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <cstddef>
#include <bitset>
#include <vector>
#include <map>

#define CHUNK_SIZE 1024 * 16 //L1 CACHE SIZE
#define MAX_COMPONENTS 64

typedef std::size_t EntityID;
typedef std::size_t ComponentTypeID;
typedef std::bitset<MAX_COMPONENTS> Signature;

#endif
