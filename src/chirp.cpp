// chirp.cpp -- a Phlex "observe" node.
//
// An observer reads a product and performs a side effect (here, printing).
// It returns nothing, so it is a leaf (end-cap) of the data-flow graph.

#include "phlex/module.hpp"   // algorithm API + PHLEX_REGISTER_ALGORITHMS

#include <iostream>

using namespace phlex;

PHLEX_REGISTER_ALGORITHMS(m)
{
  // concurrency::serial because std::cout is shared state: run one at a time so
  // the printed lines do not interleave.
  m.observe("chirp",
            [](int squared) {
              std::cout << "chirp: " << squared << std::endl;
            },
            concurrency::serial)
    // Read the product emitted by the "squarer" node.
    .input_family(product_selector{.creator = "squarer",
                                   .layer = "events",
                                   .suffix = "squared"});
}
