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
<#nodes> <Z> <Tx> <Sim Time> <m>
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
* ***m*** initial set size for Barabase-Albert Algorithm 

## Logs and Graphs

For getting the logs and graphs, please make `dumps` and `dots` directorys in your working directory. 

In the `dumps` folder, the i-th peer will dump their trees at the end of the simulation in the file `i.dump`.

A file named `results.dump` will also be generated in dumps which will have the following information:
```
Peer ID,<ComutePower>,<peer's blocks>,<in longest chain>,<% in longest chain>,<orphans received>
```


In the `dots` folder we generate `.dot` files which are in the `DOT LANGUAGE`, which can be used to visualize the trees.

Now, run the binary in the same directory. To see the results.