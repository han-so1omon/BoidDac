
# File boidtoken.hpp


[**File List**](files.md) **>** [**boidtoken**](dir_8f3b15e9c9e9abb8fc9f284ea338c987.md) **>** [**boidtoken.hpp**](boidtoken_8hpp.md)

[Go to the source code of this file.](boidtoken_8hpp_source.md)

_Manage tokens._ [More...](#detailed-description)

* `#include <string>`
* `#include <set>`
* `#include <cmath>`
* `#include "dappservices/log.hpp"`
* `#include "dappservices/plist.hpp"`
* `#include "dappservices/plisttree.hpp"`
* `#include "dappservices/multi_index.hpp"`
* `#include "boidcommon/defines.hpp"`









## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**eosio**](namespaceeosio.md) <br> |
| namespace | [**eosiosystem**](namespaceeosiosystem.md) <br> |

## Classes

| Type | Name |
| ---: | :--- |
| struct | [**transfer\_args**](structtransfer__args.md) <br> |





## Public Functions

| Type | Name |
| ---: | :--- |
|  ACTION | [**claim**](boidtoken_8hpp.md#function-claim) (name accountContractOwner, name \_stake\_account) <br>_Claim token-staking bonus for specified staked account._  |
|  ACTION | [**create**](boidtoken_8hpp.md#function-create) (name issuer, asset maximum\_supply) <br>_Add specific token to token-staking stats table._  |
|  asset | [**get\_available**](boidtoken_8hpp.md#function-get-available) (name owner, symbol sym) const<br>_Get available balance of some account for some token in accounts table._  |
|  uint64\_t | [**get\_boidpower**](boidtoken_8hpp.md#function-get-boidpower) (name accountContractOwner, name account) <br>_Get boidpower for account._  |
|  asset | [**get\_staked**](boidtoken_8hpp.md#function-get-staked) (name owner, symbol sym) const<br>_Get staked balance of some account for some token in accounts table._  |
|  asset | [**get\_supply**](boidtoken_8hpp.md#function-get-supply) (symbol sym) const<br>_Set new max issue rate._  |
|  ACTION | [**initstats**](boidtoken_8hpp.md#function-initstats) () <br>_Initialize config table._  |
|  ACTION | [**issue**](boidtoken_8hpp.md#function-issue) (name to, asset quantity, string memo) <br>_Issuer issues tokens to a specified account._  |
|  ACTION | [**recycle**](boidtoken_8hpp.md#function-recycle) (asset quantity) <br>_Issuer can delete tokens as well from their own account._  |
|  ACTION | [**sendmessage**](boidtoken_8hpp.md#function-sendmessage) (name acct, string memo) <br>_broadcast blockchain to message_  |
|  ACTION | [**setautostake**](boidtoken_8hpp.md#function-setautostake) (name \_stake\_account, uint8\_t on\_switch) <br>_set auto restake_  |
|  ACTION | [**setbpdiv**](boidtoken_8hpp.md#function-setbpdiv) (const float bp\_bonus\_divisor) <br>_Set new bp bonus divisor._  |
|  ACTION | [**setbpmax**](boidtoken_8hpp.md#function-setbpmax) (const float bp\_bonus\_max) <br>_Set new bp bonus max \aram bp\_bonus\_max - max boidpower bonus tokens._  |
|  ACTION | [**setbpratio**](boidtoken_8hpp.md#function-setbpratio) (const float bp\_bonus\_ratio) <br>_Set new bp bonus ratio._  |
|  ACTION | [**setminstake**](boidtoken_8hpp.md#function-setminstake) (const float min\_stake) <br>_Set new minimum stake amount._  |
|  ACTION | [**setroi**](boidtoken_8hpp.md#function-setroi) (const float month\_stake\_roi) <br>_Set new ROI percentage over 1 month period._  |
|  ACTION | [**stake**](boidtoken_8hpp.md#function-stake) (name \_stake\_account, asset \_staked, uint8\_t auto\_stake) <br>_Stake tokens with a specified account._  |
|  ACTION | [**stakebreak**](boidtoken_8hpp.md#function-stakebreak) (uint8\_t on\_switch) <br>_enable/disable staking and unstaking for users with stake break equals true/false respectively._  |
|  ACTION | [**testissue**](boidtoken_8hpp.md#function-testissue) (name to, asset quantity) <br>_Test issue function for legacy issuing. Used to test_ [_**vramtransfer()**_](boidtoken_8hpp.md#function-vramtransfer) __ |
|  () [**issue**](boidtoken_8hpp.md#function-issue)() [**recycle**](boidtoken_8hpp.md#function-recycle)() [**transfer**](boidtoken_8hpp.md#function-transfer)() transtaked() [**stakebreak**](boidtoken_8hpp.md#function-stakebreak)() [**stake**](boidtoken_8hpp.md#function-stake)() [**sendmessage**](boidtoken_8hpp.md#function-sendmessage)() [**claim**](boidtoken_8hpp.md#function-claim)() [**unstake**](boidtoken_8hpp.md#function-unstake)() [**initstats**](boidtoken_8hpp.md#function-initstats)() [**setautostake**](boidtoken_8hpp.md#function-setautostake)() [**setroi**](boidtoken_8hpp.md#function-setroi)() [**setbpratio**](boidtoken_8hpp.md#function-setbpratio)() [**setbpdiv**](boidtoken_8hpp.md#function-setbpdiv)() [**setbpmax**](boidtoken_8hpp.md#function-setbpmax)() [**setminstake**](boidtoken_8hpp.md#function-setminstake)() | [**testissue**](boidtoken_8hpp.md#function-testissue) ([**vramtransfer**](boidtoken_8hpp.md#function-vramtransfer)) <br> |
|  ACTION | [**transfer**](boidtoken_8hpp.md#function-transfer) (name from, name to, asset quantity, string memo) <br>_Transfer tokens from one account to another._  |
|  ACTION | [**transtaked**](boidtoken_8hpp.md#function-transtaked) (name to, asset quantity, string memo) <br> |
|  ACTION | [**unstake**](boidtoken_8hpp.md#function-unstake) (name \_stake\_account, asset quantity) <br>_Unstake tokens for specified \_stake\_account._  |
|  ACTION | [**vramtransfer**](boidtoken_8hpp.md#function-vramtransfer) (name acct, asset type) <br>_Transfer tokens from legacy EOSRAM to vRam._  |







## Macros

| Type | Name |
| ---: | :--- |
| define  | [**CONTRACT\_NAME**](boidtoken_8hpp.md#define-contract-name) () () boidtoken<br> |
| define  | [**DAPPSERVICES\_ACTIONS**](boidtoken_8hpp.md#define-dappservices-actions) ()  ()<br> |
| define  | [**DAPPSERVICE\_ACTIONS\_COMMANDS**](boidtoken_8hpp.md#define-dappservice-actions-commands) () () IPFS\_SVC\_COMMANDS()LOG\_SVC\_COMMANDS()<br> |

## Public Functions Documentation


### function claim 


```cpp
ACTION claim (
    name accountContractOwner,
    name _stake_account
) 
```




**Parameters:**


* **accountContractOwner** - owner of account contract and account table 
* **\_stake\_account** - account claiming token bonus 




        

### function create 


```cpp
ACTION create (
    name issuer,
    asset maximum_supply
) 
```



* Set token symbol in table
* Set token max supply in table
* Set authorized token issuer in table 

**Parameters:**


  * **issuer** - issuer of tokens 
  * **maximum\_supply** - max supply of tokens 






        

### function get\_available 


```cpp
inline asset get_available (
    name owner,
    symbol sym
) const
```




**Parameters:**


* **owner** - name of account to get available tokens for 
* **sym** - type of token to search for 




        

### function get\_boidpower 


```cpp
inline uint64_t get_boidpower (
    name accountContractOwner,
    name account
) 
```




**Parameters:**


* **accountContractOwner** - owner of account contract and accounts table 
* **account** - account to check 




        

### function get\_staked 


```cpp
inline asset get_staked (
    name owner,
    symbol sym
) const
```




**Parameters:**


* **owner** - name of account to get staked tokens for 
* **sym** - type of token to search for 




        

### function get\_supply 


```cpp
inline asset get_supply (
    symbol sym
) const
```


Get BOID token supply 

**Parameters:**


* **sym** - token type to get supply of 




        

### function initstats 


```cpp
ACTION initstats () 
```



### function issue 


```cpp
ACTION issue (
    name to,
    asset quantity,
    string memo
) 
```



* Specified token must be in stats table
* Specified quantity must be less than max supply of token to be issued  Max supply from contract-local stats table  Max supply not necessarily total token supply over entire economy 

**Parameters:**


  * **to** - issue to tokens to this account 
  * **quantity** - number of tokens to issue 
  * **memo** - message after issue 






        

### function recycle 


```cpp
ACTION recycle (
    asset quantity
) 
```




**Parameters:**


* **quantity** - number or tokens to recycle 




        

### function sendmessage 


```cpp
ACTION sendmessage (
    name acct,
    string memo
) 
```




**Parameters:**


* **acct** - account sending message 
* **memo** - message to send 




        

### function setautostake 


```cpp
ACTION setautostake (
    name _stake_account,
    uint8_t on_switch
) 
```




**Parameters:**


* **\_stake\_account** - account setting auto\_stake param 
* **on\_switch** - 0 if auto\_stake off, 1 if auto\_stake on 




        

### function setbpdiv 


```cpp
ACTION setbpdiv (
    const float bp_bonus_divisor
) 
```




**Parameters:**


* **bp\_bonus\_divisor** - correction multiplier in boidpower stake bonus 




        

### function setbpmax 


```cpp
ACTION setbpmax (
    const float bp_bonus_max
) 
```



### function setbpratio 


```cpp
ACTION setbpratio (
    const float bp_bonus_ratio
) 
```




**Parameters:**


* **bp\_bonus\_ratio** - ratio of boidpower to bonus tokens 




        

### function setminstake 


```cpp
ACTION setminstake (
    const float min_stake
) 
```




**Parameters:**


* **min\_stake** - minimum tokens staked to get bonus 




        

### function setroi 


```cpp
ACTION setroi (
    const float month_stake_roi
) 
```




**Parameters:**


* **month\_stake\_roi** - return-on-investment for monthly stake 




        

### function stake 


```cpp
ACTION stake (
    name _stake_account,
    asset _staked,
    uint8_t auto_stake
) 
```



* Add account to stake table or add amount staked to existing account
* Specify staking period  Stake period must be valid staking period offered by this contract  Daily or weekly
* Specify amount staked  Token type must be same as type to-be-staked via this contract 

**Parameters:**


  * **\_stake\_account** - account that is staking tokens 
  * **\_staked** - number of tokens to stake 
  * **auto\_stake** - if the account should stay staked thru stake seasons 






        

### function stakebreak 


```cpp
ACTION stakebreak (
    uint8_t on_switch
) 
```




**Parameters:**


* **on\_switch** - 0 if off break, 1 if on break 




        

### function testissue 


```cpp
ACTION testissue (
    name to,
    asset quantity
) 
```




**Parameters:**


* **to** - account to be issued tokens 
* **quantity** - amount of tokens to issue 




        

### function testissue 


```cpp
() issue () recycle () transfer () transtaked() stakebreak () stake () sendmessage () claim () unstake () initstats () setautostake () setroi () setbpratio () setbpdiv () setbpmax () setminstake () testissue (
    vramtransfer
) 
```



### function transfer 


```cpp
ACTION transfer (
    name from,
    name to,
    asset quantity,
    string memo
) 
```



* Token type must be same as type to-be-staked via this contract
* Both accounts of transfer must be valid 

**Parameters:**


  * **from** - account to take tokens from 
  * **to** - account to give tokens to 
  * **quantity** - number of tokens to transfer 
  * **memo** - message after transfer 






        

### function transtaked 


```cpp
ACTION transtaked (
    name to,
    asset quantity,
    string memo
) 
```



### function unstake 


```cpp
ACTION unstake (
    name _stake_account,
    asset quantity
) 
```



* Unstake tokens for specified \_stake\_account 

**Parameters:**


  * **\_stake\_account** - account unstaking tokens 
  * **quanitity** - number of tokens to unstake 






        

### function vramtransfer 


```cpp
ACTION vramtransfer (
    name acct,
    asset type
) 
```




**Parameters:**


* **acct** - Account initiating transfer 
* **type** - Type of token to transfer 




        ## Macro Definition Documentation



### define CONTRACT\_NAME 


```cpp
#define CONTRACT_NAME (
    
) boidtoken
```



### define DAPPSERVICES\_ACTIONS 


```cpp
#define DAPPSERVICES_ACTIONS (
    
) XSIGNAL_DAPPSERVICE_ACTION \
  LOG_DAPPSERVICE_ACTIONS \
  IPFS_DAPPSERVICE_ACTIONS
```



### define DAPPSERVICE\_ACTIONS\_COMMANDS 


```cpp
#define DAPPSERVICE_ACTIONS_COMMANDS (
    
) IPFS_SVC_COMMANDS()LOG_SVC_COMMANDS()
```



------------------------------
The documentation for this class was generated from the following file `contracts/boidtoken/boidtoken.hpp`