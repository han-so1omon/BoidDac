
# File boidteams.hpp


[**File List**](files.md) **>** [**boidteams**](dir_885226d501ced227e3fb077d4dccbffb.md) **>** [**boidteams.hpp**](boidteams_8hpp.md)

[Go to the source code of this file.](boidteams_8hpp_source.md)

_Manage teams._ [More...](#detailed-description)

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
|  bool | [**accountIsInTeam**](boidteams_8hpp.md#function-accountisinteam) (name accountContractOwner, name nodename, string teamname, name acctname) <br>_Check if account is in team._  |
|  void | [**addTeamByName**](boidteams_8hpp.md#function-addteambyname) (name leader, string teamname, name nodename) <br>_Insert named team into team table._  |
|  ACTION | [**addaccount**](boidteams_8hpp.md#function-addaccount) (name accountContractOwner, name nodename, string teamname, name acct, name teamleader) <br>_add account to team_  |
|  ACTION | [**create**](boidteams_8hpp.md#function-create) (name accountContractOwner, name nodename, string teamname, name owner) <br>_create team_  |
|  ACTION | [**erase**](boidteams_8hpp.md#function-erase) (name accountContractOwner, name nodename, string teamname, name leader) <br>_erase team_  |
|  ACTION | [**eraseaccount**](boidteams_8hpp.md#function-eraseaccount) (name accountContractOwner, name nodename, string teamname, name acct, name teamleader) <br>_erase team_  |
|  uint64\_t | [**getAvailableTeamHash**](boidteams_8hpp.md#function-getavailableteamhash) (name nodename, uint64\_t hash) <br>_Get available hash for team table._  |
|  auto | [**getTeamItr**](boidteams_8hpp.md#function-getteamitr) (T \* dummy, string teamname, name nodename) <br>_Get team by name._  |
|  string | [**memberId**](boidteams_8hpp.md#function-memberid) (string teamname, name acctname) <br>_create unique member id for team member_  |
|  void | [**removeTeamByName**](boidteams_8hpp.md#function-removeteambyname) (name leader, string teamname, name nodename) <br>_Remove named team from team table._  |
|  string | [**teamId**](boidteams_8hpp.md#function-teamid) (name nodename, string teamname) <br>_create unique team id in global scope_  |
|  bool | [**teamIsInNode**](boidteams_8hpp.md#function-teamisinnode) (name nodename, string teamname) <br>_Check if team is in node._  |







## Macros

| Type | Name |
| ---: | :--- |
| define  | [**CONTRACT\_NAME**](boidteams_8hpp.md#define-contract-name) () () boidteams<br> |
| define  | [**DAPPSERVICES\_ACTIONS**](boidteams_8hpp.md#define-dappservices-actions) ()  ()<br> |
| define  | [**DAPPSERVICE\_ACTIONS\_COMMANDS**](boidteams_8hpp.md#define-dappservice-actions-commands) () () IPFS\_SVC\_COMMANDS()LOG\_SVC\_COMMANDS()<br> |

## Public Functions Documentation


### function accountIsInTeam 


```cpp
bool accountIsInTeam (
    name accountContractOwner,
    name nodename,
    string teamname,
    name acctname
) 
```




**Parameters:**


* **accountContractOwner** - owner of accounts contract for table check 
* **nodename** - name of node to associate with team 
* **teamname** - name of team to check 
* **acctname** - name of account to check 



**Returns:**

true if account is in team, else false 





        

### function addTeamByName 


```cpp
void addTeamByName (
    name leader,
    string teamname,
    name nodename
) 
```




**Parameters:**


* **leader** - team leader 
* **teamname** - name of team 
* **nodename** - node container 



**Returns:**

true if team successfully added to team table 





        

### function addaccount 


```cpp
ACTION addaccount (
    name accountContractOwner,
    name nodename,
    string teamname,
    name acct,
    name teamleader
) 
```




**Parameters:**


* **accountContractOwner** - owner of boid account contract 
* **nodename** - name of node that team belongs to 
* **teamname** - name of team to add account to 
* **acct** - account to add to team Wparam teamleader - leader of team 




        

### function create 


```cpp
ACTION create (
    name accountContractOwner,
    name nodename,
    string teamname,
    name owner
) 
```




**Parameters:**


* **accountContractOwner** - owner of boid account contract 
* **nodename** - name of node that team will belong to 
* **teamname** - name of team to create 
* **owner** - team owner account 




        

### function erase 


```cpp
ACTION erase (
    name accountContractOwner,
    name nodename,
    string teamname,
    name leader
) 
```




**Parameters:**


* **accountContractOwner** - owner of boid account contract 
* **nodename** - name of node that team belongs to 
* **teamname** - name of team to erase 
* **leader** - team leader account 




        

### function eraseaccount 


```cpp
ACTION eraseaccount (
    name accountContractOwner,
    name nodename,
    string teamname,
    name acct,
    name teamleader
) 
```




**Parameters:**


* **accountContractOwner** - owner of boid account contract 
* **nodename** - name of node that team belongs to 
* **teamname** - name of team to add account to 
* **acct** - account to add to team 
* **teamleader** - team leader account 




        

### function getAvailableTeamHash 


```cpp
uint64_t getAvailableTeamHash (
    name nodename,
    uint64_t hash
) 
```




**Parameters:**


* **nodename** - name of node to check for hashes 
* **hash** - starting point for search 



**Returns:**

available hash value 





        

### function getTeamItr 


```cpp
template<typename T typename T>
auto getTeamItr (
    T * dummy,
    string teamname,
    name nodename
) 
```




**Parameters:**


* **dummy** variable to deduce return type 
* **teamname** - name of team 
* **nodename** - name of node 



**Returns:**

iterator of team from teams table 





        

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



**Returns:**

unique member id in a team 





        

### function removeTeamByName 


```cpp
void removeTeamByName (
    name leader,
    string teamname,
    name nodename
) 
```




**Parameters:**


* **leader** - team leader 
* **teamname** - name of team 
* **nodename** - node container 




        

### function teamId 


```cpp
inline string teamId (
    name nodename,
    string teamname
) 
```




**Parameters:**


* **nodename** - name of node for team 
* **teamname** - name of team 



**Returns:**

unique team id in global scope 





        

### function teamIsInNode 


```cpp
bool teamIsInNode (
    name nodename,
    string teamname
) 
```




**Parameters:**


* **nodename** - name of node to check 
* **teamname** - name of team to check 



**Returns:**

true if team in node, else false 





        ## Macro Definition Documentation



### define CONTRACT\_NAME 


```cpp
#define CONTRACT_NAME (
    
) boidteams
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
The documentation for this class was generated from the following file `contracts/boidteams/boidteams.hpp`