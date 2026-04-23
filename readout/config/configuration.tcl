# 3/4/26
# ============================================================
# VX27xx PHA setup for recording real detector signals
# (external analog input, self-triggered waveform capture)
# ============================================================

# Create the digitizer object.  Name must be unique.
v27xxpha create adc1

# ------------------------------------------------------------
# Acquisition start
# ------------------------------------------------------------
# use software start after the module is armed
# for the experiment, we'll want to take the FROG OR and pass it 
# into the front of the module (change SWcmd to SINlevel or SINedge)
v27xxpha config adc1 startsource [list SWcmd]

# ------------------------------------------------------------
# Trigger configuration
# ------------------------------------------------------------
# use each channel's own self-trigger to generate both:
# 	- waveform acquisition trigger
# 	- event trigger
# also consider using Ch64Trigger? 
#	- records data from all channels on the OR of all PMTs
v27xxpha config adc1 \
    wavetriggersrc  [lrepeat 64 [list ChSelfTrigger]] \
    eventtriggersrc [lrepeat 64 [list ChSelfTrigger]] \
    savetraces Always

# ------------------------------------------------------------
# Waveform / probe configuration
# ------------------------------------------------------------
v27xxpha config adc1 wavesource [lrepeat 64 ADC_DATA]

# Record 125 samples/channel at 8 ns/sample (1 us)
# Adjust recordsamples / waveresolutions to fit your pulse width
v27xxpha config adc1 \
    recordsamples     [lrepeat 64 64] \
    waveresolutions   [lrepeat 64 Res8] \
    pretriggersamples [lrepeat 64 40] \
    analogprobe1      [lrepeat 64 ADCInput] 

#TimeFilter
#EnergyFilter
#EnergyFilterBaseline
#EnergyFilterMinusBaseline
v27xxpha config adc1 \
    analogprobe2      [lrepeat 64 TimeFilter]


v27xxpha config adc1 readanalogprobes  [list on off]
v27xxpha config adc1 readdigitalprobes [list off off off off]

# ------------------------------------------------------------
# Input conditioning / channel enable
# ------------------------------------------------------------
# enable all channels and use the real input baseline
# dcoffsets are percentages of full scale
# dcoffsets between 0 to 100 - we'll have to change these
# 	- tune dcoffsets to set waveform inside ADC range
# 	- tune triggerthresholds for noise rejection
v27xxpha config adc1 \
    channelenables [lrepeat 64 true] \
    dcoffsets      [lrepeat 64 15.0]

# ------------------------------------------------------------
# Self-trigger threshold
# ------------------------------------------------------------
# this is the key setting for real signal acquisition
# we'll have to change the baseline for EACH PMT
# to low and we trigger on noise, too high and we don't
# capture the entire waveform
# units are in a 13 bit range (0 to 8191) (0-2V input) (0.25 mV increments)

# Testing different thresholds for PMTs (60Co source, middle of bar)
# PMT1 : works well around 195mV (800) (-1700V, Oretec)
# PMT2 : works well around 195mV (800) (-1600, Canberra)
v27xxpha config adc1 triggerthresholds [lrepeat 64 250]

# set polarity to match your detector
# 	- pulses coming out of the anode should be positive?
# 3/6/25 - Alternating anodes/dynodes - even channels anodes, odd dynodes
# 3/19/25 - Only anodes
v27xxpha config adc1 inputpolarities [lrepeat 64 Negative]

# ------------------------------------------------------------
# DPP-PHA filter parameters
# ------------------------------------------------------------
# all of these will have to be set per PMT!
v27xxpha config adc1 \
    tfrisetime        [lrepeat 64 4] \
    tfretriggerguard  [lrepeat 64 4] \
    efrisetime        [lrepeat 64 4] \
    efflattoptime     [lrepeat 64 4] \
    efpeakingpos      [lrepeat 64 20] \
    efpolezero        [lrepeat 64 4] \
    efpileupguardt    [lrepeat 64 10] \
    efbaselineguardt  [lrepeat 64 12] \
    eflflimitation    [lrepeat 64 false] \
    efbaselineavg     [lrepeat 64 64] \
    effinegain        [lrepeat 64 1.0]

# controls which energy events are saved?
v27xxpha config adc1 eventselector [lrepeat 64 All]

# controls which waveforms are saved?
v27xxpha config adc1 waveselector [lrepeat 64 All]


# I think that's it?

