# Virtual Memory Simulator
- The purpose of this project is to develop in C a virtual memory simulator. The simulator have data structures and a mechanism of
Memory mapping (logical -> physical) required to perform a pagination, and  two replacement algorithms for Pages: the Not Recently Used (NRU) and the Least- Recently used (LRU).  
- The file **_relatorio.txt_** explains how it works and the returned results in portuguese.
# Development enviroment
- Operational System: Fedora 23  
- Language: C  
- Compiler: gcc  

# How to compile and execute
1. Compile
```
$ gcc -o simulador simulador.c
```
2. Execute
```
$ ./simulador <algorithm-nru-or-lru> <fileName.log> <pageSize> <memorySize>
```
  - Debug mode
```
$ ./simulador <algorithm-nru-or-lru> <fileName.log> <pageSize> <memorySize> -d
```
