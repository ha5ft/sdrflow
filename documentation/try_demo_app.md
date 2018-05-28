![sdrflow logo](images/sdflow.png  "sdrflow")
**The author:** ***Dr.Janos Selmeczi, HA5FT***. You could reach me at <ha5ft.jani@freemail.hu>
***
## Trying the Demo dataflow
This is a signal processing dataflow. It implements:
- a CW transmitter which continuosly transmits a given text,
- a CW receiver, which filters a part of the spectrum, demodulate the CW signal in it and puts the demodulated signal to the sound card,
- a Channel which connect the transmitter and the reaceiver and add an interfering CW signal.

The data flow uses the (-24000Hz,+24000Hz) frequency range and uses 48000 sample/sec sampling rate.
A more detailed description with block diagrams you could find in my [Fredrichshafen SDR Academy 2018 presentation](Friedrichshafen2018/Building_sdr_application_with_sdrflow.pdf).

## Prerequisites
Your computers soung system should work.
You should have spekers or headphone connected to your computer.
The proper working of the dataflow could be checked by hearing its audio output.
## Your first experiment with Demo
The Demo data flow has a command line interface through the sdrflow framework runrime application. So you shoud run it from a ***command terminal in the sdfrflow directory***.
```bash
# start the sdrflow runtime_application
> ./bin/sdfrun
sdrflow runtime version 0.1
sdrflow>
# now you could load and start the Demo data flow
# The first name in the path used in the load command is the name of the top composite
# The second name in the path will be the name of the data flow instance
sdrflow>load /Demo/demo
OK
sdrflow>start /demo
OK
sdrflow>
# Now you should here  the "cq de ha5ft " CW transmission.
# You could change the receivin frequency.
sdrflow>fset /demo/rxfreq 8800.0
OK
sdrflow>
# Now you should hear an other CW transmission "cq de ha5khe "
# This is the interfering signal.
# Now you should change the receiving frequency
sdrflow>fset /demo/rxfreq 8400.0
OK
sdrflow>
# Due to the narrow receiving bandwith now you hear nothing
# Now its time to increase the receiving bandwidth
sdrflow>fset /demo/rxbw 1000.0
OK
sdrflow>
# Now you could hear both of the CW transmission
# You could chane one of the transmitted text
sdrflow>sset /demo/txtext "ha5ft de ha7ilm pse k "
OK
sdrflow>
# Now you could hear the change CW transmision.
# To finish the test you stop the dataflow and exit the systems
sdrflow>kill /demo
OK
sdrflow>unload /demo
OK
sdrflow>exit
>
```
## Changable parameters and it's defaul values
The Demo data flow has several changable parameters. The parameter's name include the instance name of the data flow which has benn given by you in the ***load*** command. In the following description I suppose that the instance name is ***demo***.

Parameter's path name | Default value | Value type | Meaning
--- | --- | --- | ---
/demo/txfreq | 8000.0 | fixed point | transmission frequency
/demo/txgain | 1.0 | fixed point | transmission envelop
/demo/txtext | "cq de ha5ft " | string | transmitted text
/demo/ifreq | 8800.0 | fixed point | interference frequency
/demo/igain | 1.0 | fixed point | interference envelop
/demo/itext | "cq de ha5khe " | string | interference text
/demo/rxfreq | 8000.0 | fixed point | receiving frequency
/demo/beatfreq | 800.0 | fixed point | beat frequency
/demo/rxbw | 200.0 | fixed point | receiving filter bandwidth
/demo/rx/gain | 0.5 | fixed point | receiver baseband mixer gain
/demo/rx/beatgain | 0.5 | fixed point | receiver beat mixer gain

You could ***set*** and ***get*** the ***fixed point*** parameters by the ***fset*** and ***fget*** commands. The ***string*** parameters could be ***set*** and ***get*** with the ***sset*** and ***sget*** commands. You could read more about the commands of the runtime application in the [Get familiar with the runtime application](documentation/runtime_application.md) document.
