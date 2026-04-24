# Using the unpacker
The unpacker is setup to read in a TCL input file. It has a few main components
- The stagearea path where raw events are stored
- The run numbers you want to analyze
- The directory where you want unpacked files stored
- The loading in of classes related to the modules used when taking data

This unpacker supports the V2740 in a standalone fashion - that means simple event building with no other modules mixed in. 

It also supports VMUSB readout and is nicely generalized, but I won't go into depth about this functionality here.

## configurations/v2740.tcl
- **unpacker create {name}** : create an instance of the unpacker named "{name}"
  
- **{name} stack module v2740** : register the V2740 data recorder to the unpacker
    - *-recordsamples* needs to be equal to the number of samples you set in Readout/configuration.tcl (recordsamples in v27xxpha config)
    - *-nsPerSample* needs to be set to the time bin resolution set in Readout/configuration.tcl (waveresolutions in v27xxpha config)
      
- **{name} tree create {TreeName}** create an output tree with {TreeName}
    - This creates a file named "run-{runNumber}-{TreeName}.root. 
    - *-title* : Title of the TTree
    - *-description* : Description of the TTree. Can accept some sacalers, but this is a work in progress. For now I would just keep it as the run number (run=%RUN%)
    - *-outdir* : Very important - the path where the TFile will be written to.

- **{TreeName} wavebranch v2740** : structure to store waveforms and waveform data
    - Stores the waveforms, channel ID, number of hits the channel received, timestamp, and energy of the waveform. 
