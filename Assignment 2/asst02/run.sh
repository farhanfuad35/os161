dir=~/os161
if [ ! -e $dir ]; then
	echo 'os161 is not found in ~/os161 directory. Please enter the full directory of where os161 has been installed. [ex: /mnt/academics/os161]:'
	read dir
	not_found=0
	while [ $not_found -eq 0 ]
	do
		root="${dir}/root"
		if [ -d $root ]; then
			ln -s $dir ~/os161
			not_found=1
		else
			echo 'os161 is not found in the provided directory. Please enter the correct directory. [ex: /mnt/academics/os161]:'
			read dir
		fi
	done
fi
cd src/kern/conf/
./config ASST2
printf "\n==========================\n\n"
cd ../compile/ASST2/
bmake depend
printf "\n==========================\n\n"
bmake
printf "\n==========================\n\n"
bmake install
printf "\n==========================\n\n"
cd ~/os161/root
sys161 kernel-ASST2