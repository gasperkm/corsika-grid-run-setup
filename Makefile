all:
	g++ -o corsika_grid_run corsika_grid_run.cpp
	$(info Run program with: ./corsika_grid_run <GRID XRSL script> <CORSIKA-input> <GRID shell script> [<CORSIKA-stackin>])

clean:
	rm -fr corsika_grid_run
