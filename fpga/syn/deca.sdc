# Note - The ddr3_clk_50 constraint may need to be deleted based on the UniPHY IP
set_time_format -unit ns -decimal_places 3
create_clock -name {deca_max10_clk1_50} -period 20.000  -waveform { 0.000 10.000 } [get_ports {deca_max10_clk1_50}]
derive_pll_clocks
derive_clock_uncertainty
