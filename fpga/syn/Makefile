include /home/dave/dev/fpga/rtl.dev/Makefile.inc

DIR      = quartus
PROJECT  = top
TOP      = top
SOURCES += deca.qsf deca.sdc
SOURCES += $(RTL) $(wildcard ../rtl/*.sv) top.sv
SOURCES := $(shell realpath $(SOURCES))
MACROS	 = SYNTHESIS ALTERA MAX10

#---------------------------------------------------------------------------------------------------
# Project Script
#---------------------------------------------------------------------------------------------------
.PRECIOUS: $(DIR)/setup_%.tcl
$(DIR)/setup_%.tcl: $(SOURCES)
	mkdir -p $(DIR)
	echo "$$QUARTUS_PROJECT_SCRIPT" > $@

.PHONY: script
script: $(DIR)/setup_$(PROJECT).tcl

#---------------------------------------------------------------------------------------------------
# Project Rules
#---------------------------------------------------------------------------------------------------
$(DIR)/%.qpf: $(DIR)/setup_%.tcl
	cd $(DIR); quartus_sh -t $(notdir $<)

$(DIR)/%.qar:
	cd $(DIR)
	quartus_sh --archive -revision $* -output $@.qar -no_discover $*
	cp $@ $*-$$(date +%h-%d-%Y).qar

.PHONY: project
project: $(DIR)/$(PROJECT).qpf

.PHONY: archive
archive: $(DIR)/$(PROJECT).qar

#---------------------------------------------------------------------------------------------------
# Synthesis
#---------------------------------------------------------------------------------------------------
$(DIR)/%.map.done: $(DIR)/%.qpf
	cd $(DIR); quartus_map $*
	touch $@

.PHONY: synth
synth: $(DIR)/$(PROJECT).map.done

#---------------------------------------------------------------------------------------------------
# Implementation
#---------------------------------------------------------------------------------------------------
$(DIR)/%.fit.done: $(DIR)/%.map.done
	cd $(DIR); quartus_fit $*
	touch $@

$(DIR)/%.sta.done: $(DIR)/%.fit.done
	cd $(DIR); quartus_sta $*
	touch $@

$(DIR)/%.asm.done: $(DIR)/%.fit.done
	cd $(DIR); quartus_asm $*
	touch $@

.PHONY: impl
impl: $(DIR)/$(PROJECT).fit.done $(DIR)/$(PROJECT).sta.done $(DIR)/$(PROJECT).asm.done

#---------------------------------------------------------------------------------------------------
# Program .sof over JTAG
#---------------------------------------------------------------------------------------------------
.PHONY: program
program: $(DIR)/$(PROJECT).sof
	quartus_pgm -m jtag -o "p;$<"

#---------------------------------------------------------------------------------------------------
# Summary
#---------------------------------------------------------------------------------------------------
.PHONY: $(DIR)/%.summary
$(DIR)/%.summary: impl
	@echo ""
	@echo "*** Build Summary ***"                                 
	@grep "logic elements"          $(basename $@).fit.summary
	@grep "combinational functions" $(basename $@).fit.summary
	@grep "logic registers"         $(basename $@).fit.summary
	@grep "memory bits"             $(basename $@).fit.summary
	@grep "Multiplier"              $(basename $@).fit.summary
	@grep "PLL"                     $(basename $@).fit.summary
	@grep "Total pins"              $(basename $@).fit.summary
	@echo ""
	@echo "*** Build Complete ***"
	@echo "Instances Failing Timing: `grep -c "TNS.*:.*-" $(basename $@).sta.summary`"
	@echo ""
	touch $@

.DEFAULT_GOAL := sof
.PHONY: sof
sof: impl $(DIR)/$(PROJECT).summary

.PHONY: clean
clean:
	rm -rf $(DIR)

define QUARTUS_PROJECT_SCRIPT
project_new $(PROJECT) -overwrite
set_global_assignment -name TOP_LEVEL_ENTITY $(TOP)
foreach i {$(SOURCES)} {
    switch -regexp -- $$i {
        {.*\.sv$$}        {set_global_assignment -name SYSTEMVERILOG_FILE $$i}
        {.*\.v$$}         {set_global_assignment -name VERILOG_FILE $$i}
        {.*\.vhd$$}       {set_global_assignment -name VHDL_FILE $$i}
        {.*\.vhdl$$}      {set_global_assignment -name VHDL_FILE $$i}
        {.*\.qip$$}       {set_global_assignment -name QIP_FILE $$i}
        {.*\.qsys$$}      {set_global_assignment -name QSYS_FILE $$i}
        {.*\.sdc$$}       {set_global_assignment -name SDC_FILE $$i}
        {.*\.(qsf|tcl)$$} {set_global_assignment -name SOURCE_TCL_SCRIPT $$i}
    }
}
foreach i {$(INCLUDES)} {
	set_global_assignment -name SEARCH_PATH $$i
}
foreach i {$(MACROS)} {
    set_global_assignment -name VERILOG_MACRO $$i
}
endef
export QUARTUS_PROJECT_SCRIPT
