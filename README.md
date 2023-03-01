# HashMap
* Hash map implementation using [Hopscotch hashing](https://en.wikipedia.org/wiki/Hopscotch_hashing)
* Requires $O(n)$ memory for n elements, $O(1)$ time for 1 query.

# How to use
* clone this repo
* include ```hash_map.h``` (see example below)

# Code example: 
```C++

#include <iostream>
#include "hash_map.h"

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
