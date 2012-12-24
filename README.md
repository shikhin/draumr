#Draumr

[Draumr](/Shikhin/Draumr/), a 21st century multiprocessing operating system, has been designed specially for SMP and NUMA. Conceived by Shikhin Sethi, it is currently a work under progress and *unstable*. 

Draumr thrives on the idea that the user of any OS should have ultimate control over its functioning, and yet have a user-friendly OS. 

#Build

##Requirements

For building Draumr on your local system, the following pre-requisites are needed:

* SCons build system.
* GCC or compatible compiler suite.
* NASM or compatible assembler.

##Procedure

To build Draumr, the following simple command is needed:

    scons build=release target=iso;

For developmental purposes, the ````build```` attribute can be set to ````debug```` which disables optimization. The ````target```` attribute can be set to ````pxe````, ````floppy```` or ````all```` to build the respective images/targets.

#Help

To help this project, you can fork it and use pull requests to incorporate changes in the code. Bugs can be filed via GitHub's issue-tracking system. 

The author can be contacted via his e-mail listed at the GitHub profile.