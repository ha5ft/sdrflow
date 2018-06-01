![sdrflow logo](documentation/images/sdflow.png  "logo")
**The author:** ***Dr.Janos Selmeczi, HA5FT***. You could reach me at <ha5ft.jani@freemail.hu>
***
## Important documents you should read
- [Getting started with sdrflow](documentation/get_started.md)
- [Try the included Demo data flow](documentation/try_demo_app.md)
- [Try the included M2 data flow](documentation/try_m2_app.md)
- [How the build system works](documentation/build_system.md)
- [Get familiar with the runtime application](documentation/runtime_application.md)
- [The Sdrflow User Guide](documentation/guide_to_the_sdrflow.pdf)
- [Start writing components](documentation/write_components.md)

## What is sdrflow?
Sdrflow is a development framework, which could be used to develop signal processing applications for SDR and other purposes. It has the following features:

- Component based, uses
  - ***primitive*** components written in ***C languge***
  - ***composite*** components constructed from primitives and other components and written in the ***sdrflow language***
- An application's component sytem is hierarchical. The top level composite is the application itself.
- Based on the synchronous data flow paradigm
- Main parts of the framework
  - ***sdrflow language*** for constructing composite components
  - ***compiler and assembler*** for compiling composites into binary code
  - ***runtime*** application for running your application
- The runtime application is lightweight. It could be made run on small embedded processors like ARM Cortex M4.

It is Open Source software. Licensed under GNU GPL 3 or any later version.
The current version runs only on ***64 bit Linux sytems***
## Incuded applications
The framework comes with two data flow applications:
- The ***Demo*** dataflow implements a ***CW transmitter*** connected to a ***CW receiver*** through a ***Channel*** wich add interfering signals.
- The ***M2*** data flow tests the scheduling capabilities of the framework. Its primitive components only put out a text message that they were invoked.

You could test this two data flow out of the box.
## The steps you should take
First of all you should download and compile the framework. You should follow the instructions in the [Getting started with sdrflow](documentation/get_started.md) document.
Next you could try the included Demo data flow following the instructions in the [Try the included Demo data flow](documentation/try_demo_app.md) document and you could try the included M2 dataflow following the instructions in the [Try the included M2 data flow](documentation/try_m2_app.md) document.
If you have found the system interesting for you, you should become more familiar with the system. For a brief introduction you should read the following documents:
- [How the build system works](documentation/build_system.md)
- [Get familiar with the runtime application](documentation/runtime_application.md)

For a more detailed explanation you should read the [The Sdrflow User Guide](documentation/guide_to_the_sdrflow.pdf) document.
Finally you could start to roll out your own data flows following the instructions in the [Start writing components](documentation/write_components.md) document.
If you have any questions, you could contact me at <ha5ft.jani@freemail.hu>.
