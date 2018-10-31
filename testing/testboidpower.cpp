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
  require_auth(_stake_account);

  auto el = accts.find(acct);
  accounts accts(_self, user);
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

//TODO add in send/receive boidpower requests
//     see https://developers.eos.io/eosio-home/docs/sending-an-inline-transaction-to-external-contract
