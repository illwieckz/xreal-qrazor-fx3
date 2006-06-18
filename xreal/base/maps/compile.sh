#!/bin/sh

case $1 in
	-bsp)
		../../xmap.x86 -v -leaktest -nofog $2.map
		;;

	-fastvis)
		../../xmap.x86 -vis -saveprt -fast $2.map
		;;
		
	-vis)
		../../xmap.x86 -vis -saveprt $2.map
		;;

	-light)
		../../xmap.x86 -light -patchshadows $2.map
		;;

	-xlight)
		../../xmap.x86 -light -extra -patchshadows $2.map
		;;

	-vlight)
		../../xmap.x86 -vlight -v $2.map
		;;

	-onlyents)
		../../xmap.x86 -onlyents $2.map
		;;

	-aas)
		../../bspc.x86 -bsp2aas $2.bsp
		;;
	
	-all)
		../../xmap.x86 -v -leaktest $2.map
		../../xmap.x86 -vis -saveprt $2.map
		#../../xmap.x86 -vlight -v $2.map
		../../bspc.x86 -bsp2aas $2.bsp
		;;
	*)
		echo "specify command: -bsp, -fastvis, -vis, -light, -xlight, -vlight, -aas or -all <mapname>"
		;;
esac

