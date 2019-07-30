/**
 *  @file
 *  @copyright TODO
 *  @brief Manage nodes
 *  @author errcsool
 */
 
#pragma once

#include <map>

#include "dappservices/log.hpp"
#include "dappservices/plist.hpp"
#include "dappservices/plisttree.hpp"
#include "dappservices/multi_index.hpp"

#include "boidcommon/boidcommon.hpp"

#define DAPPSERVICES_ACTIONS() \
  XSIGNAL_DAPPSERVICE_ACTION \
  LOG_DAPPSERVICE_ACTIONS \
  IPFS_DAPPSERVICE_ACTIONS

#define DAPPSERVICE_ACTIONS_COMMANDS() \
  IPFS_SVC_COMMANDS()LOG_SVC_COMMANDS() 

#define CONTRACT_NAME() boidnodes

namespace eosiosystem {
   class system_contract;
}

using std::string;

CONTRACT_START()
   public:

      /**
        @brief create boid node
        @param accountContractOwner - owner of boid accounts contract
        @param nodename - name of node to create
        @param owner - owner of node
       */
      ACTION create( 
        name accountContractOwner,
        name nodename,
        name owner);
        
      /**
        @brief add team to node
        @param teamContractOwner - owner of boid teams contract
        @param accountContractOwner - owner of boid accounts contract
        @param nodename - node to add team to
        @param teamname - name of team to add
        @param leader - name of team leader
       */
      ACTION addteam( 
        name teamContractOwner,
        name accountContractOwner,
        name nodename,
        string teamname, 
        name leader);
        
      /**
        @brief erase node
        @param accountContractOwner - owner of boid accounts contract
        @param nodename - name of node to erase
        @param owner - owner of node
       */
      ACTION erase(
        name accountContractOwner,
        name nodename,
        name owner);
      
      /**
        @brief erase team from node
        @param teamContractOwner - owner of boid teams contract
        @param accountContractOwner - owner of boid accounts contract
        @param nodename - name of node that team belongs to
        @param teamname - name of team to erase
        @param leader - team leader
       */
      ACTION eraseteam(
        name teamContractOwner,
        name accountContractOwner,
        name nodename,
        string teamname, 
        name leader);

      /**
        @brief Get team by name
        @param dummy - dummy param to deduce return type
        @param teamContractOwner - owner of team contract and team_stats table
        @param teamname - name of team
        @param nodename - name of node
        @return iterator of team from teams table
       */
      template<typename T>
      auto getTeamItr(
        T* dummy,
        name teamContractOwner,
        string teamname,
        name nodename) -> decltype(dummy->end());

   private:
      
      /*!
        vRam table for storing node information
       */
      TABLE node_stats {
         name nodename; /**< Name of node */
         name owner; /**< Owner of node */
         uint64_t numTeams; /**< Number of teams in mode */
         uint64_t numAccounts; /**< Number of accounts in node */

         uint64_t primary_key()const { return nodename.value; } //!< Index by node name
      };
      
      // node stats table (eos ram)
      typedef eosio::multi_index<"stat"_n, node_stats> nodestats;
      
      /*!
        vRam table for storing team information
       */
      TABLE team_stats {
        std::map<uint64_t,uint8_t> members; /**< List of team members */
        uint64_t num; /**< Team name hash*/
        string vanityName; /**< Team name string */
        name nodeContainer; /**< Node containing team */
        name leader; /**< Team leader */
        uint64_t numAccounts; /**< Number of accounts in team */

        uint64_t primary_key()const { return num; } //!< Index by team name hash
      };
      
      // team stats table (vram)
      typedef dapp::multi_index<"stat"_n, team_stats> teamstats;
      
      typedef eosio::multi_index<".stat"_n, team_stats> stat_t_v_abi;
      TABLE statshards {
          std::vector<char> shard_uri;
          uint64_t shard;
          uint64_t primary_key() const { return shard; }
      };
      typedef eosio::multi_index<"stat"_n, statshards> stat_t_abi;
      
      /*!
        vRam table for storing boid accounts
       */
      TABLE account {
         name acctname; /**< Name of account */
         uint64_t power; /**< Associated boidpower of account */
         std::map<string,uint8_t> teams; /**< Associated teams */
         std::map<uint64_t,uint8_t> nodes; /**< Associated nodes */
         std::map<string,uint8_t> devices; /**< Associated devices */
 
         uint64_t primary_key()const { return acctname.value; } //!< Index account by name
      };
      // accounts table (vram)
      typedef dapp::multi_index<"account"_n, account> account_t;
      
      typedef eosio::multi_index<".account"_n, account> account_t_v_abi;
      TABLE accountshards {
          std::vector<char> shard_uri;
          uint64_t shard;
          uint64_t primary_key() const { return shard; }
      };
      typedef eosio::multi_index<"account"_n, accountshards> account_t_abi;
      
CONTRACT_END(
  (create)
  (addteam)
  (erase)
  (eraseteam)
)
            