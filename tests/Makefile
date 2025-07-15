TML=../build/tml

all: test1 test1tl test2 test2tl

test1:
	$(TML) print-tml tml/test1.tml > tmp/test1.tml
	diff tml/test1.tml tmp/test1.tml
test1tl:
	$(TML) print-tml-lines tml/test1.tml > tmp/test1tl.tml
	diff tml/test1.tml tmp/test1tl.tml
test2:
	$(TML) print-tml tml/test2.tml > tmp/test2.tml
	diff tml/test2.tml tmp/test2.tml
test2tl:
	$(TML) print-tml-lines tml/test2.tml > tmp/test2tl.tml
	diff tml/test2.tml tmp/test2tl.tml

