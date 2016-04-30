Usage of the keccak implementation:
1.) change to directory
2.) mkdir bin
3.) cmake ..
4.) make

Different executables are created:
keccak_find_cubes creates cubes for 4 rounds keccak.
USAGE: ./keccak_find_cubes > Output.txt

recoverKey is responsible to recover the key of 4 rounds of keccak.
USAGE: ./recoverKey <path to cube file>
NOTE: as an alternative one can use the provided files cubes.txt or cubes1.txt

The 5 round version of the cube attack is also implemented. The executables with the extension "_extended"
can be used in the same way as the previous descriped ones.
NOTE: as an alternative one can use the provided files cubes5.txt or cubes51.txt

NOTE: Since the cubes are only checked with a certain probability with respect to linearity, it may
happen that few cubes cannot be used for key recovery.