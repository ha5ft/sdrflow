![sdrflow logo](images/sdflow.png  "sdrflow")
**The author:** ***Dr.Janos Selmeczi, HA5FT***. You could reach me at <ha5ft.jani@freemail.hu>
***
# Start writing components
There are well defined steps you should take when you create your own primitives and composites. The framework helps you in some of the steps.
## Write your own primitive
You should follow the following steps.
- Create files and directories from templates
You could create the files and directory associated with the new primitive using the build system of the framework. In the ***sdrflow directory***
```bash
> make -f primitive.create.mk YourPrimitive
```
This will create the the directory and all the files you need for your primitive. The following directory and files will be created:
  - ***sdrflow/primitive/YourPrimitive*** directory and
  - ***sdrflow/primitive/YourPrimitive/YourPrimitive.sdf.ctx*** and
  - ***sdrflow/primitive/YourPrimitive/YourPrimitive.c*** and
  - ***sdrflow/primitive/YourPrimitive/YourPrimitive.h*** files.
- Create the interface declaration in ***YourPrimitive.sdf.ctx*** file.
```
primitive	YourPrimitive
	context
; You should put the port declarations here
	end
end
```
You should put the interface declaration statements where the comment line is.
- Fill the primitive's self structure with your definitions.
```c
struct	_YourPrimitive_self
{
	char	*instance_name;
// You should put your definitions here
};
typedef	struct _YourPrimitive_self	YourPrimitive_self_t;
```
You should put those data items into the self structure which are instance level items and which you like to save from one execution to the following one of the primitive's functions.

- Create the interface definition in the context structure in the ***YourPrimitive.h*** file. For each port declaration statements in the interface declaration in the ***YourPrimitive.sdf.ctx*** file you should put a matching type pointer definition into the context structure.
```c
struct	_YourPrimitive_context
{
	YourPrimitive_self_t	*self;
// You should put your definitions here
}__attribute__((packed));
typedef struct _YourPrimitive_context	YourPrimitive_context_t;
```

- Write the 5 functions of your primitive
```c
int	Yourprimitive_load(void *sys_catalog)
{
// You should put your own code here
	return 0;
}
int	Yourprimitive_init(void *ctx)
{
// You should put your own code here
	return 0;
}
int	Yourprimitive_fire(void *context)
{
// You should put your own code here
	return 0;
}
int	Yourprimitive_cleanup(void *context)
{
// You should put your own code here
	return 0;
}
int	Yourprimitive_delete(void *context)
{
// You should put your own code here
	return 0;
}
```
The usage of the five functions
  - ***load() function***
  It is called after the shared library of the primitive's loaded into the runtime application. You shoud do class level initialization and resorce allocations in this function.
  - ***init() function***
  It is called after the primitive instance has been created. You should do instance level initializations and resource allocations in this function.
  - ***fire() function***
  This function should provide the signal processing algorithm of the primitive. It is executed whenever there are enough input data for it's execution.
  - ***cleanup() function***
  It is called before the primitive instance will be deleted. In this function you should free all resources allocated in the ***init() function***.
  - ***delete() function***
  It is called before the shared library of the primitive will be removed from the runtime application. In this function you should free all the resources allocated in the ***load() function***.
- Build your primitive
From the ***sdrflow directory***
```bash
> make YourPrimitive
```

## Write your own composite
You should follow the following steps
- Create files and directories from templates
You could create the files and directory associated with the new composite using the build system of the framework. In the ***sdrflow directory***
```bash
> make -f composite.create.mk YourComposite
```
This will create the the directory and all the files you need for your composite. The following directory and files will be created:
  - ***sdrflow/composite/YourComposite*** directory and
  - ***sdrflow/composite/YourComposite/YourComposite.sdf.src*** file.
- The created ***YourComposite.sdf.src*** file looks like the following
```
; you shold put your use statements here
composite	YourComposite
	context
; you should put your port declaration statements here
	end
	signals
; you should put your signal declaration statements here
	end
	actors
; you should put your actor declaration statements here
	end
	topology
; you should put your topology declaration nstatements here
	end
	schedule
; you should put your schedule hints statements here
	end
end
```
- In the ***YourComposite.sdf.src*** file
  - Lists all primitives and composite to be used in YourComposite composite using ***use statements***.
  - Fill the ***interface declaration*** section with ***port declaration statements***, which declare the ports of your composite.
  - Declare all the signals of the composite in the signal section using ***signal declaration statements***.
  - Declare all component instances in the ***actor section*** using ***actor declaration statements***.
  - Declare the connections of the components in the ***topology section***
  using ***topology declaration statements***.
  - Declare schedule hints in the ***shedule section***.
- Build the composite.
From the ***sdrflow directory***
```bash
> make YourComposite
```

## Get more help
You could get more help for writing components from [The Sdrflow User Guide](documentation/guide_to_the_sdrflow.pdf).
