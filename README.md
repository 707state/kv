One simple raft kv. 

Based on Boost.Asio and grpc.

# How to use 

First, clone boost library into deps/ , bootstrap and run ./stage to compile boost libraries.

Then, change the boost lib in cmake to your path or version.

Make sure you have grpc preinstalled and run 
```bash 
./gen_proto.sh
```
in proto/ . 

Finally, you should be able to compile 
```bash 
mkdir build && cd build 
cmake ..
make
```
# License
MIT Licensed
