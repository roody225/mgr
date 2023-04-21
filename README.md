# Master's thesis at University of Wroclaw

title: Offloading routingu jądra linux na eBPF/XDP z użyciem języka P4 \
author: Andrzej Tkaczyk \
supervisor: Piotr Witkowski

## Running the solution

### Dependencies
+ Internet connection
+ VirtualBox (tested 6.1) [download link](https://www.virtualbox.org/wiki/Download_Old_Builds_6_1)
+ Vagrant (tested 2.2.19) [download link](https://developer.hashicorp.com/vagrant/downloads)

### Running
+ clone the repository
```
git clone git@github.com:roody225/mgr.git
cd mgr
git submodule update --init --recursive
```
+ start the VM (this may take a few minutes)
```
vagrant up
```
+ login into the VM
```
vagrant ssh
```
+ run the agent
```
./agent/build/agent
```
+ start mininet instance
```
sudo ./router/run.py
```
+ insert table entries
```
s1 bash
./nikss-cmd.sh
exit
```
+ send a packet from h1
```
h1 bash
./send-packet.py
exit
```
### Cleaning
+ exit the mininet instance
```
exit
```
+ exit the VM
```
exit
```
+ remove the VM
```
vagrant destroy
```
