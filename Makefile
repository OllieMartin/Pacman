SHELL=C:/Windows/System32/cmd.exe
# Universal Makefile   Version: 23.02.2016
#
# Appears to work under Linux, OSX, and Cygwin/Windows.
# Under OSX it should be named "Makefile".
#
# Requires GNU 'make' and 'find'. 
 
# Target Architecture
BOARD := LaFortuna
MCU   := at90usb1286
F_CPU := 8000000UL
 
# Tool Options
CFLAGS    := -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)
# CFLAGS    += -Wl,-u,vfprintf -lprintf_flt -lm  # floating point support 
CFLAGS    += -fno-strict-aliasing  # FATfs does not adhere to strict aliasing
CFLAGS    += -Wno-main             # main() will never return 
CFLAGS    += -Wall -Wextra -pedantic
CFLAGS    += -Wstrict-overflow=5 -fstrict-overflow -Winline              
CHKFLAGS  := 
# CHKFLAGS  += -fsyntax-only
BUILD_DIR := _build
 
# Ignoring hidden directories; sorting to drop duplicates:
CFILES := $(shell find . ! -path "*/\.*" -type f -name "*.c")
CPPFILES := $(shell find . ! -path "*/\.*" -type f -name "*.cpp")
CPATHS := $(sort $(dir $(CFILES)))
CPPATHS += $(sort $(dir $(CPPFILES)))
vpath %.c   $(CPATHS)
vpath %.cpp $(CPPATHS)
HFILES := $(shell find . ! -path "*/\.*" -type f -name "*.h")
HPATHS := $(sort $(dir $(HFILES)))
vpath %.h $(HPATHS)
CFLAGS += $(addprefix -I ,$(HPATHS))
DEPENDENCIES := $(patsubst %.c,$(BUILD_DIR)/%.d,$(notdir $(CFILES)))
DEPENDENCIES += $(patsubst %.cpp,$(BUILD_DIR)/%.d,$(notdir $(CPPILES)))
OBJFILES     := $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(CFILES)))
OBJFILES     += $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(CPPFILES)))
 
.PHONY: upld prom clean check-syntax ?
 
upld: $(BUILD_DIR)/main.hex
	$(info )
	$(info =========== ${BOARD} =============)
	dfu-programmer $(MCU) erase
	dfu-programmer $(MCU) flash $(BUILD_DIR)/main.hex
	dfu-programmer $(MCU) launch --no-reset # requires dfu-programmer >= v0.7.0 
 
prom: $(BUILD_DIR)/main.eep upld
	$(info ======== EEPROM: ${BOARD} ========)
	dfu-programmer $(MCU) flash-eeprom $(BUILD_DIR)/main.eep
 
$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@avr-gcc $(CFLAGS) -MMD -MP -c $< -o $@
 
$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	@avr-g++ $(CFLAGS) -MMD -MP -c $< -o $@
 
$(BUILD_DIR)/%.elf %.elf: $(OBJFILES)
	@avr-gcc -mmcu=$(MCU) -o $@  $^
 
$(BUILD_DIR)/%.hex %.hex: $(BUILD_DIR)/%.elf
	@avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex  $<  "$@"
 
$(BUILD_DIR)/%.eep %.eep: $(BUILD_DIR)/%.elf
	@avr-objcopy -j .eeprom --change-section-lma .eeprom=0 -O ihex $< "$@"
 
 
-include $(sort $(DEPENDENCIES))
 
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
 
# Emacs flymake support
check-syntax:
	@avr-gcc $(CFLAGS) $(CHKFLAGS) -o /dev/null -S $(CHK_SOURCES)
 
clean:
	@$(RM) -rf $(BUILD_DIR)
 
?%:
	@echo '$*=$($*)'
 
?:
	$(info -------------------------------------------------)
	$(info Usage:)
	$(info Source files can be grouped into subdirectories.)
	$(info To build an executable and attempt to upload it,)
	$(info use just "make". If the executable requires EEPROM) 
	$(info initialization, use "make prom".) 
	$(info )
	$(info make mymain.hex --> to build a hex-file for mymain.c)
	$(info make mymain.eep --> for an EEPROM  file for mymain.c)
	$(info make mymain.elf --> for an elf-file for mymain.c)
	$(info make ?CFILES    --> show C source files to be used)
	$(info make ?CPPFILES  --> show C++ source files to be used)
	$(info make ?HFILES    --> show header files found)
	$(info make ?HPATHS    --> show header locations)
	$(info make ?CFLAGS    --> show compiler options)
	$(info )
	$(info See the Makefile for more variables that can be)
	$(info displayed as above.)
	$(info -------------------------------------------------)
	@:
 
 
# The MIT License (MIT)
# 
# Copyright (c) 2014-2015 Klaus-Peter Zauner
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
#===================================================================