/**
 *  @file
 *  @copyright TODO
 *  @brief Boidpower oracle
 *  @author errcsool
 */

#pragma once

#include <string>
#include <set>
#include <cmath>

#include <eosio/eosio.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/dispatcher.hpp>
#include <eosio/contract.hpp>
#include <eosio/time.hpp>
#include <eosio/system.hpp>
#include <eosio/asset.hpp>
#include <eosio/action.hpp>
#include <eosio/symbol.hpp>
#include <eosio/name.hpp>

#include "boidcommon/defines.hpp"

using namespace eosio;
using std::string;
using std::vector;
using std::set;
using eosio::const_mem_fun;
using eosio::check;
using eosio::microseconds;
using eosio::time_point;

#define BOINCPOWER 0
#define RAVENPOWER 1

/*
Power rating table for devices
  Scope : account
  Primary key : device name
  Row contains vector of validators and power reports
  Current validator takes over ram payment for row
  Rows get overwritten every round
Update power rating table
Validator table
Register validator
Validator weighting consensus
Update boidpower on next round start
*/

//TODO add in delvalidator
//TODO add in delregistrar
//TODO change updateboinc|raven to updaterate and add type parameter
//TODO add in addprotocol
//TODO rating table scope is validator and table row includes account but account is not used for querying
//TODO regdevice for a given account
//TODO add in table to track when validators submit new data over old unvalidated info
CONTRACT boidpower : public contract
{
  public:
    using contract::contract;
    
    ACTION regregistrar(name registrar);
    
    ACTION regvalidator(name validator, float weight);
    
    ACTION updateboinc(
      name validator,
      name account,
      uint64_t device_key, 
      uint64_t round,
      float rating
    );
    
    ACTION updateraven(
      name validator,
      name account,
      uint64_t device_key, 
      uint64_t round,
      float rating
    );

    ACTION setminweight(float min_weight);
    
  private:
    TABLE power {
      uint64_t                type;
      float                   rating;
      uint64_t                round;
      uint64_t        primary_key() const {
        return type;
      }
    };
    typedef eosio::multi_index<"powers"_n, power> power_t;

    /*!
      power rating table
     */
    TABLE rating {
      uint64_t                device_key;
      vector<uint64_t>        validators;

      uint64_t        primary_key () const {
        return device_key;
      }
    };
    typedef eosio::multi_index<"ratings"_n, rating> rating_t;

    /*!
      validator table
     */   
    TABLE validator {
      name            account;
      float           weight;
      
      uint64_t        primary_key() const {
        return account.value;
      }
    };
    typedef eosio::multi_index<"validators"_n, validator> validator_t;

    /*!
      configuration table
     */   
    //TODO add protocols in set in config
    TABLE config {
      uint64_t        id;
      name            registrar;
      float           min_weight;
      
      uint64_t        primary_key() const {
        return id;
      }
    };
    typedef eosio::multi_index<"configs"_n, config> config_t;
};


EOSIO_DISPATCH(boidpower,
  (regregistrar)
  (regvalidator)
  (updateboinc)
  (updateraven)
  (setminweight)
)