FLAGS = -pedantic-errors -std=c++11

main.o: main.cpp book.h member.h helper.h
	g++ $(FLAGS) -c $<

book.o: book.cpp book.h
	g++ $(FLAGS) -c $<

member.o: member.cpp member.h
	g++ $(FLAGS) -c $<

library.o: library.cpp book.h member.h helper.h
	g++ $(FLAGS) -c $<

helper.o: helper.cpp helper.h
	g++ $(FLAGS) -c $<

main: main.o book.o member.o library.o helper.o
	g++ $(FLAGS) -c $@

clean:
	rm -f main *.o library.tgz

tar:
	tar -czvf library.tgz *.cpp *.h

.PHONY: clean tar
