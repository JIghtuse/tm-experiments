GHC := ghc
PROGS := histogram/bin/hist \
	haskell-example/transfer

all: build run

build: $(PROGS)

histogram/bin/hist:
	make -C histogram

haskell-example/transfer:
	make -C haskell-example


run: $(PROGS)
	./histogram/bin/hist
	./haskell-example/transfer
	clojure ./clojure-example/transfer.clj

clean:
	make -C histogram clean
	make -C haskell-example clean
