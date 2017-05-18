compile:
	gcc htpa.c -o HTPA
run: compile
	./HTPA
clean:
	rm HTPA
