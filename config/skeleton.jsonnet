// skeleton.jsonnet -- wires the three nodes into a data-flow graph.
//
// Run with:   phlex -c skeleton.jsonnet
// (after setting PHLEX_PLUGIN_PATH -- see README.md).
{
  // Every Phlex job (since Phlex 0.4) must name its "stage": a label for this
  // processing step that is stamped on every product the job makes.  (It can
  // also be given on the command line as `phlex --stage <name> ...`.)
  stage: 'skeleton',

  // The driver creates the data cells (events) that drive the graph.
  // 'generate_layers' is a standard plugin that ships with Phlex.  Here we make
  // 5 events on a layer named "events", numbered 1..5.
  driver: {
    cpp: 'generate_layers',
    layers: {
      events: { parent: 'job', total: 5, starting_number: 1 },
    },
  },

  // Sources produce products with no input.  The map key ('counter') is the
  // node's label; 'cpp' names the plugin library to load (libcounter.so).
  sources: {
    counter: { cpp: 'counter' },
  },

  // Modules read products and may produce new ones.  The map key is the node's
  // label; Phlex uses it as the "creator" of that node's output products, which
  // is how downstream nodes refer back to it.
  modules: {
    squarer: { cpp: 'squarer' },
    chirp: { cpp: 'chirp' },
  },
}
