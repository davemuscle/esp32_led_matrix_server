
sim: 
	vlog $(wildcard ../rtl/*.sv) tb.sv
	vsim -batch -do "vcd file dump.vcd; vcd add -r *; run -all " tb

clean:
	rm -rf *.vcd *.log transcript work *.fst
