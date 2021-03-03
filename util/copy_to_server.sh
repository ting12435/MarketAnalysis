#!/bin/bash


rsync -rav -e ssh --exclude='data/*' --exclude='run/bsdr_analysis/analysis/*' --exclude='run/bsdr_analysis/output/*' ../../MarketAnalysis/ tim@192.168.172.236:/home/tim/MarketAnalysis/