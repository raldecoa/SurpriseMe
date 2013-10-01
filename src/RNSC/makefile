CC = g++
FLAGS = -Wall -O3


all: rnsc rnscfilter rnscconvert

rnsc: ./source/rnsc/main.cpp ./source/rnsc/linkedList.cpp ./source/rnsc/linkedList2.cpp ./source/rnsc/linkedList3.cpp ./source/rnsc/graph.cpp ./source/rnsc/graph2.cpp ./source/rnsc/graph3.cpp ./source/rnsc/graph4.cpp ./source/rnsc/graph5.cpp ./source/rnsc/graph6.cpp ./source/rnsc/printing.cpp ./source/rnsc/experiment.cpp ./source/rnsc/miscFunctions.cpp ./source/rnsc/definitions.h ./source/rnsc/experiment.h ./source/rnsc/graph.h ./source/rnsc/linkedList.h ./source/rnsc/miscFunctions.h ./source/rnsc/statsBook.h
	$(CC) $(FLAGS) -o rnsc ./source/rnsc/main.cpp ./source/rnsc/linkedList.cpp ./source/rnsc/linkedList2.cpp ./source/rnsc/linkedList3.cpp ./source/rnsc/graph.cpp ./source/rnsc/graph2.cpp ./source/rnsc/graph3.cpp ./source/rnsc/graph4.cpp ./source/rnsc/graph5.cpp ./source/rnsc/graph6.cpp ./source/rnsc/printing.cpp ./source/rnsc/experiment.cpp ./source/rnsc/miscFunctions.cpp

rnscfilter: ./source/filter/cycList.cpp ./source/filter/cycList.h ./source/filter/definitions.h ./source/filter/form.cpp ./source/filter/graph.cpp ./source/filter/graph.h ./source/filter/linkedList.cpp ./source/filter/linkedList.h ./source/filter/linkedList2.cpp ./source/filter/linkedList3.cpp ./source/filter/miscFunctions.cpp ./source/filter/miscFunctions.h ./source/filter/moveScheme4.cpp 
	$(CC) $(FLAGS) -o rnscfilter ./source/filter/graph.cpp ./source/filter/linkedList.cpp ./source/filter/form.cpp

rnscconvert: ./source/convert/definitions.h ./source/convert/experiment.cpp ./source/convert/experiment.h ./source/convert/graph.cpp ./source/convert/graph.h ./source/convert/graph2.cpp ./source/convert/graph3.cpp ./source/convert/graph4.cpp ./source/convert/graph5.cpp ./source/convert/graph6.cpp ./source/convert/linkedList.cpp ./source/convert/linkedList.h ./source/convert/linkedList2.cpp ./source/convert/linkedList3.cpp ./source/convert/linkedList4.cpp ./source/convert/main.cpp ./source/convert/miscFunctions.cpp ./source/convert/miscFunctions.h ./source/convert/printing.cpp ./source/convert/statsBook.h
	$(CC) $(FLAGS) -o rnscconvert ./source/convert/main.cpp ./source/convert/linkedList.cpp ./source/convert/linkedList2.cpp ./source/convert/linkedList3.cpp ./source/convert/graph.cpp ./source/convert/graph2.cpp ./source/convert/graph3.cpp ./source/convert/graph4.cpp ./source/convert/graph5.cpp ./source/convert/graph6.cpp ./source/convert/printing.cpp ./source/convert/experiment.cpp ./source/convert/miscFunctions.cpp

tar:
	tar -zcvf rnsc.tar.gz README.TXT makefile data source
zip:
	zip -r rnsc.zip README.TXT makefile data source
clean:
	rm -f rnsc rnscfilter rnscconvert