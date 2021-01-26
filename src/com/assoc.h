//
// Created by iclab on 1/26/21.
//

#ifndef TESTBED_ASSOC_H
#define TESTBED_ASSOC_H

#include "libcuckoo/cuckoohash_map.hh"

static libcuckoo::cuckoohash_map <std::string, std::string> hashTable;

void assoc_upsert(std::string key, std::string value);

std::string assoc_find(std::string);

#endif //TESTBED_ASSOC_H
