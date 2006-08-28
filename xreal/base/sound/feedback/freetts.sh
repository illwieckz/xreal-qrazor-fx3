#!/bin/sh
FREETTS_HOME=/opt/freetts-1.2.1
VOICE=mbrola_us1

for i in `ls *.txt`;
        do java -Dmbrola.base=$FREETTS_HOME/mbrola -jar $FREETTS_HOME/lib/freetts.jar -voice $VOICE -file $i -dumpAudio `basename $i .txt`.wav;
done
