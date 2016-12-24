#!/bin/bash
MORTON=/home/dmteam/dataSets/silc_raw/data/col.morton 
DATA=/home/dmteam/dataSets/silc_raw/data/col.object.dat.4300 
./knn ./data/col.cnode ./data/col.cedge 10000 $MORTON  ./data/col.object.dat.4300
