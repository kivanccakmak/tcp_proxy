default: all

all:
	(cd receive; make)
	(cd stream; make)
	(cd tx_proxy; make)
	(cd rx_proxy; make)
clean:
	(cd receive; make clean)
	(cd stream; make clean)
	(cd tx_proxy; make clean)
	(cd rx_proxy; make clean)
