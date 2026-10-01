// counter.cpp -- a Phlex "source" (provider) node.
//
// A provider runs once per data cell (think: once per event) and *returns* a
// value.  Phlex stores that value as a "product" that downstream nodes read.
//
// This provider emits a monotonically increasing count: each event's own
// sequence number.  How many events there are (and the first number) is set by
// the driver in the Jsonnet configuration, not here.

#include "phlex/source.hpp"             // provider API + PHLEX_REGISTER_PROVIDERS
#include "phlex/model/data_cell_index.hpp"  // data_cell_index

using namespace phlex;

// PHLEX_REGISTER_PROVIDERS defines the entry point Phlex calls when it loads
// this plugin library.  'm' is the object used to register provider nodes.
PHLEX_REGISTER_PROVIDERS(m)
{
  // Register a provider named "counter".  The lambda receives the current
  // data cell's index; id.number() is its monotonically increasing value.
  m.provide("counter",
            [](data_cell_index const& id) -> int {
              return static_cast<int>(id.number());
            })
    // Label the emitted product (creator="counter", suffix="value",
    // layer="events").  Downstream nodes select it by these three names.
    .output_product("counter", "value", "events");
}
