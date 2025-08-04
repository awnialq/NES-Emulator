# parse thru the nestest log to extract all the values after each instruction run. 
# this is the benchmark we will comp the emulators output with
lines_bench = open("./testRoms/nestest.log").readlines()
progc_bench = [x.split()[0] for x in lines_bench]
accum_bench = [x[48:].split()[0][2:] for x in lines_bench]
x_bench = [x[53:].split()[0][2:] for x in lines_bench]
y_bench = [x[58:].split()[0][2:] for x in lines_bench]
stackp_bench = [x[68:].split()[0][3:] for x in lines_bench]

# parse thru the emulator cpu log after each instruction (not cycle)
# we will compare this with nestest.log
lines = open("cpu_log.txt").readlines()
progc_exp = [x.split()[1] for x in lines]
accum_exp = [x.split()[3] for x in lines]
x_exp = [x.split()[5] for x in lines]
y_exp = [x.split()[7] for x in lines]
stackp_exp = [x.split()[9] for x in lines]

# the comparator
