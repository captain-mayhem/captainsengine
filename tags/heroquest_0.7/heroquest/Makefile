all:
	@echo
	@echo This is the cool Master-Makefile that does everything.
	@echo It assumes that the Engine-folder is in the .. directory
	@echo
	@echo Compiling Client...
	@cd HQClient; make
	@echo
	@echo Compiling Server...
	@cd HQServer; make
	@echo
	@echo Compiling Editor...
	@cd HQEditor; make
	@echo
	@echo Compiling obj2hmb...
	@cd ../Engine/utilities/obj2hmb; make
	@echo
	@echo Making binary models...
	@cd ../Engine/utilities/obj2hmb; ./models
	@echo
	@echo Making binary levels...
	@cd HQEditor; ./levels; ./copy
	@echo
	@echo Done! Everything is now compiled and exactly in the right place
	@echo Have fun!
