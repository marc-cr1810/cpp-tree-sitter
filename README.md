# tree-sitter-cmake

CMake project for building [Tree-sitter](http://tree-sitter.github.io/tree-sitter/).

## About

Tree-sitter is an excellent project. It provides a reusable interface for parsing many different programming languages.
But because it's aimed primarily for consumption in Node.js, it uses GYP as the build system.
GYP is terrible for modern-day developers needs.

This project aims to provide an integrated way to use Tree-sitter in CMake projects.
Just use this to build and install Tree-sitter, as well as the CMake config files.

As a bonus, it even includes several Tree-sitter languages in library form.

I originally wrote this to make importing Tree-sitter into a project I am
working on. I decided to release this separate in case anyone finds it useful.

## Getting Started

This project includes Tree-sitter and sub-projects as Git Submodules, so make
sure you add the submodules when you checkout the source.

```bash
git clone --recurse-submodules https://github.com/marc-cr1810/cpp-tree-sitter
mkdir cpp-tree-sitter/build
cd cpp-tree-sitter/build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
sudo cmake --build . --target install
```

The above commands should check out the git submodules this project pulls in.
Otherwise, CMake will check out the git submodules when run.

## Using

```cmake
find_package(Tree-Sitter CONFIG)
# Gives Tree-Sitter::Tree-Sitter

find_package(Tree-Sitter CONFIG REQUIRED CPP Rust)
# Gives Tree-Sitter::Tree-Sitter, Tree-Sitter::Tree-Sitter-CPP, Tree-Sitter::Tree-Sitter-Rust
```

The complete list of targets is:

* `Tree-Sitter::Tree-Sitter`
* `Tree-Sitter::Tree-Sitter-C`
* `Tree-Sitter::Tree-Sitter-CPP`
* `Tree-Sitter::Tree-Sitter-C-Sharp`
* `Tree-Sitter::Tree-Sitter-Go`
* `Tree-Sitter::Tree-Sitter-Java`
* `Tree-Sitter::Tree-Sitter-JavaScript`
* `Tree-Sitter::Tree-Sitter-Python`
* `Tree-Sitter::Tree-Sitter-Rust`
* `Tree-Sitter::Tree-Sitter-TypeScript`

I will probably add more language parsers later.

Also provided is a new header file (`tree_sitter/langs.h`), that contains the
definitions for all the language parsers included.

```c
const TSLanguage *tree_sitter_c();
const TSLanguage *tree_sitter_cpp();
const TSLanguage *tree_sitter_c_sharp();
const TSLanguage *tree_sitter_go();
const TSLanguage *tree_sitter_java();
const TSLanguage *tree_sitter_javascript();
const TSLanguage *tree_sitter_python();
const TSLanguage *tree_sitter_rust();
const TSLanguage *tree_sitter_typescript();
```

Translating the parsing and tree inspection operations from the example to
use the C++ wrappers then yields a `demo.cpp` like:

```cpp
#include <cassert>
#include <cstdio>
#include <memory>
#include <string>

#include <tree_sitter/cpp-tree-sitter.hpp>

int main() {
  // Create a language and parser.
  ts::Language language = tree_sitter_json();
  ts::Parser parser{language};

  // Parse the provided string into a syntax tree.
  std::string sourcecode = "[1, null]";
  ts::Tree tree = parser.parseString(sourcecode);

  // Get the root node of the syntax tree. 
  ts::Node root = tree.getRootNode();

  // Get some child nodes.
  ts::Node array = root.getNamedChild(0);
  ts::Node number = array.getNamedChild(0);

  // Check that the nodes have the expected types.
  assert(root.getType() == "document");
  assert(array.getType() == "array");
  assert(number.getType() == "number");

  // Check that the nodes have the expected child counts.
  assert(root.getNumChildren() == 1);
  assert(array.getNumChildren() == 5);
  assert(array.getNumNamedChildren() == 2);
  assert(number.getNumChildren() == 0);

  // Print the syntax tree as an S-expression.
  auto treestring = root.getSExpr();
  printf("Syntax tree: %s\n", treestring.get());

  return 0;
}
```

In particular, some of the underlying APIs now use method calls for
easier discoverability, and resource cleaning is automatic.

## License

This is nothing more than a simple CMake script and some supporting files.
It is released as Public Domain.

This is not endorsed by the Tree-sitter developers. If they contact me and ask
me to remove this I will.
