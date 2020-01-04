# dispatch

## Pre-req

### OSX
```
brew install boost
brew install zeromq
brew install libsodium
brew install capnp pycapnp
sudo pip install tornado
sudo pip install netifaces
git submodule init
git submodule update
```

### Linux
```
sudo apt-get install libtool pkg-config build-essential autoconf automake uuid-dev wget python-dev
sudo apt-get install libboost-all-dev
cd ~ && git clone git://github.com/jedisct1/libsodium.git && cd libsodium
./autogen.sh && ./configure
make -j2 && sudo make install
cd .. && wget http://download.zeromq.org/zeromq-4.1.4.tar.gz
tar -zxf zeromq-4.1.4.tar.gz && cd zeromq-4.1.4
./configure
make -j2 && sudo make install
sudo pip install pyzmq
sudo pip install tornado
sudo pip install netifaces
```

### Path
In .bash_profile or .bashrc, set:
```
export A17_ROOT=/Users/XXXXXX/a17
export PYTHONPATH=$A17_ROOT/capnp_msgs/build/py:$A17_ROOT/dispatch/build/py:$PYTHONPATH
export LD_LIBRARY_PATH=$A17_ROOT/capnp_msgs/build/lib:$A17_ROOT/dispatch/build/lib:$LD_LIBRARY_PATH
```
where A17_ROOT is the path to the directory that contains the folders capnp_msgs, a17core and dispatch and XXXXXX is the user for the system

## Clone
use ```--recursive``` to grab submodules when cloning

## Build
```
mkdir build && cd build
cmake ..
make install
```
