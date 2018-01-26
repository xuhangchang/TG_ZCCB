CC = gcc
FLAGS = -Wall -g -o

FEATURE_SRC = extractFeature.c
MATCH_SRC = match.c
RATE_SRC = rate.c

EXTRACT_FEA = ../bin/extractFeature
MATCH = ../bin/match
RATE = ../bin/rate

all:extractFeature match rate
extractFeature: $(FEATURE_SRC)
	$(CC) $(FLAGS) ${EXTRACT_FEA} $(FEATURE_SRC) -lrt -ldl -L../lib -lTGFingerVeinApi -Wl,-rpath ../lib 
match: $(MATCH_SRC)
	$(CC) $(FLAGS) ${MATCH} $(MATCH_SRC) -lrt -ldl -L../lib -lTGFingerVeinApi -Wl,-rpath ../lib 
rate: $(RATE_SRC)
	$(CC) $(FLAGS) ${RATE} $(RATE_SRC) -lrt -ldl 
  

clean:
	rm -rf ${EXTRACT_FEA} ${MATCH} ${RATE}
	
.PHONY: clean
