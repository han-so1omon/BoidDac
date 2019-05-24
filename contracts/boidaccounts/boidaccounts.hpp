/**
 *  @file
 *  @copyright TODO
 *  @brief Manage accounts
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

#define CONTRACT_NAME() boidaccounts

namespace eosiosystem {
   class system_contract;
}

using std::string;

                
//TODO generate private/public key pair for account upon creation                
//FIXME do not store teamnames and nodenames, as it
//      makes erasing teams/nodes very difficult
                
CONTRACT_START()
   public:

      /**
        @brief create boid account
        @param acctname - name of account to create
       */
      ACTION create(
        name acctname);
                     
      /**
        @brief associate account with device
        @param devContractAcct - owner of boidpower contract
        @param acctname - name of account to associate
        @param devname - name of device to associate
        @param devType - type of device association
       */
      ACTION associatedev(
        name devContractAcct,
        name acctname,
        string devname,
        uint8_t devType);
                           
      /**
        @brief associate account with team
        @param acctname - name of account to associate
        @param teamleader - leader of team
        @param teamname - name of team to associate
        @param nodename - name of node to associate
        @param memberType - type of team association
       */
      ACTION assignteam(
        name acctname,
        name teamleader,
        string teamname,
        name nodename,
        uint8_t memberType);
      
      /**
        @brief associate account with node
        @param acctname - name of account to associate
        @param nodeowner - owner of node
        @param nodename - name of node to associate
        @param memberType - type of node association
       */
      ACTION assignnode(
        name acctname,
        name nodeowner,
        name nodename,
        uint8_t memberType);
        
      /**
        @brief update power of account
        @param devContractAcct - account of boidpower contract and device table
        @param acctname - name of account to update
       */
      ACTION updatepower(
        name devContractAcct,
        name acctname);
        
      /**
        @brief erase account
        @param devContractAcct - owner of boidpower contract and device table
        @param acctname - name of account to erase
        @param devName - name of device to erase in association
        @param freedev - whether to free associated device
       */
      ACTION erase(
        name devContractAcct,
        name acctname,
        string devName,
        bool freedev);
      
      /**
        @brief erase device from account
        @param devContractAcct - owner of boidpower contract
        @param acctname - name of account associated with device
        @param devname - name of device to erase
       */
      ACTION erasedev(
        name devContractAcct,
        name acctname,
        string devname);
      
      /**
        @brief erase team from account
        @param acctname - name of account to access
        @param teamleader - name of team leader
        @param teamname - name of team to erase from account
        @param nodename - name of node with team
       */
      ACTION eraseteam(
        name acctname,
        name teamleader,
        string teamname,
        name nodename);
      
      /**
        @brief erase node from account
        @param acctname - name of account to access
        @param nodeowner - owner of node
        @param nodename - name of node to erase from account
       */
      ACTION erasenode(
        name acctname,
        name nodeowner,
        name nodename);
        
      template<typename T>
      auto getDeviceItr(
        T* dummy,
        name deviceContractOwner,
        string devname) -> decltype(dummy->end());

      /**
        @brief get memberId from team name and account name
        @param teamname - name of team
        @param acctname - name of account
       */
      inline string memberId(
        string teamname,
        name acctname);

      /**
        @brief get teamId from node name and account name
        @param nodename - name of node
        @param teamname - name of team
       */
      inline string teamId(
        name nodename,
        string teamname);
      
   private:
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
      
      /*!
        vRam table for storing boid devices
       */
      TABLE device {
         uint64_t devname; /**< Hash of device name */
         string devnameStr; /**< Device name string */
         uint64_t power; /**< Associated boidpower */
         name owner; /**< Owner of device */
         name ownerNode; /**< Deprecated */
         bool open; /**< Device can be reassigned */
         uint64_t isFree; /**< Device has no owner */
         bool freeze; /**< Device is not available for use */
         std::map<uint8_t,uint64_t> powerSources; /**< Contribution types for */
         uint64_t origHash; /**< For handling device hash collisions */
         std::vector<uint64_t> collisions; /**< List of device hash collisions */

         uint64_t primary_key()const { return devname; } //!< Index table by device hash
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

CONTRACT_END(
  (create)
  (erase)
  (associatedev)
  (assignteam)
  (assignnode)
  (updatepower)
  (erasedev)
  (eraseteam)
  (erasenode)
)

string boidaccounts::memberId(
        string teamname,
        name acctname)
{
  string acctstr = acctname.to_string();
  return acctstr + teamname;
}

string boidaccounts::teamId(
  name nodename,
  string teamname)
{
  string nodestr = nodename.to_string();
  return teamname + nodestr;
}