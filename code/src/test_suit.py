import subprocess, sys

concrete, abs_no_wd, abs_wd = [], [], []

params = ["./unrelyzer", "-c"]
params.extend(sys.argv[1:])

for i in xrange(0, 10):
	proc = subprocess.Popen(params, stdout=subprocess.PIPE)
	while True:
		line = proc.stdout.readline()
		if line != '':
			if line.startswith("---------------------- Result of Concrete Analysis ("):
				concrete.append(int(line.replace("---------------------- Result of Concrete Analysis (", "").replace(" clock ticks) ----------------------", "")))
		else:
			break

params = ["./unrelyzer", "-a"]
params.extend(sys.argv[1:])

for i in xrange(0, 10):
	proc = subprocess.Popen(params, stdout=subprocess.PIPE)
	while True:
		line = proc.stdout.readline()
		if line != '':
			if line.startswith("---------------------- Result of Abstract Analysis ("):
				abs_no_wd.append(int(line.replace("---------------------- Result of Abstract Analysis (", "").replace(" clock ticks) ----------------------", "")))
		else:
			break

params = ["./unrelyzer", "-aw"]
params.extend(sys.argv[1:])

for i in xrange(0, 10):
	proc = subprocess.Popen(params, stdout=subprocess.PIPE)
	while True:
		line = proc.stdout.readline()
		if line != '':
			if line.startswith("---------------------- Result of Abstract Analysis ("):
				abs_wd.append(int(line.replace("---------------------- Result of Abstract Analysis (", "").replace(" clock ticks) ----------------------", "")))
		else:
			break

#params = ["./unrelyzer", "-f"]
#params.extend(sys.argv[1:])
#proc = subprocess.Popen(params, stdout=subprocess.PIPE)
#subprocess.Popen(["dot", "-Tpng", "cfg.dot", "-o", "cfg.png"], stdout=subprocess.PIPE)

print "\nEach analysis has been performed 10 times and the following results are average of that"
print "---------------------------------------------------------------------------------------\n"
print "Concrete Analysis : ", str(float(reduce(lambda x,y: x+y, concrete)) / 10)
print "Abstract Analysis : ", str(float(reduce(lambda x,y: x+y, abs_no_wd)) / 10)
print "Abstract Analysis (widening) : ", str(float(reduce(lambda x,y: x+y, abs_wd)) / 10)
