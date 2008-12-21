#!/bin/sh

COMPILER='../../xmap2.x86'
MAP='q4bolledm2.map'

case $1 in
	-bsp)
		$COMPILER -fs_basepath ../.. -fs_game "base" -game "xreal" -v -leaktest $MAP
		;;

	-fastvis)
		$COMPILER -fs_basepath ../.. -fs_game "base" -game "xreal" -vis -fast $MAP
		;;
		
	-vis)
		$COMPILER -fs_basepath ../.. -fs_game "base" -game "xreal" -vis $MAP
		;;

	-light)
		$COMPILER -fs_basepath ../.. -fs_game "base" -game "xreal" -light -fast -filter -v -samplesize 4 -lightmapsize 2048 -super 2 -areascale 1.0 -pointscale 1.0 -skyscale 1.0 $MAP
		;;

	-all)
		$COMPILER -fs_basepath ../.. -fs_game "tremulous" -game "xreal" -v -leaktest $MAP
		# TODO
		;;
	*)
		echo "specify command: -bsp, -fastvis, -vis, -light, or -all"
		;;
esac

