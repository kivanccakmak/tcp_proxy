# Overview

1. **Problem Statement:** Additional Increase Multiplicative Decrease(AIMD) approach of Transmission Control Protocol(TCP) 
suffers when multiple packet losses exists in significantly lossy link. 

2. **Solution:** Split TCP connection in between source and destination, use multiple TCP connections in lossy link.

3. **How is it so:** Pair proxy. One of the backbone node(*transmitter proxy*) splits TCP connection in between source and destination. 
Consequently passes hijacked data to -instead of destination- second node(*receiver proxy*) via multiple TCP connections. 
Then, receiver proxy demultiplexes incoming data and finally passes towards the destination.

![] (doc/figs/proxy_topo.bmp)

# usage

This code is for a bit messy setup, so if you aren't crazy for TCP session hijacking and performance enhancement, I advice you to pass. 
Otherwise, I strongly advice you to read all below!

As illustrated above, we assume that the experiment needs **4** devices - *commands provided with IP addresses of figure*.
We also assume that proxy devices have two different interfaces, one for getting incoming data and one for forwarding data - *commands
below provided with ethernet interfaces(eth0, eth1) but those can be anything*.

### configuration
Connect devices as in topology figure and copy repo to all of them. Consequently,
if you would use different ip addresses, change **network.conf** file, which is in root directory of repo. 

* ***stream*** -> Agnostic Source
* ***dest*** -> Agnostic Destination
* ***tx\_proxy*** -> Transmitter Proxy
* ***rx\_proxy*** -> Receiver Proxy

#### add file 
provide raw file to forward into **stream/** directory of source device and set it's name 
into **file\_name** variable in **network.conf**.

### brctl usage
Both of the proxy nodes should bridge multiple network interfaces via **brctl**. Below, I provide commands for **tx\_proxy**, but it should be done for **rx\_proxy** as well.

* `sudo ifconfig eth0 down`
* `sudo ifconfig eth1 down`
* `sudo brctl addbr br0`
* `sudo brctl addif br0 eth0`
* `sudo brctl addif br0 eth1 `
* `sudo ifconfig eth0 up`
* `sudo ifconfig eth1 up`
* `sudo ifconfig br0 up`
* `sudo ifconfig br0 192.168.2.201`

### iptables usage
Transmitter proxy should define routing rules via **iptables**.

* `iptables -t mangle -F`
* `iptables -t nat -F`
* `iptables -t nat -I PREROUTING -p tcp -s 192.168.2.11 -j REDIRECT --to-port 5000`
* `iptables -t mangle -I PREROUTING -i br0 -s 192.168.2.11 -p tcp --syn -j NFQUEUE --queue-num=0`

### compile
`make`

#### run
First, connect to all devices(via ssh or sth. else) Consequently, run binaries in devices respectively:

1. ***./receive***
2. ***./rx\_proxy***
3. ***sudo ./tx\_proxy***
4. ***./stream***

## working principles

### tcp session hijacking

netfilter library is used to get hijacked TCP packets from kernel space to user space, which would then be passed
to second proxy node via multiple connections.

### packet reordering 

![] (doc/figs/encaps.bmp)

Second node receives packets from multiple connections. Those may not arrive sequentially due to losses in the medium. 
For this reason, we add our own headers to TCP packets in between pair proxy nodes, which are joint sequence numbers of 
multiple TCP connections.

# doxygen

`make doxygen`
