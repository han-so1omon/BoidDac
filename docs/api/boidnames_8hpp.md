
# File boidnames.hpp


[**File List**](files.md) **>** [**boidcommon**](dir_1379e245553e8cc39a16063d19589c5a.md) **>** [**boidnames.hpp**](boidnames_8hpp.md)

[Go to the source code of this file.](boidnames_8hpp_source.md)



* `#include <inttypes.h>`
* `#include "defines.hpp"`















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint64\_t | [**boidDeviceNameHash**](boidnames_8hpp.md#function-boiddevicenamehash) (string s, uint64\_t seed=0) <br>_Device name hash using Paul Larson's simple hash_ [https://stackoverflow.com/questions/98153/whats-the-best-hashing-algorithm-to-use-on-a-stl-string-when-using-hash-map](https://stackoverflow.com/questions/98153/whats-the-best-hashing-algorithm-to-use-on-a-stl-string-when-using-hash-map) _._ |
|  uint64\_t | [**boidTeamNameHash**](boidnames_8hpp.md#function-boidteamnamehash) (string s, uint64\_t seed=7) <br>_Team name hash using Paul Larson's simple hash_ [https://stackoverflow.com/questions/98153/whats-the-best-hashing-algorithm-to-use-on-a-stl-string-when-using-hash-map](https://stackoverflow.com/questions/98153/whats-the-best-hashing-algorithm-to-use-on-a-stl-string-when-using-hash-map) _._ |
|  bool | [**boidValidDeviceName**](boidnames_8hpp.md#function-boidvaliddevicename) (string s) <br>_Check validity of name for device._  |
|  bool | [**boidValidNodeName**](boidnames_8hpp.md#function-boidvalidnodename) (string s) <br>_Check validity of name for node._  |
|  bool | [**boidValidTeamName**](boidnames_8hpp.md#function-boidvalidteamname) (string s) <br>_Check validity of name for team._  |








## Public Functions Documentation


### function boidDeviceNameHash 


```cpp
uint64_t boidDeviceNameHash (
    string s,
    uint64_t seed=0
) 
```




**Parameters:**


* **s** - devname 
* **seed** - seed for randomness 



**Returns:**

name hash 





        

### function boidTeamNameHash 


```cpp
uint64_t boidTeamNameHash (
    string s,
    uint64_t seed=7
) 
```




**Parameters:**


* **s** - teamname 
* **seed** - seed for randomness 



**Returns:**

name hash 





        

### function boidValidDeviceName 


```cpp
bool boidValidDeviceName (
    string s
) 
```




**Parameters:**


* **s** - devname 



**Returns:**

true if valid name, else false 





        

### function boidValidNodeName 


```cpp
bool boidValidNodeName (
    string s
) 
```




**Parameters:**


* **s** - nodename 



**Returns:**

true if valid name, else false 





        

### function boidValidTeamName 


```cpp
bool boidValidTeamName (
    string s
) 
```




**Parameters:**


* **s** - teamname 



**Returns:**

true if valid name, else false 





        

------------------------------
The documentation for this class was generated from the following file `contracts/boidcommon/boidnames.hpp`