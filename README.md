# Master's thesis at University of Wroclaw

title: Offloading of Linux Kernel routing to eBPF/XDP using the P4 language \
author: Andrzej Tkaczyk \
advisor: Piotr Witkowski

## Running the solution

### Dependencies
+ Internet connection
+ VirtualBox (tested 6.1) [download link](https://www.virtualbox.org/wiki/Download_Old_Builds_6_1)
+ Vagrant (tested 2.2.19) [download link](https://developer.hashicorp.com/vagrant/downloads)

### Running
+ update submodules
```
git submodule update --init --recursive
```
+ start the VM (this may take a few minutes)
```
vagrant up
```
+ login into the router VM
```
vagrant ssh router
```
+ insert the p4 pipeline and attach eth devices
```
sudo nikss-ctl pipeline load id 0 p4_router.o
sudo nikss-ctl add-port pipe 0 dev eth1
sudo nikss-ctl add-port pipe 0 dev eth2
```
+ run the agent
```
sudo ./agent/build/agent
```
+ in another terminal login into the host VM
```
vagrant ssh host1
```
+ check the connection
```
ping 192.168.200.12 -c 3
ping6 22::1 -c 3
```
### Cleaning
+ exit a VM
```
exit
```
+ remove the VMs env
```
vagrant destroy
```
