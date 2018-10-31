/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <string>
#include <map>
#include <cmath>

using namespace eosio;
using std::string;
using std::map;
using eosio::const_mem_fun;

//TODO store boidpower and send out int32-rounded boidpower
class testboidpower : public contract
{
  public:
    testboidpower(account_name self) : contract(self) {}

    // @abi action
    void create(account_name issuer, asset maximum_supply);

    void insert(account_name user, uint32_t boidpower);

  private:
    // @abi table accounts i64
    struct account
    {
      account_name key
      uint32_t boidpower; // TODO update boidpower daily
        
      uint64_t primary_key() const { return key.value; }
    };
    typedef eosio::multi_index<N(accounts), account> accounts;
};

EOSIO_ABI( testboidpower,(create)(insert))
