#!/bin/sh

for i in `find code/client/ -iname "*.h"`; do indent $i; done
for i in `find code/client/ -iname "*.c"`; do indent $i; done
for i in `find code/qcommon/ -iname "*.h"`; do indent $i; done
for i in `find code/qcommon/ -iname "*.c"`; do indent $i; done
for i in `find code/renderer/ -iname "*.h"`; do indent $i; done
for i in `find code/renderer/ -iname "*.c"`; do indent $i; done
for i in `find code/server/ -iname "*.h"`; do indent $i; done
for i in `find code/server/ -iname "*.c"`; do indent $i; done
for i in `find code/sys/ -iname "*.h"`; do indent $i; done
for i in `find code/sys/ -iname "*.c"`; do indent $i; done
for i in `find code/null/ -iname "*.h"`; do indent $i; done
for i in `find code/null/ -iname "*.c"`; do indent $i; done

for i in `find code/common/ -iname "*.h"`; do indent $i; done
for i in `find code/common/ -iname "*.c"`; do indent $i; done
for i in `find code/xmap/ -iname "*.h"`; do indent $i; done
for i in `find code/xmap/ -iname "*.h"`; do indent $i; done
#for i in `find code/xmap2/ -iname "*.h"`; do indent $i; done
for i in `find code/xmap2/ -iname "*.c"`; do indent $i; done

for i in `find code/xmass/ -iname "*.h"`; do indent $i; done
for i in `find code/xmass/ -iname "*.c"`; do indent $i; done






