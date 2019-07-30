
# File boidteams.hpp

[**File List**](files.md) **>** [**boidteams**](dir_885226d501ced227e3fb077d4dccbffb.md) **>** [**boidteams.hpp**](boidteams_8hpp.md)

[Go to the documentation of this file.](boidteams_8hpp.md) 


````cpp

#pragma once

//TODO store list of team members
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


#define CONTRACT_NAME() boidteams

namespace eosiosystem {
   class system_contract;
}

using std::string;

CONTRACT_START()
   public:

      ACTION create( 
        name accountContractOwner,
        name nodename,
        string teamname, 
        name owner);
        
      ACTION addaccount(
        name accountContractOwner,
        name nodename,
        string teamname, 
        name acct,
        name teamleader);

      ACTION erase(
        name accountContractOwner,
        name nodename,
        string teamname, 
        name leader);

      ACTION eraseaccount(
        name accountContractOwner,
        name nodename,
        string teamname, 
        name acct,
        name teamleader);

      void addTeamByName(
        name leader,
        string teamname,
        name nodename);
      
      void removeTeamByName(
        name leader,
        string teamname,
        name nodename);
      
      uint64_t getAvailableTeamHash(
        name nodename,
        uint64_t hash);

      template<typename T>
      auto getTeamItr(
        T* dummy,
        string teamname,
        name nodename) -> decltype(dummy->end());
      
      bool teamIsInNode(
        name nodename,
        string teamname);

      bool accountIsInTeam(
        name accountContractOwner,
        name nodename,
        string teamname,
        name acctname);
      
      inline string memberId(
        string teamname,
        name acctname);
      
      inline string teamId(
        name nodename,
        string teamname);
      
   private:
      TABLE team_members {
        uint64_t dummyId; 
        uint64_t teamId; 
        uint128_t memberId; 
        name acctname; 
        uint64_t primary_key()const { return dummyId; } 
        uint64_t by_teamId()const { return teamId; } 
        uint128_t by_memberId()const { return memberId; } 
      };
      // team members table (vram)
      typedef dapp::multi_index<
        "member"_n,
        team_members,
        indexed_by<
          "teamid"_n,
          const_mem_fun<
            team_members,
            uint64_t,
            &team_members::by_teamId
          >
        >,
        indexed_by<
          "memberid"_n,
          const_mem_fun<
            team_members,
            uint128_t,
            &team_members::by_memberId
          >
        >
      > teammembers;
      
      typedef eosio::multi_index<".member"_n, team_members> member_t_v_abi;
      TABLE membershards {
          std::vector<char> shard_uri;
          uint64_t shard;
          uint64_t primary_key() const { return shard; }
      };
      typedef eosio::multi_index<"member"_n, membershards> member_t_abi;
      
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
      TABLE device {
         uint64_t devname; 
         string devnameStr; 
         uint64_t power; 
         name owner; 
         name ownerNode; 
         bool open; 
         uint64_t isFree; 
         bool freeze; 
         std::map<uint8_t,uint64_t> powerSources; 
         uint64_t origHash; 
         std::vector<uint64_t> collisions; 
         uint64_t primary_key()const { return devname; } 
         uint64_t by_free()const { return isFree; } 
      };
      typedef dapp::multi_index<
        "device"_n,
        device,
        eosio::indexed_by<
          "free"_n,
          const_mem_fun<
            device,
            uint64_t,
            &device::by_free
          >
        >
      > device_t;
      
      typedef eosio::multi_index<".device"_n, device> device_t_v_abi;
      TABLE deviceshards {
          std::vector<char> shard_uri;
          uint64_t shard;
          uint64_t primary_key() const { return shard; }
      };
      typedef eosio::multi_index<"device"_n, deviceshards> device_t_abi;

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
      

CONTRACT_END((create)(addaccount)(erase)(eraseaccount))

string boidteams::memberId(
        string teamname,
        name acctname)
{
  return acctname.to_string() + teamname;
}

string boidteams::teamId(
  name nodename,
  string teamname)
{
  return teamname + nodename.to_string();
}
````

