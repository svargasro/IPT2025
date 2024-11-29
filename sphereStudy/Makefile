all:cAvsRe.png Fyvst.png




puntoD.x: puntoD.cpp
	g++ $^ -o $@
	./puntoD.x

cAvsRe.png Fyvst.png: plot2.py puntoD.x
	python3 plot2.py

clean:
	rm -f *.o *.x *.dat
