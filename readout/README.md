# Configuring the V2740 module, readout binary, and ReadoutShell

### Notes on configuration/setting up the V2740 readout binary
- This directory contains two example configuration files for the V2740. 
- The binary which unpacks the V2740 data into the FRIBDAQ data stream is defined by the **Skeleton** class. This class looks for a configuration file named **configuration.tcl** in the Readout directory. The config file must be named **configuration.tcl**
- Class **Skeleton** is also linked to the file **ReadoutCallouts.tcl**. Upon inspection, you'll see that **ReadoutCallouts.tcl** controls the the event building ring sources - this is important.

### configuration.tcl
- Refer to the [FRIBDAQ documentation on how to setupthis digizer](https://docs.frib.msu.edu/daq/newsite/nscldaq-12.0/c15708.html)
- 72.2.2 defines all the parameters you can tune on the digitizer.
- For saving waveforms, the following variables are will you will want to start with: 
  - inputpolarities
  - triggerthresholds
  - recordsamples
  - waveresolutions (minimum = 8ns time bins)
  - pretriggersamples

### ReadoutCallouts.tcl
Let's break down the main things you'll have to change in this file, focusing on the code snippet below.
'''
proc OnStart {} {
	EVBC::initialize -restart true -glombuild true -glomdt {GlomWindow} -destring {EventRing}
}

EVBC::registerRingSource tcp://localhost/{RawRing} "" {sourceID} {ModuleName} 1 1 1 0
'''
#### Variables:
- RawRing : a ring buffer of sufficient memory to hold the raw data, streaming directly from the V2740
- EventRing : a ring buffer of sufficient memory, usually larger than RawRing, that holds FRIBDAQ defiend PHYSICS_EVENTS matched by timestamp.
  - EventRing contains the data that is written to disk. Both the unpacker and the waveform-viewer are expecting to see this kind of data (not the RawRing!)
- GlomWindow : A window (in the clock ticks of the V2740) to look for correlated events from RawRing to push into EventRing.
  - The V2740 has a 125MSPS ADC clock, meaning each GlomWindow unit represents 8 ns.
  - As an example:
      - If GlomWindow = 1, and two channels fire with a time difference of 5 ns, then the traces of those channels will be sent to EventRing.
      - If GlomWindow = 1, and two channesl fire with a tiem difference of 10 ns, **neither** of the traces will be sent to EventRing.
- SourceID : ID of the source. Refer to the FRIBDAQ documentation for more information.
- ModuleName : For a standalone V2740 run this is cosmetic. The name appears in the event builder window after a run has begun.

### Skeleton.cpp

### Steps to setup the ReadoutShell
This assumes you have properly installed the [CAEN V2740 USB protocols](https://www.caen.it/download/?filter=V2740) (requires sudo permissions to run)

1. C **config/configuration.tcl** one directory up into the **Readout** directory.
2. Refer to the FRIBDAQ documentation, make whatever changes you need to to configure
3. Power on the digitizer and plug it into your DAQ machine.
4. Obtain the PID of the digitizer. This can be done by rerunning the regPID.sh script provided from the CAEN USB driver.
5. Open Skeleton.cpp. In Skeleton::SetupReadout, change the PID to match the PID of the digitizer. Change the module name to match whatever module name you chose when writing configuration.tcl
   '''
   pSegment->addModule(
    "adc1",                   // Name of module in configuration.tcl
    "53405",                   // PID for USB connection, IP If ethernet.
    1,                        // System unique source id.
    true                      // Indicates it's USB not Ethernet defaults to false.
  );
   '''
6. 
