WHTAP
=======

WHTAP is a HTAP prototype, which was originally proposed in the following paper.

    Liang Li, Gang Wu, Guoren Wang, Ye Yuan: Accelerating Hybrid Transactional/Analytical Processing 
    Using Consistent Dual-Snapshot. DASFAA (1) 2019: 52-69
    
Code-Branches
------------

- _master_ branch is the WHTAP impementation, NOTE that CC_ALG must be set as TICTOC, since WHTAP is based on TICTOC
- _compare_ branch is some related algorithms.

Build
------------

To build the database.

    make -j
    
Configuration
-------------

DBMS configurations can be changed in the config.h file. Please refer to README for the meaning of each configuration. Here we only list several most important ones. 

    THREAD_CNT        : Number of worker threads running in the database.
    WORKLOAD          : Supported workloads include YCSB and TPCC
    CC_ALG            : Concurrency control algorithm. Seven algorithms are supported 
                        (DL_DETECT, NO_WAIT, HEKATON, SILO, TICTOC) 
    MAX_TXN_PER_PART  : Number of transactions to run per thread per partition.
                        
Configurations can also be specified as command argument at runtime. Run the following command for a full list of program argument. 
    
    ./rundb -h

Run
---

The DBMS can be run with 

    ./rundb -r0.5 -w0.5 -z0.9 -R16 -l3 -t16 -x8


Acknowledgment to [DBx1000](https://github.com/yxymit/DBx1000)
------------

First of all, WHTAP is base on the DBx1000 project, which is an single node OLTP database management system (DBMS). The goal of DBx1000 is to make DBMS scalable on future 1000-core processors. We implemented all the seven classic concurrency control schemes in DBx1000. They exhibit different scalability properties under different workloads. The concurrency control scalability study is described in the following paper. 

    Staring into the Abyss: An Evaluation of Concurrency Control with One Thousand Cores
    Xiangyao Yu, George Bezerra, Andrew Pavlo, Srinivas Devadas, Michael Stonebraker
    http://www.vldb.org/pvldb/vol8/p209-yu.pdf
