#!/bin/bash -xe
find . | egrep '\.(hh|cpp|java)$' | grep -v CMake | grep -v target | xargs wc -l
