# Top-k Document Retrieval

    Top-k document retrieval is a full-text index. Given a pattern, it can be a word, a phase or 
    arbitrary substring, you can get the most relevant documents with it. 

## Requirements

* To compile the code, you need to install the [sdsl](https://github.com/simongog/sdsl-lite)
  library.
* `g++`version 4.9 or higher.

## Installation

To download and compile the code use the following commands.

```Bash
git clone https://github.com/pucika/top-k-document-retrieval
cd top-k-document/src
make
```

## Example

The doc/ directory contains the document set.

```cpp
#include <iostream>
#include "gst.h"

using namespace top_k;

int main(int argc, char* argv[])
{
  GST gst;
  gst.constructure("../docs/");
  string pattern;
  int k = 0;
  while(true) {
    getline(cin, pattern);
    std::cin >> k;
    std::cin.ignore();
    get.search(pattern, k);
  }
  return 0;
}
```
