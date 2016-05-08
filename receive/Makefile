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

GCC = gcc

TARGET = receive

SRC = receive.c

DEP += ../commons/argv_reader/argv_read.c

CFLAGS = -DRECV

OUT = output.txt

default: all

all: $(TARGET)

$(TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET) -lconfig $(CFLAGS)

clean:
	rm -rf $(TARGET) $(OUT) $(OUT)
