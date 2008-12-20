#!/bin/sh

COMPILER='../../xmap2.x86'
MAP='tremor.map'

case $1 in
	-bsp)
		$COMPILER -fs_basepath ../.. -fs_game "tremulous" -game "xreal" -v -leaktest $MAP
		;;

	-fastvis)
		$COMPILER -fs_basepath ../.. -fs_game "tremulous" -game "xreal" -vis -fast $MAP
		;;
		
	-vis)
		$COMPILER -fs_basepath ../.. -fs_game "tremulous" -game "xreal" -vis $MAP
		;;

	-light)
		$COMPILER -fs_basepath ../.. -fs_game "tremulous" -game "xreal" -light -fast -filter -v -samplesize 1 -lightmapsize 2048 -super 2 -pointscale 3.0 -skyscale 1.0 $MAP
		;;

	-all)
		$COMPILER -fs_basepath ../.. -fs_game "tremulous" -game "xreal" -v -leaktest $MAP
		# TODO
		;;
	*)
		echo "specify command: -bsp, -fastvis, -vis, -light, or -all"
		;;
esac

