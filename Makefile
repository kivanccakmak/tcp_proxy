#
# Copyright (C) 2016  Kıvanç Çakmak <kivanccakmak@gmail.com>
# Author: Kıvanç Çakmak <kivanccakmak@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

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



