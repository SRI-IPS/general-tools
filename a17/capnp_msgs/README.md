# capnp_msgs

## Pre-req

### OSX

```sh
brew install capnp
sudo pip install pycapnp
```

### Linux

```sh
sudo apt-get install python-dev
curl -O https://capnproto.org/capnproto-c++-0.5.3.tar.gz
tar zxf capnproto-c++-0.5.3.tar.gz
cd capnproto-c++-0.5.3
./configure
make check
sudo make install
sudo pip install pycapnp
```

## Build

```sh
mkdir build
cd build
cmake ..
make install
```
