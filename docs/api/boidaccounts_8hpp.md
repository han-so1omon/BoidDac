
# File boidaccounts.hpp


[**File List**](files.md) **>** [**boidaccounts**](dir_5457141dbb61115f5a8cfafcf4df73ff.md) **>** [**boidaccounts.hpp**](boidaccounts_8hpp.md)

[Go to the source code of this file.](boidaccounts_8hpp_source.md)

_Manage accounts._ [More...](#detailed-description)

* `#include <map>`
* `#include "dappservices/log.hpp"`
* `#include "dappservices/plist.hpp"`
* `#include "dappservices/plisttree.hpp"`
* `#include "dappservices/multi_index.hpp"`
* `#include "boidcommon/boidcommon.hpp"`









## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**eosiosystem**](namespaceeosiosystem.md) <br> |






## Public Functions

| Type | Name |
| ---: | :--- |
|  ACTION | [**assignnode**](boidaccounts_8hpp.md#function-assignnode) (name acctname, name nodeowner, name nodename, uint8\_t memberType) <br>_associate account with node_  |
|  ACTION | [**assignteam**](boidaccounts_8hpp.md#function-assignteam) (name acctname, name teamleader, string teamname, name nodename, uint8\_t memberType) <br>_associate account with team_  |
|  ACTION | [**associatedev**](boidaccounts_8hpp.md#function-associatedev) (name devContractAcct, name acctname, string devname, uint8\_t devType) <br>_associate account with device_  |
|  ACTION | [**create**](boidaccounts_8hpp.md#function-create) (name acctname) <br>_create boid account_  |
|  ACTION | [**erase**](boidaccounts_8hpp.md#function-erase) (name devContractAcct, name acctname, string devName, bool freedev) <br>_erase account_  |
|  ACTION | [**erasedev**](boidaccounts_8hpp.md#function-erasedev) (name devContractAcct, name acctname, string devname) <br>_erase device from account_  |
|  ACTION | [**erasenode**](boidaccounts_8hpp.md#function-erasenode) (name acctname, name nodeowner, name nodename) <br>_erase node from account_  |
|  ACTION | [**eraseteam**](boidaccounts_8hpp.md#function-eraseteam) (name acctname, name teamleader, string teamname, name nodename) <br>_erase team from account_  |
|  auto | [**getDeviceItr**](boidaccounts_8hpp.md#function-getdeviceitr) (T \* dummy, name deviceContractOwner, string devname) <br> |
|  string | [**memberId**](boidaccounts_8hpp.md#function-memberid) (string teamname, name acctname) <br>_get memberId from team name and account name_  |
|  string | [**teamId**](boidaccounts_8hpp.md#function-teamid) (name nodename, string teamname) <br>_get teamId from node name and account name_  |
|  ACTION | [**updatepower**](boidaccounts_8hpp.md#function-updatepower) (name devContractAcct, name acctname) <br>_update power of account_  |







## Macros

| Type | Name |
| ---: | :--- |
| define  | [**CONTRACT\_NAME**](boidaccounts_8hpp.md#define-contract-name) () () boidaccounts<br> |
| define  | [**DAPPSERVICES\_ACTIONS**](boidaccounts_8hpp.md#define-dappservices-actions) ()  ()<br> |
| define  | [**DAPPSERVICE\_ACTIONS\_COMMANDS**](boidaccounts_8hpp.md#define-dappservice-actions-commands) () () IPFS\_SVC\_COMMANDS()LOG\_SVC\_COMMANDS()<br> |

## Public Functions Documentation


### function assignnode 


```cpp
ACTION assignnode (
    name acctname,
    name nodeowner,
    name nodename,
    uint8_t memberType
) 
```




**Parameters:**


* **acctname** - name of account to associate 
* **nodeowner** - owner of node 
* **nodename** - name of node to associate 
* **memberType** - type of node association 




        

### function assignteam 


```cpp
ACTION assignteam (
    name acctname,
    name teamleader,
    string teamname,
    name nodename,
    uint8_t memberType
) 
```




**Parameters:**


* **acctname** - name of account to associate 
* **teamleader** - leader of team 
* **teamname** - name of team to associate 
* **nodename** - name of node to associate 
* **memberType** - type of team association 




        

### function associatedev 


```cpp
ACTION associatedev (
    name devContractAcct,
    name acctname,
    string devname,
    uint8_t devType
) 
```




**Parameters:**


* **devContractAcct** - owner of boidpower contract 
* **acctname** - name of account to associate 
* **devname** - name of device to associate 
* **devType** - type of device association 




        

### function create 


```cpp
ACTION create (
    name acctname
) 
```




**Parameters:**


* **acctname** - name of account to create 




        

### function erase 


```cpp
ACTION erase (
    name devContractAcct,
    name acctname,
    string devName,
    bool freedev
) 
```




**Parameters:**


* **devContractAcct** - owner of boidpower contract and device table 
* **acctname** - name of account to erase 
* **devName** - name of device to erase in association 
* **freedev** - whether to free associated device 




        

### function erasedev 


```cpp
ACTION erasedev (
    name devContractAcct,
    name acctname,
    string devname
) 
```




**Parameters:**


* **devContractAcct** - owner of boidpower contract 
* **acctname** - name of account associated with device 
* **devname** - name of device to erase 




        

### function erasenode 


```cpp
ACTION erasenode (
    name acctname,
    name nodeowner,
    name nodename
) 
```




**Parameters:**


* **acctname** - name of account to access 
* **nodeowner** - owner of node 
* **nodename** - name of node to erase from account 




        

### function eraseteam 


```cpp
ACTION eraseteam (
    name acctname,
    name teamleader,
    string teamname,
    name nodename
) 
```




**Parameters:**


* **acctname** - name of account to access 
* **teamleader** - name of team leader 
* **teamname** - name of team to erase from account 
* **nodename** - name of node with team 




        

### function getDeviceItr 


```cpp
template<typename T typename T>
auto getDeviceItr (
    T * dummy,
    name deviceContractOwner,
    string devname
) 
```



### function memberId 


```cpp
inline string memberId (
    string teamname,
    name acctname
) 
```




**Parameters:**


* **teamname** - name of team 
* **acctname** - name of account 




        

### function teamId 


```cpp
inline string teamId (
    name nodename,
    string teamname
) 
```




**Parameters:**


* **nodename** - name of node 
* **teamname** - name of team 




        

### function updatepower 


```cpp
ACTION updatepower (
    name devContractAcct,
    name acctname
) 
```




**Parameters:**


* **devContractAcct** - account of boidpower contract and device table 
* **acctname** - name of account to update 




        ## Macro Definition Documentation



### define CONTRACT\_NAME 


```cpp
#define CONTRACT_NAME (
    
) boidaccounts
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
The documentation for this class was generated from the following file `contracts/boidaccounts/boidaccounts.hpp`