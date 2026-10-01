# phlex-skeleton

A tiny, heavily-commented Phlex package meant as a **first tutorial** for
students with only minimal C++ experience.  It depends on nothing but
[Phlex](https://github.com/framework-r-d/phlex) and builds three plugin
"nodes" that are wired together by a Jsonnet configuration file:

```
 counter  ─────►  squarer  ─────►  chirp
 (source)        (transform)      (observer)
 emits 1,2,3…    emits n·n        prints to stdout
```

- **`counter`** — a *source* (provider) node.  Once per event it emits a
  monotonically increasing integer (the event number).
- **`squarer`** — a *transform* node.  It reads the counter's integer and emits
  its square.
- **`chirp`** — an *observer* node.  It reads the squared integer and prints it
  to `std::cout`.

## What a Phlex "node" is

A node is just an ordinary C++ callable (a function or a lambda).  Phlex calls
it for you and moves its result — a **product** — along the edges of a
data-flow graph.  A product is identified by three names:

- **creator** — the node that made it,
- **layer** — the level of the event hierarchy it lives on (here: `events`),
- **suffix** — a short name for the product itself (e.g. `value`, `squared`).

A node declares which products it *consumes* with a `product_selector{...}`
giving those three names, and which products it *produces* via
`.output_product(...)` / `.output_product_suffixes(...)`.  That is the entire
wiring mechanism — matching names connect nodes together.

Each `.cpp` file in `src/` registers one node through a macro:

- `PHLEX_REGISTER_PROVIDERS` — for source nodes (link `phlex::source`),
- `PHLEX_REGISTER_ALGORITHMS` — for transform/observer nodes (link `phlex::module`).

## Files

```
phlex-skeleton/
├── CMakeLists.txt            # builds the three plugin libraries
├── src/
│   ├── counter.cpp           # source  : emits a monotonic count
│   ├── squarer.cpp           # transform: emits the square of its input
│   └── chirp.cpp             # observer : prints its input to stdout
├── config/
│   └── skeleton.jsonnet       # wires counter → squarer → chirp
└── README.md
```

Each node's library name is `lib<name>.so`, and `<name>` is exactly the string
you write as `cpp: '<name>'` in the Jsonnet config.  So `src/squarer.cpp`
builds `libsquarer.so`, referenced as `cpp: 'squarer'`.

## Building

Phlex and a C++23 compiler must be available.  In this project that comes from
activating one of the run-time environments (e.g. `cd runs/envs/gcc15`, which
`direnv` activates), after which `phlex`, `cmake` and `g++` are all on your
`PATH`.  Then configure and build out-of-source:

```bash
cd devel/phlex-skeleton
cmake -G Ninja -S . -B build
cmake --build build
```

This produces `build/libcounter.so`, `build/libsquarer.so` and
`build/libchirp.so`.

> **Note.** The `CMakeLists.txt` calls `find_package(Boost COMPONENTS json)`,
> `TBB`, `spdlog` and `fmt` *before* `find_package(phlex)`.  That ordering is a
> required workaround because Phlex's exported CMake config does not pull in its
> own transitive dependencies; without it CMake errors with "target not found".

## Running

Phlex finds plugin libraries by scanning the colon-separated `PHLEX_PLUGIN_PATH`
for `lib<name>.so`.  It needs two directories: our freshly-built `build/`, and
the directory holding Phlex's own bundled plugins (such as `generate_layers`,
the driver used in the config).  The second one sits next to the `phlex`
binary:

```bash
cd devel/phlex-skeleton

# our plugins + Phlex's own bundled plugins (generate_layers lives here)
PHLEXLIB="$(dirname "$(readlink -f "$(command -v phlex)")")/../lib"
export PHLEX_PLUGIN_PATH="$PWD/build:$PHLEXLIB"

phlex -c config/skeleton.jsonnet
```

Expected output (5 events → the squares of 1..5; the order varies because the
graph runs in parallel):

```
chirp: 1
chirp: 4
chirp: 9
chirp: 16
chirp: 25
```

Change `total: 5` in `config/skeleton.jsonnet` to run more or fewer events —
the event count lives in the config's driver, not on the command line.

## Things to try

These exercises grow the package one small step at a time.  Each "add a node"
exercise is the same three-part recipe: **write a `.cpp`, add it to CMake, add
it to the config.**

### 1. Change existing behaviour (no new files)

- Make `chirp` print something friendlier, e.g.
  `std::cout << "the square is " << squared << "\n";`.
- Make `squarer` cube instead of square: `return x * x * x;` (rename things if
  you like it to stay honest).
- Run more events by raising `total` in `config/skeleton.jsonnet`.

### 2. Add a brand-new algorithm (the main exercise)

Add an **`adder`** transform that reads the counter's `value` and emits
`value + 100`.

1. **Write `src/adder.cpp`** (copy `src/squarer.cpp` and adjust):

   ```cpp
   #include "phlex/module.hpp"
   using namespace phlex;

   namespace {
     int add_offset(int x) { return x + 100; }
   }

   PHLEX_REGISTER_ALGORITHMS(m)
   {
     m.transform("adder", add_offset, concurrency::unlimited)
       .input_family(product_selector{.creator = "counter",
                                      .layer = "events",
                                      .suffix = "value"})
       .output_product_suffixes("plus100");
   }
   ```

2. **Add it to `CMakeLists.txt`**:

   ```cmake
   add_library(adder MODULE src/adder.cpp)
   target_link_libraries(adder PRIVATE phlex::module)
   ```

3. **Wire it in `config/skeleton.jsonnet`** by adding to the `modules:` map:

   ```jsonnet
   adder: { cpp: 'adder' },
   ```

   At this point `adder` runs but nothing reads its `plus100` product.  To see
   it, point a second observer at it, or change `chirp` to read
   `{.creator = "adder", .layer = "events", .suffix = "plus100"}` instead.

4. **Rebuild and run** (`cmake --build build`, then `phlex -c ...`).

### 3. Add a second observer

Copy `src/chirp.cpp` to `src/chirp_raw.cpp`, rename the node to `chirp_raw`,
and have it read the *counter's* product directly
(`{.creator = "counter", .layer = "events", .suffix = "value"}`) so you can see
the input and the output side by side.  Add the CMake target and a
`chirp_raw: { cpp: 'chirp_raw' }` entry in the config.

### 4. Pass a parameter through the config

Config values reach a node through its registration macro.  Change the macro to
take a config argument and read a parameter:

```cpp
PHLEX_REGISTER_ALGORITHMS(m, config)
{
  auto const offset = config.get<int>("offset", 100);   // default 100
  m.transform("adder", [offset](int x) { return x + offset; }, concurrency::unlimited)
    .input_family(product_selector{.creator = "counter", .layer = "events", .suffix = "value"})
    .output_product_suffixes("plus_offset");
}
```

Then set it in the config: `adder: { cpp: 'adder', offset: 42 }`.

---

### Quick reference

| Node kind   | Macro                       | Link against     | Reads input?    | Writes output?            |
|-------------|-----------------------------|------------------|-----------------|---------------------------|
| source      | `PHLEX_REGISTER_PROVIDERS`  | `phlex::source`  | no (per event)  | `.output_product(...)`    |
| transform   | `PHLEX_REGISTER_ALGORITHMS` | `phlex::module`  | `.input_family` | `.output_product_suffixes`|
| observer    | `PHLEX_REGISTER_ALGORITHMS` | `phlex::module`  | `.input_family` | no (returns `void`)       |
