// squarer.cpp -- a Phlex "transform" node.
//
// A transform reads one or more input products and returns a new product.
// This one reads the integer produced by the counter and returns its square.

#include "phlex/module.hpp"   // algorithm API + PHLEX_REGISTER_ALGORITHMS

using namespace phlex;

// A plain, ordinary C++ function: no framework types, just int -> int.
// Phlex adapts it into a graph node for us.
namespace {
  int square(int x) { return x * x; }
}

// PHLEX_REGISTER_ALGORITHMS defines the entry point for module (non-source)
// nodes.  'm' registers transform/observe/... nodes.
PHLEX_REGISTER_ALGORITHMS(m)
{
  m.transform("squarer", square, concurrency::unlimited)
    // Read the product (creator="counter", layer="events", suffix="value").
    .input_family(product_selector{.creator = "counter",
                                   .layer = "events",
                                   .suffix = "value"})
    // Emit the result with suffix "squared".  Phlex sets the product's creator
    // to this node's label ("squarer") and keeps it on the same "events" layer.
    .output_product_suffixes("squared");
}
