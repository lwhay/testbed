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

string seed_handler(logger log, GlobalRingMap &global_hash_rings) {
  log->info("Received a global hash ring membership request.");

  ClusterMembership membership;

  for (const auto &pair : global_hash_rings) {
    Tier tid = pair.first;
    GlobalHashRing hash_ring = pair.second;

    ClusterMembership_TierMembership *tier = membership.add_tiers();
    tier->set_tier_id(tid);

    for (const ServerThread &st : hash_ring.get_unique_servers()) {
      auto server = tier->add_servers();
      server->set_private_ip(st.private_ip());
      server->set_public_ip(st.public_ip());
    }
  }

  string serialized;
  membership.SerializeToString(&serialized);
  return serialized;
}
