# HashMap
* Hash map implementation using [Hopscotch hashing](https://en.wikipedia.org/wiki/Hopscotch_hashing)
* Requires $O(n)$ memory for n elements, $O(1)$ time for 1 query.

# How to use
* clone this repo
* include ```HashMap.h``` (see example below)

# Code example: 
```C++

#include <iostream>
#include "HashMap.h"

HashMap <string, string> hash_map;
hash_map["Hello"] = "world!";
hash_map["name"] = "Aydar";
if (hash_map.count("Hello")) {
  std::cout << "Hello " << hash_map["Hello"] << '\n';
}
for (auto element: hash_map) {
   std::cout << element.first << " " << element.second << '\n';
}
```
# Fine tuning
You can change HashMap hyperparameters in file ```"HashMap.h"```:
* max_load_factor (optimal = 0.70): the percentage at which the HashMap will increase its capacity,  <b>make it smaller to speed up answer time, but increase the HashMap capacity</b>
* capacity_multiplier (optimal = 2.0): coefficient by which the memory size will increase, <b>make it bigger to speed up answer time, but increase the HashMap capacity </b>
