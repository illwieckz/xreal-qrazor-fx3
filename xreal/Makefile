default:
	@echo "WARNING: build system doesn't rely on GNU make. Spawning SCons"
	@echo "  use scons -h for help"
	scons

all:
	scons --debug=time
	
clean:
	scons -c