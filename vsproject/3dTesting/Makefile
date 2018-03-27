FLAGS = -Zi
COMPILER = cl.exe 
SRC = \
	main.c
OUTPUT = -o bin/app.exe
INC = include/	

all:
	cls
	$(COMPILER) $(SRC) $(FLAGS) -I$(INC) $(OUTPUT)
run:
	cls
	cd bin
	app.exe 
	cd..