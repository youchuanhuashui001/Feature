- PA

1. ubuntu 16.04 install llvm-12

   ```shell
   printf "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-12 main" |sudo tee /etc/apt/sources.list.d/llvm-toolchain-xenial-12.list
   wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key |sudo apt-key add -
   sudo apt update
   sudo apt install llvm-12
   ```

   

