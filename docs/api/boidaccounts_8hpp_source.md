
# File boidaccounts.hpp

[**File List**](files.md) **>** [**boidaccounts**](dir_5457141dbb61115f5a8cfafcf4df73ff.md) **>** [**boidaccounts.hpp**](boidaccounts_8hpp.md)

[Go to the documentation of this file.](boidaccounts_8hpp.md) 


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

      ACTION create(
        name acctname);
                     
      ACTION associatedev(
        name devContractAcct,
        name acctname,
        string devname,
        uint8_t devType);
                           
      ACTION assignteam(
        name acctname,
        name teamleader,
        string teamname,
        name nodename,
        uint8_t memberType);
      
      ACTION assignnode(
        name acctname,
        name nodeowner,
        name nodename,
        uint8_t memberType);
        
      ACTION updatepower(
        name devContractAcct,
        name acctname);
        
      ACTION erase(
        name devContractAcct,
        name acctname,
        string devName,
        bool freedev);
      
      ACTION erasedev(
        name devContractAcct,
        name acctname,
        string devname);
      
      ACTION eraseteam(
        name acctname,
        name teamleader,
        string teamname,
        name nodename);
      
      ACTION erasenode(
        name acctname,
        name nodeowner,
        name nodename);
        
      template<typename T>
      auto getDeviceItr(
        T* dummy,
        name deviceContractOwner,
        string devname) -> decltype(dummy->end());

      inline string memberId(
        string teamname,
        name acctname);

      inline string teamId(
        name nodename,
        string teamname);
      
   private:
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
````

