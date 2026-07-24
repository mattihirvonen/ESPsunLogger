
# Replace following variable values with real values !!!
# Use notation ":=" allow give variable values from command line like
#
#   make  USER=matti  PASSWD=secretword
#
APP    := mqttLogger
USER   := root
PASSWD := rootxx
TARGET := 192.168.1.184
FOLDER := /root

#-----------------------------------------------------------
# Code::Blocks dependencies trick

Debug:   all
Release: all

cleanDebug:   clean
cleanRelease: cleanDebug

#-----------------------------------------------------------

all: scp  # mqttLogger

CFLAGS = -Wall -Wextra


udpLogger:  udpLogger.c
	gcc     udpLogger.c  $(CFLAGS) -o udpLogger


mqttLogger: mqttLogger.cpp  Makefile
	g++     mqttLogger.cpp  $(CFLAGS) -o mqttLogger -lmosquitto


clean:
	rm      mqttLogger


scp: mqttLogger
	sshpass -p $(PASSWD)  scp  mqttLogger  $(USER)@$(TARGET):$(FOLDER)/mqttLogger
