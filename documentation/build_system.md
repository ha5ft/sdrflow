![sdrflow logo](images/sdflow.png  "sdrflow")
**The author:** ***Dr.Janos Selmeczi, HA5FT***. You could reach me at <ha5ft.jani@freemail.hu>
***
# How the build system works

The framework has a build system based on make. Using it you could build the entire framework or build a specific compoment of it. The build system presumes a specific directory structure and directory naming conventions as well as file naming conventions. You must use those conventions if you add components to the framework. If you like to add some new components to the framework all you have to do to create a subdirectory with the components name in the ***primitive*** or ***composite*** subdirectory, place your source files named according to the naming convention. The build system will recognize the new component and will build it.

## Directory structure and naming conventions

***sdrflow***
This is the root directory of the framework. It contains some file containing informations about the framework and all the makefiles. All the other files of the framework are in subdirectories.
- ***actor*** subdirectory
After a build it contains the binary files of the primitives and the composites. This subdirectory is created by the build system.
- ***bin*** subdirectory
It contains the binary executables of the framework. This subdirectory is created by the build sytem.
- ***composite*** subdirectory
It has a subdirectory for each of the composite components. The components subdirectory name is the component name.
  - ***composite_name*** subdirectory
  It contains the source files of the composite ***composite_name***
- ***context*** subdirectory
It contains the interface declaration files of the primitive ans composite components. This subdirectory is created by the build sytem.
- ***build*** subdirectory
It contains a hierarchy of subdirectories. These subdirectories hold the compiling products of the system executables, the system libraries, the composite components and the primitive components. This subdirectory and its subdirectory hierarchy are created by the build system.
- ***default*** subdirectory
It contains the template files used to create new primitive or composite component.
- ***documentation*** subdirectory
It contains the documentation files of the framework. It may have subdirectories.
- ***include*** subdirectory
If you have special library files for your primitive components in the subdirectory ***lib*** then the header files for those libraries may be placed in this subdirectory. You could place herw som common header files for your primitives. Currently it contains the ***primitive_interface.h*** header file.
- ***lib*** subdirectory
You may place special library files for your primitive components in this subdirectory. Currently it is not used.
- ***primitive*** subdirectory
It has a subdirectory for each of the primitive components. The components subdirectory name is the component name.
  - ***primitive_name*** subdirectory
  It contains the source files of the primitive ***primitive_name***
- ***syslib*** subdirectory
It contains the static library files for the framework executables. It is created by the build system.
- ***system*** subdirectory.
In its 4 subdirectories it containes the source files for the framwork executables.
  - ***common*** subdirectory
  It containes the source files for the static libraries of the framework executables. Currently there are 3 such a library.
  - ***sdfasm*** subdirectory
  It contains the source files for the frameworks assembler.
  - ***sdfrun*** subdirectory
  It contains the source files for the frameworks runtime executable.
  - ***sdfsrc*** subdirectory

## Files and file name conventions

### Executables of the framework
- ***sdfasm***
The assembler of the framework. It translate the assembler source code produced by the compiler into binary code.
- ***sdfsrc***
The compiler of the framework. It compiles the composite declaration into assambler source code.
- ***sdfrun***
The runtime executable of the framework.

### Files associated with primitive components
- ***primitive_name.sdf.ctx***
The interface declaration file of the primitive. **Important note:** You must place this file into the ***primitive/primitive_name*** subdirectory. The build system will copy this file into the ***context*** subdirectiry.
- ***primitive_name.h***
Header file for your primitive component. It is in the ***primitive/primitive_name*** subdirectory.
- ***primitive_name.c***
It contains the source code for your primitive components. In the current implementation of the build system you should put all the source code into a single source file. It is in the ***primitive/primitive_name*** subdirectory.
- ***primitive_interface.h***
Header file containing the declaration for the primitives c language interface. It is in the ***include*** subdirectory. You should include this header into your primitives source code.
- ***primitive_name.sdf.so***
The shared library file created from the primitive source code. It is in the ***actor*** subdirectory. The build sytem will place it to there.

### Files associated with composite components
- ***composite_name.sdf.ctx***
The interface declaration file of the composite. This file is generated by the frameworks compiler and place to the ***context*** subdirectory by the build system.
- ***composite_name.sdf.src***
It contains the declaration of the composite components. It is in the ***composite/composite_name*** subdirectory.
- ***composite_name.sdf.asm***
It is the assembler source file of the composite. It is produced by the frameworks compiler and placed into the ***build/composite/composite_name*** subdirectory.
- ***composite_name.sdf.bin***
It is the binary code of the composite. It is produced by the frameworks assembler and placed into the ***actor*** subdirectory by the build system.

## Make targets
There are several build targets in the build system.
- ***all***
This will build the frameworks executables and all the composite and primitive components in the framework.
- ***executables***
This will build the system executables.
- ***composites***
This will build all the composites in the framework.
- ***primitives***
This will build all the primitives in the framework.
- ***sdfsrc***
This will build the frameworks compiler.
- ***sdfasm***
This will build the frameworks assembler.
- ***sdfrun***
This will build the frameworks runtime.
- ***common***
This will build the static libraries for the frameworks executables.
- ***comosite_name***
This will build the ***composite_name*** composite.
- ***primitive-name***
This will build the ***primitive-name*** primitive.
- ***clean***
This will delete all build products.
- ***clean_primitives***
This will delete all build product associated with primitives.
- ***clean_composites***
This will delete all build products associated with composites.

## Using the build system

You should use the build system from the root directory of the framework. You use it in the following ways:
```bash
make make_target
```

## Creating new components
 You could use the build sytem for creating new components. The build system will create the new primitive or composite components from the template files in the default directory. For your own purposes you should modify the created source files.

 ```bash
# Create a new primitive named P1
make -f primitive.create.mk P1
# Create a new composite called C1
make -f composite.create.mk C1
 ```
