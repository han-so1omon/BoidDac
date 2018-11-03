/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
*/

#include "testboidpower.hpp"
#include <math.h>

void testboidpower::create(account_name issuer, asset maximum_supply)
{
  require_auth(_self);
}

void testboidpower::insert(account_name user, uint32_t boidpower) {
  require_auth(user);

  accounts accts(_self, user);
  auto el = accts.find(user);
  if (el != accts.end()) {
    accts.modify(el,user, [&](auto &a) {
          a.boidpower = boidpower;
    });
  } else {
    accts.emplace(user, [&](auto &a) {
          a.boidpower = boidpower;
    });
  }
}

//     see https://developers.eos.io/eosio-home/docs/sending-an-inline-transaction-to-external-contract
void testboidpower::send_boidpower_update(account_name requester, set<account_name> req_accts) {
  require_auth("boid.stake"_n);
  accounts accts(_self,requester);

  map<account_name, uint32_t> bp_table;
  for (auto it = req_accts.begin(); it != req_accts.end(); it++) {
    auto el = accts.find(*it);
    if (el != accts.end()) {
      bp_table[*it] = el->boidpower;
    }
  }

  action(
    permission_level{get_self(),"active"_n},
    "boid.stake"_n,
    "update_boidpower"_n,
    std::make_tuple(get_self(),bp_table)
  ).send();
}
