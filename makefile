
all: 
	g++  DynamicSched.cpp -o sim
	#g++ -O3 BranchPredictorExp.cpp -o simExp

clean:
	rm -f sim

