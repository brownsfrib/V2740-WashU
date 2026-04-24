# V2740 FRIBDAQ package

The following contains instructions on setting up and taking data with a [CAEN V2740 digitizer](https://www.caen.it/products/v2740/)

## Requirements
1. Your DAQ machine must have a version of the [FRIBDAQ](https://github.com/FRIBDAQ) greater than 12.1-018
2. All programs are contained in an [FRIBDAQ container](https://hub.docker.com/u/fribdaq) greater than bullseye (bullseye or bookworm should work).

## Layout of repository
### Shell
- File **shellrc** is the only critical file you'll have to update.
- In **shellrc**, update the FRIBDAQ version (currently set to 12.1-018) to whatever DAQ version you are using
- Depending on which FRIBDAQ ISO image you're using (bookworm or bullseye) you will also have to update the path to the ROOT setup script (currently set using bullseye to ROOT version 6.26.04)
- Update shell_aliases and shell_functions at your leisure.

### Readout
- Storage of the **ReadoutShell** GUI script.
- Storage of the custom **Readout** binary used for unpacking data from the V2740. 
- Storage of the **config/configuration.tcl** script used to set FPGA parameters on the V2740.
- More details inside. I highly suggest [this documentation](https://docs.frib.msu.edu/daq/newsite/nscldaq-12.0/c15708.html), specifically section 72.2 on "Getting Data".
  
### Unpacker
- Source code/binary for unpacking raw binary (evt) files from the V2740
- There is a dedicated README in this directory with instructions on how to setup a configuration file/use the unpacker. More details inside.
- Fun fact, this is also a generalized VMUSB unpacker! Kinda cool!

### Waveform-viewer
- Binary for viewing digitized waveforms online. 
- Binary stored as **waveform-viewer/bin/viewWaveform**. The master parent directory has a symlink to this binary called *onlineTrace*.
- If you run this executable, I suggest you do **not** run it on the same DAQ machine, lest you backlog the event building ring buffer. If you do run it on the same DAQ machine that is running the ReadoutShell, do not run it for very long (unless you have allocated a huge ring buffer!)
- Example usage:
    - Given an event building ringbuffer named **evbout** running over a tcp connection, type:
    - ./onlineTrace tcp://localhost/evbout
- Currently setup to view channels 0-16. More details inside on updating the source code - it is a very light executable. 
