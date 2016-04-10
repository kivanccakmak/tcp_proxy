1. **Problem Statement:** Additional Increase Multiplicative Decrease(AIMD) approach of Congestion Control(CN) algorithms in 
Transmission Control Protocol(TCP) suffers when multiple packet losses exists in the medium. 

2. **Solution:** Proxy real connection and manage multiple TCP connections in between two nodes of backbone.
3. **How is it so:** Pair proxy. One of them(transmitter proxy) splits TCP connection in between the original source and receiver. 
Consequently passes hijacked data to -which is his collegue- second node(receiver proxy) via multiple TCP connections. Then, second 
node demultiplexes incoming data and finally passes towards the original destination.

# Proxy Mechanism 

![] (/home/kivanc/workspace/tcp_proxy/figs/topo.bmp)

brctl is used to bridge multiple network interfaces of proxy nodes. iptables is used to define routing rules.
netfilter library is used to get hijacked TCP packets from kernel space to user space, which would then be passed
to second proxy node via multiple connections.

# Packet Reordering

![] (/home/kivanc/workspace/tcp_proxy/figs/encaps.bmp)

Second node receives packets from multiple connections. Those may not arrive sequentially due to losses in the medium. 
For this reason, we add our own headers to TCP packets in between pair proxy nodes, which are joint sequence numbers of 
multiple TCP connections.
