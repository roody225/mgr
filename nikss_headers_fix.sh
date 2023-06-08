#!/bin/bash

for i in include/*
do
  sed -i -e 's/#include\s<nikss.h>/#include <nikss\/nikss.h>/' $i
done
