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

#ifndef KVS_INCLUDE_MONITOR_POLICIES_HPP_
#define KVS_INCLUDE_MONITOR_POLICIES_HPP_

#include "hash_ring.hpp"

extern bool kEnableTiering;
extern bool kEnableElasticity;
extern bool kEnableSelectiveRep;

void storage_policy(logger log, GlobalRingMap &global_hash_rings,
                    TimePoint &grace_start, SummaryStats &ss,
                    unsigned &memory_node_count, unsigned &ebs_node_count,
                    unsigned &new_memory_count, unsigned &new_ebs_count,
                    bool &removing_ebs_node, Address management_ip,
                    MonitoringThread &mt,
                    map<Address, unsigned> &departing_node_map,
                    SocketCache &pushers);

void movement_policy(logger log, GlobalRingMap &global_hash_rings,
                     LocalRingMap &local_hash_rings, TimePoint &grace_start,
                     SummaryStats &ss, unsigned &memory_node_count,
                     unsigned &ebs_node_count, unsigned &new_memory_count,
                     unsigned &new_ebs_count, Address management_ip,
                     map<Key, KeyReplication> &key_replication_map,
                     map<Key, unsigned> &key_access_summary,
                     map<Key, unsigned> &key_size, MonitoringThread &mt,
                     SocketCache &pushers, zmq::socket_t &response_puller,
                     vector<Address> &routing_ips, unsigned &rid);

void slo_policy(logger log, GlobalRingMap &global_hash_rings,
                LocalRingMap &local_hash_rings, TimePoint &grace_start,
                SummaryStats &ss, unsigned &memory_node_count,
                unsigned &new_memory_count, bool &removing_memory_node,
                Address management_ip,
                map<Key, KeyReplication> &key_replication_map,
                map<Key, unsigned> &key_access_summary, MonitoringThread &mt,
                map<Address, unsigned> &departing_node_map,
                SocketCache &pushers, zmq::socket_t &response_puller,
                vector<Address> &routing_ips, unsigned &rid,
                map<Key, std::pair<double, unsigned>> &latency_miss_ratio_map);

#endif // KVS_INCLUDE_MONITOR_POLICIES_HPP_
