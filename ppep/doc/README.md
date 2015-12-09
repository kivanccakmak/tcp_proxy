Intro Goes Here.

1. **Problem Statement:** Additional Increase Multiplicative Decrease(AIMD) approach of Congestion Control(CN) algorithms in Transmission Control Protocol(TCP) suffers when instantenous changes exist in transmission medium. Distributed Control Function(DCF) of IEEE 802.11 protocol allows stations to access channel when medium is empty.

when side-band interference exists. 
2. **Solution:** Be aware side-band interferance, manage multiple TCP connections.
3. **How is it so:** Pair Proxy. One of them gets packets from source, splits TCP
connection in between source and receiver. Consequently, checks side-band interference
and opens/closes multiple TCP connections to pass proxied data towards second (companion) device. 
Then, second device demultiplexes data coming from multiple TCP connections and passes towards
original destination.

# Proxy Mechanism 


![Caption Text] (/home/kivi/workspace/tcp_proxy/relay/figs/proxy_picture.bmp)

iptables and netfilter libraries used.
netfilter provides getting packets from
kernel space to user space, then we pass
this packets to another receiver with multiple
TCP connections.

# Packet Reordering

![Caption Text] (/home/kivi/workspace/tcp_proxy/relay/figs/transmitter_proxy.bmp)


![Caption Text] (/home/kivi/workspace/tcp_proxy/relay/figs/receiver_proxy.bmp)

Packets multiplexed with multiple TCP connections.
Receiver might not get them sequentially due to 
characteristic of Wireless Channel. We add  
our own headers to TCP packets, which are common
sequence numbers of multiple TCP connections.

## Encapsulation and Decapsulation

We encapsulate TCP packet with another sequence number.
Receiver side of proxy decapsulates it, then reorders.

### Reordering Logic

Each receiver has temporariy buffer, and queue has sequential
buffer. If one of the receiver gets sequentially expected 
packet, he adds it into queue and wakes queue up. Then, 
queue passes data to end-destination.

# Multiple TCP Connection Usage

Add sniffer device into setup. Get interference statistics
by sniffing via tcpdump and get instantenous statistics
from it. Then, decide to add or remove TCP connections
from this statistics.

## Used Algorithm

Whenever interferance increases, start to close TCP connections.
Whenever interferance decreases, initiate TCP connections.

