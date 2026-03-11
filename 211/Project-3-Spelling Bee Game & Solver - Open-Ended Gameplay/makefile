build:
	rm -f spBee.exe
	gcc main.c -o spellingBee.exe

run:
	./spellingBee.exe

run_simp:
	./spellingBee.exe -d newsamp.txt

valgrind:
	rm -f spBee_debug.exe
	gcc -g main.c -o spellB_debug.exe
	echo "watched w" > sampleIn.txt
	valgrind -s --tool=memcheck --leak-check=yes --track-origins=yes ./spellB_debug.exe < sampleIn.txt

clean:
	rm -f spellingBee.exe
	rm -f spellB_debug.exe

run_play: build
	./spellingBee.exe -p -r 7

run_sample: build
	printf "acdeit\ne\n" > sampleIn.txt
	./spellingBee.exe -d newsamp.txt < sampleIn.txt

