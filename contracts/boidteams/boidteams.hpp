/**
 *  @file
 *  @copyright TODO
 *  @brief Manage teams
 *  @author errcsool
 */
 
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

      /**
        @brief create team
        @param accountContractOwner - owner of boid account contract
        @param nodename - name of node that team will belong to
        @param teamname - name of team to create
        @param owner - team owner account
       */
      ACTION create( 
        name accountContractOwner,
        name nodename,
        string teamname, 
        name owner);
        
      /**
        @brief add account to team
        @param accountContractOwner - owner of boid account contract
        @param nodename - name of node that team belongs to
        @param teamname - name of team to add account to
        @param acct - account to add to team
        Wparam teamleader - leader of team
       */
      ACTION addaccount(
        name accountContractOwner,
        name nodename,
        uint64_t num, 
        string teamname,
        name acct,
        name teamleader);

      /**
        @brief erase team
        @param accountContractOwner - owner of boid account contract
        @param nodename - name of node that team belongs to
        @param teamname - name of team to erase
        @param leader - team leader account
       */       
      ACTION erase(
        name accountContractOwner,
        name nodename,
        uint64_t num, 
        name leader);

      /**
        @brief erase team
        @param accountContractOwner - owner of boid account contract
        @param nodename - name of node that team belongs to
        @param teamname - name of team to add account to
        @param acct - account to add to team
        @param teamleader - team leader account
       */       
      ACTION eraseaccount(
        name accountContractOwner,
        name nodename,
        uint64_t num, 
        name acct,
        name teamleader);

      /**
        @brief Insert named team into team table
        @param leader - team leader
        @param teamname - name of team
        @param nodename - node container
        @return true if team successfully added to team table
       */
      uint64_t addTeam(
        name leader,
        string teamname,
        name nodename);
      
      /**
        @brief Remove named team from team table
        @param leader - team leader
        @param teamname - name of team
        @param nodename - node container
       */
      void removeTeam(
        name leader,
        name nodename,
        uint64_t num);
      
      /**
        @brief Get available hash for team table
        @param nodename - name of node to check for hashes
        @param hash - starting point for search
        @return available hash value
       */
      uint64_t getAvailableTeamNum(name nodename);

      void removeTeamNum(
        name nodename,
        uint64_t num);

      /**
        @brief Check if team is in node
        @param nodename - name of node to check
        @param teamname - name of team to check
        @return true if team in node, else false
       */
      bool teamIsInNode(
        name nodename,
        string teamname);

      /**
        @brief Check if account is in team
        @param accountContractOwner - owner of accounts contract for table check
        @param nodename - name of node to associate with team
        @param teamname - name of team to check
        @param acctname - name of account to check
        @return true if account is in team, else false
       */
      bool accountIsInTeam(
        name accountContractOwner,
        name nodename,
        string teamname,
        name acctname);
      
      /**
        @brief create unique member id for team member
        @param teamname - name of team
        @param acctname - name of account
        @return unique member id in a team
       */
      inline string memberId(
        string teamname,
        name acctname);
      
      /**
        @brief create unique team id in global scope
        @param nodename - name of node for team
        @param teamname - name of team
        @return unique team id in global scope
       */
      inline string teamId(
        name nodename,
        string teamname);
      
   private:
     /*!
      open device number table. because auto-increment is not yet supported
      */
      TABLE teamnum {
        uint64_t dummy;
        uint64_t freeInc;
        std::vector<uint64_t> otherFree;
        
        uint64_t primary_key()const { return dummy; }
      };
      typedef eosio::multi_index<"teamnum"_n, teamnum> teamnum_t;
      
      /*!
        vRam table of team members
        **Future**
       */
      TABLE team_members {
        uint64_t dummyId; /**< Dummy id for unique primary index */
        uint64_t teamId; /**< Team id */
        uint128_t memberId; /**< Member id */
        name acctname; /**< Name of account */
        
        uint64_t primary_key()const { return dummyId; } //!< Dummy primary index
        uint64_t by_teamId()const { return teamId; } //!< Index by team id
        uint128_t by_memberId()const { return memberId; } //!< Index by member id
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
        vRam table for storing boid devices
       */
      TABLE device {
         uint64_t num; /**< Hash of device name */
         string vanityName; /**< Device name string */
         uint64_t power; /**< Associated boidpower */
         name owner; /**< Owner of device */
         name ownerNode; /**< Deprecated */
         bool open; /**< Device can be reassigned */
         uint64_t isFree; /**< Device has no owner */
         bool freeze; /**< Device is not available for use */
         std::map<uint8_t,uint64_t> powerSources; /**< Contribution types for */

         uint64_t primary_key()const { return num; } //!< Index table by device hash
         uint64_t by_free()const { return isFree; } //!< Secondary index by free devices
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