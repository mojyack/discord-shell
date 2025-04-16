#!/bin/bash

set -e

cd /tmp
git clone --depth=1 -b v10.1.2 https://github.com/brainboxdotcc/DPP.git
cd DPP
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=release -DCMAKE_INSTALL_LIBDIR=lib -DBUILD_VOICE_SUPPORT=OFF -DDPP_BUILD_TEST=OFF -DDPP_NO_VCPKG=OFF -DDPP_NO_CONAN=OFF -DCMAKE_INSTALL_PREFIX=/tmp/rootfs
make -j$(nprocs)
make install
