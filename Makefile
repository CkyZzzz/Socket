CPP = g++
CPPFLAGS  = -g -Wall

all: 
	$(CPP) $(CPPFLAGS) -o client client.cpp
	$(CPP) $(CPPFLAGS) -o serverAoutput serverA.cpp
	$(CPP) $(CPPFLAGS) -o serverBoutput serverB.cpp
	$(CPP) $(CPPFLAGS) -o monitoroutput monitor.cpp
	$(CPP) $(CPPFLAGS) -o awsoutput aws.cpp

serverA:
	./serverAoutput
serverB:
	./serverBoutput
monitor:
	./monitoroutput
aws:
	./awsoutput
