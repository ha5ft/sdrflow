![sdrflow logo](images/sdflow.png  "sdrflow")
**The author:** ***Dr.Janos Selmeczi, HA5FT***. You could reach me at <ha5ft.jani@freemail.hu>
***
## Getting started with sdrflow
### Platform requirements
You should have an ***64 bit Linux system*** to try the sdrflow framework. I used the ***ubuntu 16.04.4*** system for the development and the framework has been tested on ***ubuntu 18.04*** too.  A friend of mine has compiled the framework on 64 bit Linux on ARM processor.
### Install prerequisites
You should have some packeges installed on your system. On ubuntu or debian systems you could installed them using the folowing command:

```bash
> sudo  apt-get install git make gcc libpulse-dev libfftw3-dev
```

### Get the framework source

You could download the softwate using git.

```bash
# create a working directory
> mkdir sdrwork
# go to this directory
> cd sdrwork
# clone the sdrflow repository from github
> git clone https://github.com/ha5ft/sdrflow
```
you will get the framework in the ***sdrflow*** directory in your working directory.

### Compile the framework

Before you could start experimenting with the framewor you should compile it. The framework comes with a ***build system*** based on ***make***. The build system will build the framework's executabels and all the primitives and composites which come with the framework. To build the framework you should do the following:

```bash
# go to the sdrflow directory
> cd sdrflow
# build the framework
> make all
```
After that you could check the sdrflow directory. Several new directory have been created.
- In the ***sdrflow/bin*** directory you will find the executables of the framework.
- In the ***sdrfow/actor*** directory you will finde the binary codes for primitives and composites.
- In the ***sdrflow/context*** directory you will finde the interface declaration files of the primitives and composites.

Now you are ready to thest the built in data flows and start to roll out your own ones.
