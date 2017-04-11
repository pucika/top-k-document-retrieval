OBJECT = gst.o GST_test.o 
CXX = g++
CPPFLAGS = -O3 -std=c++11 
LIB = -I /usr/local/include -L /usr/local/lib -lsdsl -ldivsufsort -ldivsufsort64 \
		-lrmqrmm

TARGET:$(OBJECT)
	$(CXX) $(CPPFLAGS)  -o top-k $(OBJECT) $(LIB)
gst.o: gst.cpp gst.h
	$(CXX) -c $(CPPFLAGS) gst.cpp $(LIB)
GST_test.o:GST_test.cpp
	$(CXX) -c $(CPPFLAGS) GST_test.cpp
clean:
	-rm top-k $(OBJECT) 
