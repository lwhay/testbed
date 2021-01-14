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

#include "route/routing_handlers.hpp"

TEST_F(RoutingHandlerTest, Seed) {
  EXPECT_EQ(global_hash_rings[Tier::MEMORY].size(), 3000);

  string serialized = seed_handler(log_, global_hash_rings);

  EXPECT_EQ(global_hash_rings[Tier::MEMORY].size(), 3000);

  ClusterMembership membership;
  membership.ParseFromString(serialized);

  EXPECT_EQ(membership.tiers_size(), 1);
  for (const auto &tier : membership.tiers()) {
    for (const auto &other : tier.servers()) {
      EXPECT_EQ(tier.tier_id(), Tier::MEMORY);
      EXPECT_EQ(other.private_ip(), ip);
      EXPECT_EQ(other.public_ip(), ip);
    }
  }
}
