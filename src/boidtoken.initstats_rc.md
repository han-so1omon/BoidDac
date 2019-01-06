# Action - `{{ initstats }}`

This Contract is legally binding and can be used in the event of a dispute. Disputes shall be settled through the standard arbitration process established by EOS.IO.

### Description

The `{{ initstats }}` action... 

INTENT: The intent of `{{ initstats }}` is for the owner to initialize the parameters that influence the staking rewards equation and to initialize the values that track user participation amounts and staked account counts. It is intended to be executed once after the initial deployment of the contract and before executing the running action to enable contract staking functions. It could also serve to reset the contract for use under a different stake table in the event of some sort of unforeseen contract failure. Execution is restricted to owner only permissions.

TERM: This action lasts for the duration of the processing of the contract.

### Inputs and Input Types

The `{{ initstats }}` action requires the following `inputs` and `input types`:

| Action | Input | Input Type |
|:--|:--|:--|
| `{{ initstats }}` | | |
