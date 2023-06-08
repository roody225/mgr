#!/bin/bash

sudo nikss-ctl pipeline load id 0 p4_router.o
sudo nikss-ctl add-port pipe 0 dev eth1
sudo nikss-ctl add-port pipe 0 dev eth2