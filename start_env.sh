#!/usr/bin/bash

export basePath="$(pwd)"
export imgName="$(basename $(pwd))"

export SINGULARITY_SHELL="/bin/bash"
export APPTAINER_SHELL="/bin/bash"
export imgBullseye="/usr/opt/nscl-bullseye.img"
export SING_IMAGE=$imgBullseye #DAQROOT specific, uses SING_IMAGE variable
echo -e "\033[1;32mStarting bullseye container ${imgName}\033[0m\n"

echo /usr/opt/opt-bullseye:/usr/opt > ~/.singularity_bindpoints
echo /scratch >> ~/.singularity_bindpoints
echo /usr/lib/x86_64-linux-gnu/modulecmd.tcl >> ~/.singularity_bindpoints

singularity exec --pwd $basePath --workdir $basePath \
	--bind /usr/opt/opt-bullseye:/usr/opt,/scratch \
	--bind /usr/lib/x86_64-linux-gnu/modulecmd.tcl \
	--bind /usr/opt:/non-container \
	$imgBullseye /bin/bash --rcfile $basePath/shell/shellrc
