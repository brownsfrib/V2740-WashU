# One quick note:
#	On the 'input stagearea' and '-outdir' lines, make sure you do NOT have a trailing '/' character
# 	This is an easy fix on the unpacker, but I haven't gotten around to it yet :/

unpacker create V2740Unpacker

set runNumbers [list 19 20 21]

V2740Unpacker input stagearea "/the/correct/path/to/your/data/WashU/stagearea/complete"
V2740Unpacker input add runs $runNumbers

V2740Unpacker stack module v2740 V2740 -recordsamples 64 -nsPerSample 8

V2740Unpacker tree create V2740Tree
V2740Tree config \
    -title "V2740 CsI unpacker testing" \
    -description "run=%RUN%" \
    -outdir "/the/correct/path/to/WashU/data/unpacked"

# Keep the depth = number of channels in the V2740
# Writes out number of hits, which channel got hit, timestamp, energy, and waveform
V2740Tree wavebranch v2740 -name Trace -nsamples 64 -depth 64 -default -1e2
V2740Tree add $runNumbers

