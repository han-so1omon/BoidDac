/**
 *  @file
 *  @copyright TODO
 *  @brief Manage devices and associated boidpower
 *  @author errcsool
 */
 
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
#define POWER_SOURCE_MINING 0 //!< Crypto mining boidpower source
#define POWER_SOURCE_BOINC  1 //!< BOINC boidpower source
#define POWER_SOURCE_VRAM   2 //!< vRam boidpower source

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

/*!
  Devices are globally unique (atomic) objects in the boid network.
  Devices are responsible for computations on BOID network
  Devices + boidpower serve as a form of KYC
 */
CONTRACT_START()
   public:

      /** 
        @brief Create device
        @param accountContractOwner - owner of boidaccounts contract
        @param owner - device owner
        @param devname - name of device
       */
      ACTION create(
        name accountContractOwner,
        name owner,
        string devname);
      
      /**
        @brief Erase device
        @param owner - owner of device
        @param devname - name of device to erase
       */
      ACTION erase(
        name owner,
        string devname);
      
      /**
        Wbrief Change device owner
        @param accountContractOwner - owner of boidaccounts contract`
        @param owner - new device owner
        @param devname - name of device to change
       */
      ACTION changeowner(
        name accountContractOwner,
        name owner,
        string devname);
        
      /**
        @brief Free device from owner
        @param accountContractOwner - owner of boidaccounts contract
        @param owner - owner of device to free
        @param devname - name of device to free
       */
      ACTION freedevice(
        name accountContractOwner,
        name owner,
        string devname);
        
      /**
        @brief Assign device to boidpower quantity
        @param devname - name of device to assign power
        @param quantity - amount of power
       */
      ACTION assignpower( 
        string devname,
        uint64_t quantity);
      
      /**
        @brief Open device to changes
        @param owner - owner of device to open
        @param devname - name of device to open
        @param open - whether to open or close device
       */
      ACTION open(
        name owner,
        string devname,
        bool open);
        
      /**
        @brief Freeze device from contributing
        @param owner - owner of device to freeze
        @param devname - name of device to freeze
        @param freeze - whether to freeze or unfreeze device
       */
      ACTION freeze(
        name owner,
        string devname,
        bool freeze);
      
      /**
        @brief Insert named device into device table
        @param owner - owner of device
        @param devname - name of device
        @return true if device successfully added to device table
       */
      void addDeviceByName(
        name owner,
        string devname);
      
      /**
        @brief Remove named device from device table
        @param owner - owner of device
        @param devname - name of device
        @return true if device successfully added to device table
       */
      void removeDeviceByName(
        name owner,
        string devname);
      
      /**
        @brief Get available hash for device table
        @param hash - starting point for search
        @return available hash value
       */
      uint64_t getAvailableDeviceHash(
        uint64_t hash);
      
      /**
        @brief Get device by name
        @param dummy - dummy variable to deduce return type
        @param devname - name of device
        @return iterator of device from device table
       */
      template<typename T>
      auto getDeviceItr(
        T* dummy,
        string devname) -> decltype(dummy->end());
      
      /**
        @brief Check if account exists
        @param accountContractOwner - owner of boidaccounts contract
        @param acctname - name of account to check
        @return true if account exists, else false
       */
      bool accountExists( 
        name accountContractOwner,
        name acctname);
               
   private:
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
        Eos Ram table for storing configuration for boidpower
        equation
       */
      TABLE bp_config {
         uint64_t max; /**< Max possible boidpower */
         uint64_t decayTimeConstant; /**< Time decay constant for boidpower equation */
         uint64_t dummy; /**< Dummy variable for indexing */

         uint64_t primary_key()const { return dummy; } //!< Dummy indexing
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