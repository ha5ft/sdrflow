![sdrflow logo](images/sdflow.png  "sdrflow")
**The author:** ***Dr.Janos Selmeczi, HA5FT***. You could reach me at <ha5ft.jani@freemail.hu>
***
# Getting familiar with the runtime application

The frameworks runtime application has the following features:
- Runs multiple data flows parallel.
- Runs a dataflow continuosly or runs it for a given number of cycles.
- Has a text based command interface.
- Could get and set those constants, variables in the data flow which has a single value.

## Command syntax

We will desribe the commands by BNF notations. First we describe some important compőonents of the commands.

```
SP ::= " "
letter ::= ("a" | ...... | "z" | "A" |..... | "Z")
digit ::= ("1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" | "0")
signs ::= ("+" | "=" | "?" | "," | "/" | " ")
string ::= """ ((letter | digit | signs) {(letter | digit | signs)}) """
unsigned_integer_number ::= digit {digit}
integer_number ::= (("+" | "-") unsigned_integer) | unsigned_integer_number)
fixed_point_number ::= integer_number "." unsigned_integer_number
floating_point_number ::= (fixed_point_number | (fixed_point_number ("e" | "E") integer_number))
simple_name ::= letter | {letter | digit | "_" | ".")
identifier ::= simple_name | ("_" {"_"} simple_name)
dataflow_instance_name ::= identifier
top_level_composite_name ::= identifier
composite_name ::= identifier
composite_instance_name ::= identifier
constant_name ::= identifier
variable_name ::= identifier
parameter_name ::= constant_name | variable_name
parameter_path_name ::= "/" dataflow_instance_name  { "/" composite_instance_name } "/" parameter_name
```
Now we describe each commands.

```
load_command ::= "load" SP "/" top_level_composite_name "/" dataflow_instance_name SP unsigned_integer_number
run_command ::= "run" SP "/" dataflow_instance_name SP unsigned_integer_number
start_command ::= "start" SP "/" top_level_composite_name "/" dataflow_instance_name
kill_command ::= "kill" SP "/" dataflow_instance_name
unload_command ::= "unload" SP "/" dataflow_instance_name
iget_command ::= "iget"
lget_command ::= "lget"
fget_command ::= "fget"
dget_command ::= "dget"
sget_command ::= "sget"
iset_command ::= "iset" SP parameter_path_name SP integer_number
lset_command ::= "lset" SP parameter_path_name SP integer_number
fset_command ::= "fset" SP parameter_path_name SP floating_point_number
dset_command ::= "dset" SP parameter_path_name SP floating_point_number
sset_command ::= "sset" SP parameter_path_name SP string
exit_command ::= "exit"
```
## Command semantics

### Data flow managements commands

Using these commands you could start and stop dataflows.

- ***load***
Loads a top level composite and all its composite and primitive components by navigating through the components hierarchy of the top level composite. After loading all the components the command creates all component instances. Finally it names the top level composite instace by the ***dataflow_instance_name*** identifier.
- ***run***
Runs ***unsigned_integer_number*** of cycles of the top level data flow instance which has the ***dataflow_instance_name*** name. After the data flow stops the state of the data flow is saved. After the ***run*** command a new ***run*** command or a ***start*** command could be issued and the data flow continuos to run from the saved state.
- ***start***
Starts running the ***dataflow_instance_name*** data flow in its own thread without limitig the cycles to run.
- ***kill***
Stops running the ***dataflow_instance_name*** data flow. When the data flow stops its state is saved. After the ***kill*** command a new ***start*** or ***run*** command could be issued and the data flow continuos to run from the saved state.
- ***unload***
Unloads all composite and primitive components of the ***dataflow_instance_name*** data flow by navigating through the component hierarchy. A component will be unloaded only if it is not used by an other data flow instance. All the resources associated with the data flow instance are freed.
- ***exit***
Stops all data flow instance running. Unloads all the components. Frees all the resources and exit the runtime application.

## Parameter management commands

Using these command you could get or set a constant or a variable in the data flow. The commands work only on those constants and variables which have a single data value only. The commands not work on vectors or on constants and variables which contain multiple vectors.
You could get more information about signal classes (stream, variable, constant) and signal types (int, long, float, double ....) from the [The sdrflow User Guide](documentation/guide_to_the_sdrflow.pdf).

### The set commands

Using these commands you could set the value of a constant or a variable in the data flow.

- ***iset*** : sets the value of the ***int type*** constant or variable to the ***integer_number***
- ***lset*** : sets the value of the ***long type*** constant or variable to the ***integer_number***
- ***fset*** : sets the value of the ***float type*** constant or variable to the ***floating_point_number***
- ***dset*** : sets the value of the ***double type*** constant or variable to the ***floating_point_number***
- ***sset*** : sets the value of the ***char[] type*** constant or variable to the ***string***

### The get commands

- ***iget*** : gets the value of the ***int type*** constant or variable
- ***lget*** : gets the value of the ***long type*** constant or variable
- ***fget*** : gets the value of the ***float type*** constant or variable
- ***dget*** : gets the value of the ***double type*** constant or variable
- ***sget*** : gets the value of the ***char[] type*** constant or variable
