# Copyright 2021 Ellogon BV.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
CXX?=g++
PYTHON3?=python3
CFLAGS?=
CFLAGS+=-DWITH_SPINNER
CFLAGS+=$(shell pkg-config vips-cpp --cflags)

CXXFLAGS+=-std=c++17 $(CFLAGS)

LDFLAGS?=-pthread $(shell pkg-config vips-cpp --libs) -ltiff

OBJECTS=tile-generator.o aperio-svs-encoding.o
MAIN_OBJECTS=svg2svs.o

DEPENDENCY_RULES=$(OBJECTS:=.d) $(MAIN_OBJECTS:=.d)

TARGETS=svg2svs

all: svg2svs

svg2svs: svg2svs.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cc compiler-flags
	$(CXX) $(CXXFLAGS)  -c  $< -o $@
	@$(CXX) $(CXXFLAGS) -MM $< > $@.d

-include $(DEPENDENCY_RULES)

clean:
	rm -rf $(TARGETS) $(OBJECTS) $(MAIN_OBJECTS) $(DEPENDENCY_RULES)

compiler-flags: FORCE
	@echo '$(CXX) $(CXXFLAGS) | cmp -s - $@ || echo '$(CXX) $(CXXFLAGS) > $@

.PHONY: FORCE
