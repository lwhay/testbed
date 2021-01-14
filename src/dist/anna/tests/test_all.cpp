//  Copyright 2019 U.C. Berkeley RISE Lab
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdlib.h>
#include <vector>

#include "gtest/gtest.h"

#include "kvs.pb.h"
#include "misc.pb.h"
#include "replication.pb.h"
#include "types.hpp"
#include "utils/server_utils.hpp"

#include "kvs/server_handler_base.hpp"
#include "kvs/test_gossip_handler.hpp"
#include "kvs/test_node_depart_handler.hpp"
#include "kvs/test_node_join_handler.hpp"
#include "kvs/test_rep_factor_change_handler.hpp"
#include "kvs/test_rep_factor_response_handler.hpp"
#include "kvs/test_self_depart_handler.hpp"
#include "kvs/test_user_request_handler.hpp"

#include "include/lattices/test_bool_lattice.hpp"
#include "include/lattices/test_map_lattice.hpp"
#include "include/lattices/test_max_lattice.hpp"
#include "include/lattices/test_set_lattice.hpp"

unsigned kDefaultLocalReplication = 1;
unsigned kSelfTierId = kMemoryTierId;
unsigned kThreadNum = 1;
vector<unsigned> kSelfTierIdVector = {kSelfTierId};
map<TierId, TierMetadata> kTierMetadata = {};

unsigned kEbsThreadCount = 1;
unsigned kMemoryThreadCount = 1;
unsigned kRoutingThreadCount = 1;

int main(int argc, char *argv[]) {
  log->set_level(spdlog::level::info);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
