## Steps to Compile and Deploy

For newer Vertica versions compilation is straightforward.
Older Vertica versions (e.g. v9.2) were compiled using an old g++ version (e.g. v4.8).
It can be installed using this commands (the example is for Ubuntu 22):

```
mkdir install_g++-4.8
cd install_g++-4.8/
wget http://mirrors.kernel.org/ubuntu/pool/universe/g/gcc-4.8/g++-4.8_4.8.5-4ubuntu8_amd64.deb
wget http://mirrors.kernel.org/ubuntu/pool/universe/g/gcc-4.8/libstdc++-4.8-dev_4.8.5-4ubuntu8_amd64.deb
wget http://mirrors.kernel.org/ubuntu/pool/universe/g/gcc-4.8/gcc-4.8-base_4.8.5-4ubuntu8_amd64.deb
wget http://mirrors.kernel.org/ubuntu/pool/universe/g/gcc-4.8/gcc-4.8_4.8.5-4ubuntu8_amd64.deb
wget http://mirrors.kernel.org/ubuntu/pool/universe/g/gcc-4.8/libgcc-4.8-dev_4.8.5-4ubuntu8_amd64.deb
wget http://mirrors.kernel.org/ubuntu/pool/universe/g/gcc-4.8/cpp-4.8_4.8.5-4ubuntu8_amd64.deb
wget http://mirrors.kernel.org/ubuntu/pool/universe/g/gcc-4.8/libasan0_4.8.5-4ubuntu8_amd64.deb
sudo apt update
sudo apt install ./gcc-4.8_4.8.5-4ubuntu8_amd64.deb ./gcc-4.8-base_4.8.5-4ubuntu8_amd64.deb ./libstdc++-4.8-dev_4.8.5-4ubuntu8_amd64.deb ./cpp-4.8_4.8.5-4ubuntu8_amd64.deb ./libgcc-4.8-dev_4.8.5-4ubuntu8_amd64.deb ./libasan0_4.8.5-4ubuntu8_amd64.deb ./g++-4.8_4.8.5-4ubuntu8_amd64.deb
```

Anyway, to compile the UDx run something like that:

```shell
cd vertica-siphash-udx
g++-4.8 -I/opt/vertica/sdk/include -I/path/to/project/vertica-siphash-udx -Wall -shared -Wno-unused-value -fPIC -o SipHash.so SipHashUDx.cpp siphash.c /opt/vertica/sdk/include/Vertica.cpp
```

Then use the following SQL command to register the library in Vertica and test the function:

```sql
CREATE OR REPLACE LIBRARY SipHashLibrary AS '/home/deploy/vertica-siphash-udx/SipHash.so';
CREATE OR REPLACE FUNCTION SIPHASH AS LANGUAGE 'C++' NAME 'SipHashFunctionFactory' LIBRARY SipHashLibrary;
SELECT SIPHASH('test input') AS hash_value;
```

It is enough to do this on one of the cluster nodes, but need to be done at each cluster.
