
# File boidnodes.hpp


[**File List**](files.md) **>** [**boidnodes**](dir_faa9e3ab3ac8951a334caa7b59b8744e.md) **>** [**boidnodes.hpp**](boidnodes_8hpp.md)

[Go to the source code of this file.](boidnodes_8hpp_source.md)

_Manage nodes._ [More...](#detailed-description)

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
|  ACTION | [**addteam**](boidnodes_8hpp.md#function-addteam) (name teamContractOwner, name accountContractOwner, name nodename, string teamname, name leader) <br>_add team to node_  |
|  ACTION | [**create**](boidnodes_8hpp.md#function-create) (name accountContractOwner, name nodename, name owner) <br>_create boid node_  |
|  ACTION | [**erase**](boidnodes_8hpp.md#function-erase) (name accountContractOwner, name nodename, name owner) <br>_erase node_  |
|  ACTION | [**eraseteam**](boidnodes_8hpp.md#function-eraseteam) (name teamContractOwner, name accountContractOwner, name nodename, string teamname, name leader) <br>_erase team from node_  |
|  auto | [**getTeamItr**](boidnodes_8hpp.md#function-getteamitr) (T \* dummy, name teamContractOwner, string teamname, name nodename) <br>_Get team by name._  |







## Macros

| Type | Name |
| ---: | :--- |
| define  | [**CONTRACT\_NAME**](boidnodes_8hpp.md#define-contract-name) () () boidnodes<br> |
| define  | [**DAPPSERVICES\_ACTIONS**](boidnodes_8hpp.md#define-dappservices-actions) ()  ()<br> |
| define  | [**DAPPSERVICE\_ACTIONS\_COMMANDS**](boidnodes_8hpp.md#define-dappservice-actions-commands) () () IPFS\_SVC\_COMMANDS()LOG\_SVC\_COMMANDS()<br> |

## Public Functions Documentation


### function addteam 


```cpp
ACTION addteam (
    name teamContractOwner,
    name accountContractOwner,
    name nodename,
    string teamname,
    name leader
) 
```




**Parameters:**


* **teamContractOwner** - owner of boid teams contract 
* **accountContractOwner** - owner of boid accounts contract 
* **nodename** - node to add team to 
* **teamname** - name of team to add 
* **leader** - name of team leader 




        

### function create 


```cpp
ACTION create (
    name accountContractOwner,
    name nodename,
    name owner
) 
```




**Parameters:**


* **accountContractOwner** - owner of boid accounts contract 
* **nodename** - name of node to create 
* **owner** - owner of node 




        

### function erase 


```cpp
ACTION erase (
    name accountContractOwner,
    name nodename,
    name owner
) 
```




**Parameters:**


* **accountContractOwner** - owner of boid accounts contract 
* **nodename** - name of node to erase 
* **owner** - owner of node 




        

### function eraseteam 


```cpp
ACTION eraseteam (
    name teamContractOwner,
    name accountContractOwner,
    name nodename,
    string teamname,
    name leader
) 
```




**Parameters:**


* **teamContractOwner** - owner of boid teams contract 
* **accountContractOwner** - owner of boid accounts contract 
* **nodename** - name of node that team belongs to 
* **teamname** - name of team to erase 
* **leader** - team leader 




        

### function getTeamItr 


```cpp
template<typename T typename T>
auto getTeamItr (
    T * dummy,
    name teamContractOwner,
    string teamname,
    name nodename
) 
```




**Parameters:**


* **dummy** - dummy param to deduce return type 
* **teamContractOwner** - owner of team contract and team\_stats table 
* **teamname** - name of team 
* **nodename** - name of node 



**Returns:**

iterator of team from teams table 





        ## Macro Definition Documentation



### define CONTRACT\_NAME 


```cpp
#define CONTRACT_NAME (
    
) boidnodes
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
The documentation for this class was generated from the following file `contracts/boidnodes/boidnodes.hpp`