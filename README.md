1. **Problem Statement:** Additional Increase Multiplicative Decrease(AIMD) approach of Congestion Control(CN) algorithms in 
Transmission Control Protocol(TCP) suffers when multiple packet losses exists in the medium. 

2. **Solution:** Proxy real connection and manage multiple TCP connections in between two nodes of backbone.
3. **How is it so:** Pair proxy. One of them(transmitter proxy) splits TCP connection in between the original source and receiver. 
Consequently passes hijacked data to -which is his collegue- second node(receiver proxy) via multiple TCP connections. Then, second 
node demultiplexes incoming data and finally passes towards the original destination.

![] (figs/proxy_topo.bmp)

# usage

As illustrated above, we assume that the experiment needs 4 devices - *commands provided with IP addresses of figure*.
We also assume that proxy devices would have two different interfaces, one for getting incoming data and one for forwarding data - *commands
below provided with ethernet interfaces(eth0, eth1) but those can be anything, such as wlan0 etc*.

### configuration
Connect devices as in topology figure and copy repo to all of them. Consequently,
if you use different ip addresses, change **network.conf** file, which is in root directory of repo. 

#### network.conf's stream
provide file to forward into stream device's stream/ directory, and update file\_name variable.

### brctl usage
Both of the proxy nodes should bridge multiple network interfaces via brctl.

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
Transmitter proxy should define routing rules via iptables.

* `iptables -t mangle -F`
* `iptables -t nat -F`
* `iptables -t nat -I PREROUTING -p tcp -s 192.168.2.11 -j REDIRECT --to-port 5000`
* `iptables -t mangle -I PREROUTING -i br0 -s 192.168.2.11 -p tcp --syn -j NFQUEUE --queue-num=0`

### compile
run **make** in root directory at each devices

## working principles

### tcp session hijacking

netfilter library is used to get hijacked TCP packets from kernel space to user space, which would then be passed
to second proxy node via multiple connections.

### packet reordering 

![] (figs/encaps.bmp)

Second node receives packets from multiple connections. Those may not arrive sequentially due to losses in the medium. 
For this reason, we add our own headers to TCP packets in between pair proxy nodes, which are joint sequence numbers of 
multiple TCP connections.




