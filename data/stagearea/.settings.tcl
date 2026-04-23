set EventLogger /usr/opt/nscldaq/12.1-018/bin/eventlog
set EventLoggerRing tcp://localhost/browns
set EventLogUseNsrcsFlag 1
set EventLogAdditionalSources 0
set EventLogUseGUIRunNumber 0
set EventLogUseChecksumFlag 1
set EventLogRunFilePrefix run
set StageArea /projects/hira/sam/unpacker-26/WashU/stagearea
set run 0
set title {}
set recording 0
set timedRun 0
set duration 0
set dataSources {{host localhost parameters {--ring=RAWDATA --no-barriers} path /projects/hira/sam/unpacker-26/WashU/readout/Readout provider SSHPipe sourceid 0 wdir /projects/hira/sam/unpacker-26/WashU/readout}}
set segmentsize 1000000
