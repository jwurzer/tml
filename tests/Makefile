TML=../build/tml

all: test1 test2

test1:
	$(TML) print-tml tml/test1.tml > tmp/test1.tml
	diff tml/test1.tml tmp/test1.tml
test2:
	$(TML) print-tml tml/test2.tml > tmp/test2.tml
	diff tml/test2.tml tmp/test2.tml

