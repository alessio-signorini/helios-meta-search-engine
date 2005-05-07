all: clean
	cd src; make
	
clean:
	rm -f helios

tar:
	cd ..; tar cjf helios/ver/newversion.tar.bz2 helios/Makefile helios/engines.dat helios/*.txt helios/*.TXT helios/src/* helios/parser/*.prs helios/parser/*.txt helios/parser/*.TXT helios/ver/changelog.txt

