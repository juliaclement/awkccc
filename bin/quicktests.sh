#! /bin/sh
cd ~/Projects/c++/awkccc
for i in awk_samples/*.awk; do
	echo $i;
	bin/awkccc :$i;
done
