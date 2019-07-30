
# File boidpower.hpp


[**File List**](files.md) **>** [**boidpower**](dir_4fa9b7c4a3edefd214ebf5845c852217.md) **>** [**boidpower.hpp**](boidpower_8hpp.md)

[Go to the source code of this file.](boidpower_8hpp_source.md)

_Manage devices and associated boidpower._ [More...](#detailed-description)

* `#include <vector>`
* `#include <functional>`
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
|  bool | [**accountExists**](boidpower_8hpp.md#function-accountexists) (name accountContractOwner, name acctname) <br>_Check if account exists._  |
|  void | [**addDeviceByName**](boidpower_8hpp.md#function-adddevicebyname) (name owner, string devname) <br>_Insert named device into device table._  |
|  ACTION | [**assignpower**](boidpower_8hpp.md#function-assignpower) (string devname, uint64\_t quantity) <br>_Assign device to boidpower quantity._  |
|  ACTION | [**changeowner**](boidpower_8hpp.md#function-changeowner) (name accountContractOwner, name owner, string devname) <br> |
|  ACTION | [**create**](boidpower_8hpp.md#function-create) (name accountContractOwner, name owner, string devname) <br>_Create device._  |
|  ACTION | [**erase**](boidpower_8hpp.md#function-erase) (name owner, string devname) <br>_Erase device._  |
|  ACTION | [**freedevice**](boidpower_8hpp.md#function-freedevice) (name accountContractOwner, name owner, string devname) <br>_Free device from owner._  |
|  ACTION | [**freeze**](boidpower_8hpp.md#function-freeze) (name owner, string devname, bool freeze) <br>_Freeze device from contributing._  |
|  uint64\_t | [**getAvailableDeviceHash**](boidpower_8hpp.md#function-getavailabledevicehash) (uint64\_t hash) <br>_Get available hash for device table._  |
|  auto | [**getDeviceItr**](boidpower_8hpp.md#function-getdeviceitr) (T \* dummy, string devname) <br>_Get device by name._  |
|  ACTION | [**open**](boidpower_8hpp.md#function-open) (name owner, string devname, bool open) <br>_Open device to changes._  |
|  void | [**removeDeviceByName**](boidpower_8hpp.md#function-removedevicebyname) (name owner, string devname) <br>_Remove named device from device table._  |







## Macros

| Type | Name |
| ---: | :--- |
| define  | [**CONTRACT\_NAME**](boidpower_8hpp.md#define-contract-name) () () boidpower<br> |
| define  | [**DAPPSERVICES\_ACTIONS**](boidpower_8hpp.md#define-dappservices-actions) ()  ()<br> |
| define  | [**DAPPSERVICE\_ACTIONS\_COMMANDS**](boidpower_8hpp.md#define-dappservice-actions-commands) () () IPFS\_SVC\_COMMANDS()LOG\_SVC\_COMMANDS()<br> |
| define  | [**POWER\_SOURCE\_BOINC**](boidpower_8hpp.md#define-power-source-boinc)  () 1<br>_BOINC boidpower source._  |
| define  | [**POWER\_SOURCE\_MINING**](boidpower_8hpp.md#define-power-source-mining)  () 0<br>_Crypto mining boidpower source._  |
| define  | [**POWER\_SOURCE\_VRAM**](boidpower_8hpp.md#define-power-source-vram)  () 2<br>_vRam boidpower source_  |

## Public Functions Documentation


### function accountExists 


```cpp
bool accountExists (
    name accountContractOwner,
    name acctname
) 
```




**Parameters:**


* **accountContractOwner** - owner of boidaccounts contract 
* **acctname** - name of account to check 



**Returns:**

true if account exists, else false 





        

### function addDeviceByName 


```cpp
void addDeviceByName (
    name owner,
    string devname
) 
```




**Parameters:**


* **owner** - owner of device 
* **devname** - name of device 



**Returns:**

true if device successfully added to device table 





        

### function assignpower 


```cpp
ACTION assignpower (
    string devname,
    uint64_t quantity
) 
```




**Parameters:**


* **devname** - name of device to assign power 
* **quantity** - amount of power 




        

### function changeowner 


```cpp
ACTION changeowner (
    name accountContractOwner,
    name owner,
    string devname
) 
```


Wbrief Change device owner 

**Parameters:**


* **accountContractOwner** - owner of boidaccounts contract` 
* **owner** - new device owner 
* **devname** - name of device to change 




        

### function create 


```cpp
ACTION create (
    name accountContractOwner,
    name owner,
    string devname
) 
```


Devices are globally unique (atomic) objects in the boid network. Devices are responsible for computations on BOID network Devices + boidpower serve as a form of KYC

**Parameters:**


* **accountContractOwner** - owner of boidaccounts contract 
* **owner** - device owner 
* **devname** - name of device 




        

### function erase 


```cpp
ACTION erase (
    name owner,
    string devname
) 
```




**Parameters:**


* **owner** - owner of device 
* **devname** - name of device to erase 




        

### function freedevice 


```cpp
ACTION freedevice (
    name accountContractOwner,
    name owner,
    string devname
) 
```




**Parameters:**


* **accountContractOwner** - owner of boidaccounts contract 
* **owner** - owner of device to free 
* **devname** - name of device to free 




        

### function freeze 


```cpp
ACTION freeze (
    name owner,
    string devname,
    bool freeze
) 
```




**Parameters:**


* **owner** - owner of device to freeze 
* **devname** - name of device to freeze 
* **freeze** - whether to freeze or unfreeze device 




        

### function getAvailableDeviceHash 


```cpp
uint64_t getAvailableDeviceHash (
    uint64_t hash
) 
```




**Parameters:**


* **hash** - starting point for search 



**Returns:**

available hash value 





        

### function getDeviceItr 


```cpp
template<typename T typename T>
auto getDeviceItr (
    T * dummy,
    string devname
) 
```




**Parameters:**


* **dummy** - dummy variable to deduce return type 
* **devname** - name of device 



**Returns:**

iterator of device from device table 





        

### function open 


```cpp
ACTION open (
    name owner,
    string devname,
    bool open
) 
```




**Parameters:**


* **owner** - owner of device to open 
* **devname** - name of device to open 
* **open** - whether to open or close device 




        

### function removeDeviceByName 


```cpp
void removeDeviceByName (
    name owner,
    string devname
) 
```




**Parameters:**


* **owner** - owner of device 
* **devname** - name of device 



**Returns:**

true if device successfully added to device table 





        ## Macro Definition Documentation



### define CONTRACT\_NAME 


```cpp
#define CONTRACT_NAME (
    
) boidpower
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



### define POWER\_SOURCE\_BOINC 


```cpp
#define POWER_SOURCE_BOINC () 
```



### define POWER\_SOURCE\_MINING 


```cpp
#define POWER_SOURCE_MINING () 
```



### define POWER\_SOURCE\_VRAM 


```cpp
#define POWER_SOURCE_VRAM () 
```



------------------------------
The documentation for this class was generated from the following file `contracts/boidpower/boidpower.hpp`