CC = g++
FLAGS = --std=c++11
EXEC = solver

all:
	@echo "Building solver..."
	$(CC) $(FLAGS) PMS_solver.cpp -o $(EXEC)
	@echo "Done."

clean:
	@echo "Cleaning up..."
	rm $(EXEC)
	@echo "Done."
