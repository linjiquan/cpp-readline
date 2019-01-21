CC=g++
FLAGS=-std=c++11
LIBS=-lreadline -lpthread

all:
	${CC} ${FLAGS} example/main.cpp src/xma_shell.cpp src/xma_thread.cpp src/xma_application.cpp ${LIBS}
