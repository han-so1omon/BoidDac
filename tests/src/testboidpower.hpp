/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <string>
#include <map>
#include <set>
#include <cmath>

using namespace eosio;
using std::string;
using std::map;
using std::set;
using eosio::const_mem_fun;

//TODO store boidpower and send out int32-rounded boidpower
class testboidpower : public contract
{
  public:
    testboidpower(account_name self) : contract(self) {}

    // @abi action
    void create(account_name issuer, asset maximum_supply);

    // @abi action
    void insert(account_name user, uint32_t boidpower);

    // @abi action
    void sndnewbp(account_name req_acct);

  private:
    // @abi table accounts i64
    struct [[eosio::table]] account
    {
      account_name key;
      uint32_t boidpower; // TODO update boidpower daily
        
      uint64_t primary_key() const { return key; }
      EOSLIB_SERIALIZE (account, (key)(boidpower));

    };
    typedef eosio::multi_index<N(accounts), account> accounts;
};

EOSIO_ABI( testboidpower,(create)(insert)(sndnewbp))
