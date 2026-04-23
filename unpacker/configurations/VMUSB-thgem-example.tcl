#3/10/26

set runTest {}
for {set i 101} {$i <= 120} {incr i} {
	lappend runTest $i
}

unpacker create thgem

# Make sure you don't have trailling '/' in your path
thgem input stagearea "/mnt/rawdata/e23025/complete"
thgem input add runs $runTest


# Same setup as your VMUSB stack
# If you change the daqconfig.tcl, you need to change this as well
# The order in which "stack module" is called needs to reflect the "stack config event -modules" call in daqconfig.tcl
set tdcDepth 16
thgem stack module TDC TDC1190 -depth $tdcDepth -referenceChannel 0 -nchannels 32
thgem stack module ADC ADCV792

thgem tree create rawTree
rawTree config \
    -title "ThGEM raw data" \
    -description "run=%RUN%" \
    -outdir "/mnt/analysis/e23025/unpacked"

rawTree branch TDC -channels 32 -depth $tdcDepth -default -1e6
rawTree branch ADC -channels 32 -default -1e6

rawTree add $runTest

# Mapping tree to list TDC and ADC channels to detector specific stuff
# TDC Mapping - don't worry about the 0 after the name 
# 	- you need it, it does nothing in this context

rawTree map TDC MASTER_TRIG 		[list TDC 0] 
rawTree map TDC BR_BOT 			[list TDC 1]
rawTree map TDC BL_BOT 			[list TDC 2]
rawTree map TDC PPAC_ANODE 		[list TDC 3]
rawTree map TDC PX1 			[list TDC 4] 
rawTree map TDC PX2 			[list TDC 5] 
rawTree map TDC PY1 			[list TDC 6] 
rawTree map TDC PY2 			[list TDC 7] 
rawTree map TDC MON_TOP 		[list TDC 8] 
rawTree map TDC MON_BOT 		[list TDC 9] 
rawTree map TDC MON_TOP_OR_BOT 		[list TDC 10] 
rawTree map TDC MWPC_TRIG 		[list TDC 11] 
rawTree map TDC L_AND_R_TRIG 		[list TDC 12] 
rawTree map TDC BR_Y1 			[list TDC 16]
rawTree map TDC BR_Y2 			[list TDC 18]
rawTree map TDC BR_X1 			[list TDC 20]
rawTree map TDC BR_X2 			[list TDC 22]
rawTree map TDC BL_Y1 			[list TDC 24]
rawTree map TDC BL_Y2 			[list TDC 26]
rawTree map TDC BL_X1 			[list TDC 28]
rawTree map TDC BL_X2 			[list TDC 30]

rawTree map ADC BR_BOT 			[list ADC 0]
rawTree map ADC BL_BOT 			[list ADC 2]
rawTree map ADC MON_TOP 		[list ADC 4]
rawTree map ADC MON_BOT 		[list ADC 6]
