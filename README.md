# Demystifying and Improving Lazy Promotion in Cache Eviction [Experiment, Analysis & Benchmark]

This repository contains the source code and experimental data for the paper "Demystifying and Improving Lazy Promotion in Cache Eviction".

## Repository Structure

The project is organized as follows:

- `simulator/`: Contains the implementation of various lazy promotion techniques on a cache simulator based on [libCacheSim](https://github.com/1a1a11a/libCacheSim).
- `scripts/`: Contains Python scripts to parse and process datasets, and to generate figures for the paper.
- `data/`: Contains the raw data from our experiments.
- `figures/`: Contains the figures generated from the scripts.

## Environment Setup

Tested on **Ubuntu 24.02**

### Dependencies

The simulator depends on the following tools and libraries:

- `cmake`
- `build-essential`
- `libglib2.0-dev`
- `libgoogle-perftools-dev` (tcmalloc)
- `zstd`

You can install these dependencies using the provided script:

```bash
cd simulator/scripts
bash install_dependency.sh
```

This script supports Ubuntu, CentOS, and macOS.

### Building the Simulator

After installing the dependencies, you can build the simulator:

```bash
mkdir _build
cd _build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

The simulator executable `cachesim` will be available in the `simulator/_build/bin/` directory.

## Reproducing Experiment Results

The following sections describe how to reproduce the experiment results from the paper.

### Running Simulations

All simulations are run using the `cachesim` executable. The general command format is:

```bash
./_build/bin/cachesim [trace_path] [trace_type] [algorithm] -e [eviction_param] [cache_size] --ignore-obj-size 1
```

The traces used in the paper are available in https://ftp.pdl.cmu.edu/pub/datasets/twemcacheWorkload/cacheDatasets/. The `trace_type` is `oracleGeneral`.

Here are the commands for each algorithm discussed in the paper:

**1. FIFO**
```bash
./_build/bin/cachesim $file oracleGeneral fifo 0.01 --ignore-obj-size 1
```

**2. LRU**
```bash
./_build/bin/cachesim $file oracleGeneral lru 0.01 --ignore-obj-size 1
```

**3. Prob-LRU**
```bash
./_build/bin/cachesim $file oracleGeneral lru-prob -e prob=0.5 0.01 --ignore-obj-size 1
```

**4. Delay-LRU**
```bash
./_build/bin/cachesim $file oracleGeneral lru-delay -e delay-time=0.2 0.01 --ignore-obj-size 1
```

**5. Batch**
```bash
./_build/bin/cachesim $file oracleGeneral batch -e batch-size=0.5 0.01 --ignore-obj-size 1
```

**6. FIFO-reinsertion(FR) / CLOCK**
```bash
./_build/bin/cachesim $file oracleGeneral clock -e n_bit_counter=1 0.01 --ignore-obj-size 1
```

**7. RandomK**
```bash
./_build/bin/cachesim $file oracleGeneral randomK -e k=5 0.01 --ignore-obj-size 1
```

**8. Belady-FR**
```bash
./_build/bin/cachesim $file oracleGeneral beladyclock -e scaler=0.5 0.01 --ignore-obj-size 1
```

**9. Offline-FR**
```bash
./_build/bin/cachesim $file oracleGeneral offlineFR -e scaler=0.5 0.01 --ignore-obj-size 1
```

**10. Delay-FR**
```bash
./_build/bin/cachesim $file oracleGeneral delayfr -e delay-ratio=0.05 0.01 --ignore-obj-size 1
```

**11. AGE**
```bash
./_build/bin/cachesim $file oracleGeneral age -e scaler=0.4 0.01 --ignore-obj-size 1
```
### Hardware
We recommend running this experiments on hardware with at least 256 GB of RAM. It is required to run some larger traces.

### Cluster Experiment
We ran our experiments using [distComp](https://github.com/1a1a11a/distComp).
Utilizing its capability in managing hundred-thousands of experiments across multiple nodes.
we provided scripts for that in `scripts/generate_task.sh`.

### Experiment Size and Duration
Considering that running all the experiments included in the paper would
consume a lot of spaces to store the traces and take at least half a day using 15 nodes (64 core, 376 GB RAM).
We also provide the simulation result in libCacheSim format on `data/`.

### Analyzing Results

The `scripts/` directory contains Python scripts for analyzing the simulation results and generating the figures used in the paper.

- `age_parser.py`: Parses the output of the AGE algorithm simulations.
- `common.py`: Common utility functions for the parsing scripts.
- `docs_writer.py`: Generates documentation from the results.
- `generate_figures.py`: Generates the figures.
- `matplotlib_wrapper.py` and `plotly_wrapper.py`: Wrappers for plotting libraries.
- `other_parser.py`: Parses the output of other algorithms.
- `outputs_parser.py`: A generic output parser.
- `utils.py`: Utility functions.

To generate the figures, you can run the `generate_figures.py` script. You may need to install the required Python packages listed in `scripts/requirements.txt`.
