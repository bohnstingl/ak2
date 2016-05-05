1.) compile the file using: g++ -std=c++11 des.cpp -lpthread -o des
2.) run des

The output contains the bias for 3, 5 and 7 rounds of DES with respect to the linear cryptanalysis
task. 
The key recovery is basically implemented in, but we could not manage to perform the attack
explicitly. This was because for some reason the assumption about the influencing bits of the 
8th round key (bits 42 - 47) didn't hold in our case.
