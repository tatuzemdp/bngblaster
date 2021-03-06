# Introduction

Building a BNG from scratch requires a lot of testing but commercial BNG test software 
is often very expensive, resource intensive and provide a lot of administrative overhead 
to maintain such. 

Therefore we decided to build our own BNG test solution which is completely build from scratch 
targeted for max scaling with small resource footprint, simple to use and easy to integrate in 
our test automation infrastructure. 

The BNG Blaster is able to simulate more than hundred thousand PPPoE subscribers including IPTV, 
traffic verification and convergence testing capabilities.

* *High Scaling:* > 100K sessions
* *Low CPU and Memory Footprint:* < 100MB memory for 16K sessions
* *Portable:* runs on every modern linux, virtual machines and containers
* *User Space:* all protocols implemented in user-space from scratch and optimized for performance
* *IPTV:* IGMP version 1, 2 and 3 with automated channel zapping test
* *Automation:* the BNG Blaster Controller provides an automation friendly REST API and robot keywords

```
$ bngblaster --help

      ____   __   ____         _        __                                  ,/
     / __ \ / /_ / __ ) _____ (_)_____ / /__                              ,'/
    / /_/ // __// __  |/ ___// // ___// //_/                            ,' /
   / _, _// /_ / /_/ // /   / // /__ / ,<                             ,'  /_____,    
  /_/ |_| \__//_____//_/   /_/ \___//_/|_|                          .'____    ,'   
      ____   _   _  ______   ____   _               _                    /  ,'
     / __ ) / | / // ____/  / __ ) / /____ _ _____ / /_ ___   ____      / ,'
    / __  |/  |/ // / __   / __  |/ // __ `// ___// __// _ \ / ___/    /,'
   / /_/ // /|  // /_/ /  / /_/ // // /_/ /(__  )/ /_ /  __// /       / 
  /_____//_/ |_/ \____/  /_____//_/ \__,_//____/ \__/ \___//_/

Usage: bngblaster [OPTIONS]

  -h --help
  -C --config <args>
  -l --logging error|igmp|io|pppoe|normal|pcap|timer|timer-detail|ip
  -L --log-file <args>
  -u --username <args>
  -p --password <args>
  -P --pcap-capture <args>
  -J --json-report <args>
  -c --pppoe-session-count <args>
  -g --mc-group <args>
  -s --mc-source <args>
  -r --mc-group-count <args>
  -z --mc-zapping-interval <args>
  -S --control socket (UDS) <args>
  -I --interactive (ncurses)
```

The BNG Blaster includes an optional interactive mode (`-I`) with realtime stats and 
log viewer as shown below.

![BNG Blaster Interactive](images/bbl_interactive.png)

## Theory Of Operation

The BNG Blaster has been completely built from scratch, including user-space implementations of the entire protocol 
stack you need for interfacing with a BNG. It’s core is based on a very simple event loop which serves timers and signals. 
The timers have been built using a constant time (O(1)) library which we built purposely to start, restart and delete the 
protocol session FSM timers quickly and at scale.

BNG Blaster expects a Linux kernel interface which is up, but not configured with any IP addresses or VLAN as it expects to 
receive and transmit raw ethernet packets.

BNG Blaster does I/O using high-speed polling timers using the so-called PACKET_RX_RING/PACKET_TX_RING abstraction where a 
userspace program gets a fast-lane into reading and writing to kernel interfaces using a shared ring buffer. The shared ring 
buffer is a memory mapped "window" that is shared between kernel and user-space. This low overhead abstraction allows to 
transmit and receive traffic without doing expensive system calls. 

![BNG Blaster Architecture](images/bbl_arch.png)

Sending and transmitting traffic is as easy as just by copying a packet into a buffer and setting a flag. This is super 
efficient and hence we have measured the I/O performance of roughly 1M pps per single CPU thread, which is more than enough for 
our purposes here.

BNG Blasters primary design goal is to simulate thousands of subscriber CPE's with a small hardware resource footprint. Simple 
to use and easy to integrate in our robot test automation infrastructure. This allows to simulate more than hundred thousand 
PPPoE subscribers including IPTV, traffic verification and convergence testing from a single medium scale virtual machine or to 
run the blaster directly from a laptop. 

The BNG Blaster provides two types of interfaces. The first interface is called the access interface which emulates the PPPoE 
sessions. The second interface-type is called network interface. This is used for emulating the core-facing side of the 
internet.

![BNG Blaster Interfaces](images/bbl_interfaces.png)

This allows to verify IP reachability by sending bidirectional traffic between all PPPoE sessions on access-interface and the 
network interface. The network interface is also used to inject downstream multicast test traffic for IPTV tests. 
