DOC_DIR     = doc
DOXYGEN     = doxygen
DOXYGEN_DIR = doxygen
DOXYFILE    = $(DOC_DIR)/doxygen.config
FIGS_FILE   = doc/figs


SOURCES += $(wildcard ../receive/*.c)  \
           $(wildcard ../stream/*.c)   \
           $(wildcard ../tx_proxy/*.c) \
           $(wildcard ../rx_proxy/*.c) \
           $(wildcard ../commons/*.c)  \

HEADERS += $(wildcard ../receive/*.h)  \
           $(wildcard ../stream/*.h)   \
           $(wildcard ../tx_proxy/*.h) \
           $(wildcard ../rx_proxy/*.h) \
           $(wildcard ../commons/*.h)  \

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
	(cd doc; rm -rf doxygen)

doxygen: $(DOXYFILE) $(SOURCES) $(HEADERS) README.md
	(cd $(DOC_DIR); rm -rf $(DOXYGEN_DIR))
	$(DOXYGEN) $(DOXYFILE)
	mv $(DOXYGEN_DIR) $(DOC_DIR)
	cp -r $(FIGS_FILE) $(DOC_DIR)/$(DOXYGEN_DIR)/html 

dox_clear:
	(cd doc; rm -rf doxygen)



