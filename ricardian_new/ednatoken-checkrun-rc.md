# Action - `{{ checkrun }}`

This Contract is legally binding and can be used in the event of a dispute. Disputes shall be settled through the standard arbitration process established by EOS.IO.

### Description

The `{{ checkrun }}` action...
INTENT: The intent of checkrun action is to allow the contract owner to review the outcome of an impending execution of the runpayout action. It does not modify any data in tables or otherwise It simply report on the outcome of the current payout forecast for stakers based on current user stake adoption and bonus settings. Execution is restricted to owner only permissions. 
TERM: This action lasts for the duration of the processing of the contract.  

### Input and Input Type

The `{{ checkrun }}` action requires the following `input` and `input type`:

| Action | Input | Input Type |
|:--|:--|:--|
| `{{ checkrun }}` | `{{ _bonusVar }}` | `{{ uint64 }}` |
