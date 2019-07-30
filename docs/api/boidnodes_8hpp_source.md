
# File boidnodes.hpp

[**File List**](files.md) **>** [**boidnodes**](dir_faa9e3ab3ac8951a334caa7b59b8744e.md) **>** [**boidnodes.hpp**](boidnodes_8hpp.md)

[Go to the documentation of this file.](boidnodes_8hpp.md) 


````cpp

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

      ACTION create( 
        name accountContractOwner,
        name nodename,
        name owner);
        
      ACTION addteam( 
        name teamContractOwner,
        name accountContractOwner,
        name nodename,
        string teamname, 
        name leader);
        
      ACTION erase(
        name accountContractOwner,
        name nodename,
        name owner);
      
      ACTION eraseteam(
        name teamContractOwner,
        name accountContractOwner,
        name nodename,
        string teamname, 
        name leader);

      template<typename T>
      auto getTeamItr(
        T* dummy,
        name teamContractOwner,
        string teamname,
        name nodename) -> decltype(dummy->end());

   private:
      
      TABLE node_stats {
         name nodename; 
         name owner; 
         uint64_t numTeams; 
         uint64_t numAccounts; 
         uint64_t primary_key()const { return nodename.value; } 
      };
      
      // node stats table (eos ram)
      typedef eosio::multi_index<"stat"_n, node_stats> nodestats;
      
      TABLE team_stats {
        std::map<uint64_t,uint8_t> members; 
        uint64_t teamname; 
        string teamnameStr; 
        name nodeContainer; 
        name leader; 
        uint64_t numAccounts; 
        uint64_t origHash; 
        std::vector<uint64_t> collisions; 
        uint64_t primary_key()const { return teamname; } 
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

      TABLE account {
         name acctname; 
         uint64_t power; 
         std::map<string,uint8_t> teams; 
         std::map<uint64_t,uint8_t> nodes; 
         std::map<string,uint8_t> devices; 
         uint64_t primary_key()const { return acctname.value; } 
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
            
````

