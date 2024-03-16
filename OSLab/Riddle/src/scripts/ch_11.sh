#!/bin/bash

touch secret_number;
ln secret_number link; 
cat link | awk '{print $12}';
