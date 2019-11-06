# Readers-Writers-Problem-Kernel-Module

The kernel module rw_rcu creates two readers and two writer threads.
The readers reads the entries in a circular doubly linked list and
the writers modify the list by adding new entries to the list.
RCU is used for synchronization between readers and writers.

## Instructions to run the code:

1. `git clone https://github.com/Madhuparna04/Readers-Writers-Problem-Kernel-Module.git`

2. `make`

3. `sudo insmod rw_rcu.ko`

4. `dmesg`

5. `sudo rmmod rw_rcu.ko`
