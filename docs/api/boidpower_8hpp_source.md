
# File boidpower.hpp

[**File List**](files.md) **>** [**boidpower**](dir_4fa9b7c4a3edefd214ebf5845c852217.md) **>** [**boidpower.hpp**](boidpower_8hpp.md)

[Go to the documentation of this file.](boidpower_8hpp.md) 


````cpp

#pragma once

#include <vector>
#include <functional>

#include "dappservices/log.hpp"
#include "dappservices/plist.hpp"
#include "dappservices/plisttree.hpp"
#include "dappservices/multi_index.hpp"

#include "boidcommon/boidcommon.hpp"


/*
  Sources of boidpower
 */
#define POWER_SOURCE_MINING 0 
#define POWER_SOURCE_BOINC  1 
#define POWER_SOURCE_VRAM   2 

#define DAPPSERVICES_ACTIONS() \
  XSIGNAL_DAPPSERVICE_ACTION \
  LOG_DAPPSERVICE_ACTIONS \
  IPFS_DAPPSERVICE_ACTIONS

#define DAPPSERVICE_ACTIONS_COMMANDS() \
  IPFS_SVC_COMMANDS()LOG_SVC_COMMANDS() 

#define CONTRACT_NAME() boidpower

namespace eosiosystem {
   class system_contract;
}

using std::string;

CONTRACT_START()
   public:

      ACTION create(
        name accountContractOwner,
        name owner,
        string devname);
      
      ACTION erase(
        name owner,
        string devname);
      
      ACTION changeowner(
        name accountContractOwner,
        name owner,
        string devname);
        
      ACTION freedevice(
        name accountContractOwner,
        name owner,
        string devname);
        
      ACTION assignpower( 
        string devname,
        uint64_t quantity);
      
      ACTION open(
        name owner,
        string devname,
        bool open);
        
      ACTION freeze(
        name owner,
        string devname,
        bool freeze);
      
      void addDeviceByName(
        name owner,
        string devname);
      
      void removeDeviceByName(
        name owner,
        string devname);
      
      uint64_t getAvailableDeviceHash(
        uint64_t hash);
      
      template<typename T>
      auto getDeviceItr(
        T* dummy,
        string devname) -> decltype(dummy->end());
      
      bool accountExists( 
        name accountContractOwner,
        name acctname);
               
   private:
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
      
      TABLE bp_config {
         uint64_t max; 
         uint64_t decayTimeConstant; 
         uint64_t dummy; 
         uint64_t primary_key()const { return dummy; } 
      };

      typedef eosio::multi_index<"config"_n, bp_config> config;

CONTRACT_END(
  (create)
  (erase)
  (changeowner)
  (freedevice)
  (assignpower)
  (open)
  (freeze)
)
````

