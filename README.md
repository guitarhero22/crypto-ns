# crypto-ns

Simulation of a P2P Cryptocurrency Network

## Build

You will need a `build` directory, and `cmake` and `make` installed.

Do the following:
```bash
$ mkdir build
$ cd build
$ cmake .. && cmake --build .
$ ./crytpo-ns <config-file>
```

The executable takes in the path to a config file which contains the simulation parameters in the following format.

```
<#nodes> <Z> <Tx> <Sim Time>
<T_1>
<T_2>
<T_3>
.
.
.
<T_{#nodes}>
```
* ***#node*** is the number of nodes
* ***Z*** is the % of slow nodes
* ***Tx*** is the mean transaction time.
* ***Sim Time*** is the simulation time
* ***T_k*** is the mean of interarrival of blocks for the k-th node

## Logs and Graphs

For getting the logs and graphs, please make `dumps` and `dots` directorys in your working directory. 

Now, run the binary in the same directory.