SRC = $(shell find . -name '*.cpp') $(shell find . -name '*.c')
HEADERS = $(shell find . -name '*.h')
EXCLUDE_SRC = 
FSRC = $(filter-out $(EXCLUDE_SRC), $(SRC))
OBJ = $(FSRC:=.o)

DEP_DIR = .deps

GTEST = /usr/src/gtest/src/gtest-all.cc /usr/src/gtest/src/gtest_main.cc

FLAGS = -fPIC -Wall -Wno-unknown-pragmas -I include
CXXFLAGS = -std=c++14
CFLAGS = 
LINKFLAGS = -I /usr/src/gtest/ $(GTEST) -lz -ldl -pthread

OUTNAME = libttl
OUTFILE = test

DEBVERSION = $(shell git describe --tags --long | cut -d- -f1,2 | cut -c2-)
DEBFOLDER = $(OUTNAME)_$(DEBVERSION)

.PHONY: clean debug release

release: FLAGS += -O2 -march=native
release: $(OUTFILE)

debug: FLAGS += -g -O0 -D_LIBCPP_DEBUG=1 -fprofile-arcs -ftest-coverage -fsanitize=address -fsanitize=leak -fsanitize=undefined
debug: LINKFLAGS += -lgcov --coverage -fsanitize=address -fsanitize=leak -fsanitize=undefined
debug: $(OUTFILE)

$(OUTFILE): $(OBJ)
	@echo Generating binary
	@$(CXX) -o $@ -lasan $^ $(LINKFLAGS)
	@echo Build done

%.cc.o: %.cc
	@echo Building $<
	@$(CXX) -c $(FLAGS) $(CXXFLAGS) $< -o $@
	@mkdir -p `dirname $(DEP_DIR)/$@.d`
	@$(CXX) -c $(FLAGS) $(CXXFLAGS) -MT '$@' -MM $< > $(DEP_DIR)/$@.d

%.cpp.o: %.cpp
	@echo Building $<
	@$(CXX) -c $(FLAGS) $(CXXFLAGS) $< -o $@
	@mkdir -p `dirname $(DEP_DIR)/$@.d`
	@$(CXX) -c $(FLAGS) $(CXXFLAGS) -MT '$@' -MM $< > $(DEP_DIR)/$@.d

%.c.o: %.c
	@echo Building $<
	@$(CC) -c $(FLAGS) $(CFLAGS) $< -o $@
	@mkdir -p `dirname $(DEP_DIR)/$@.d`
	@$(CC) -c $(FLAGS) $(CFLAGS) -MT '$@' -MM $< > $(DEP_DIR)/$@.d

clean:
	@echo Removing binary
	@rm -f $(OUTFILE)
	@echo Removing objects
	@rm -f $(OBJ)
	@echo Removing trace files
	@rm -f $(OBJ:%.o=%.gcda)
	@rm -f $(OBJ:%.o=%.gcno)
	@echo Removing dependency files
	@rm -rf $(DEP_DIR)
	@echo Removing package folders
	@rm -r -f $(DEBFOLDER)

package: $(DEBFOLDER).deb

install: $(DEBFOLDER).deb
	@sudo dpkg -i $(DEBFOLDER).deb

$(DEBFOLDER).deb: $(HEADERS)
	@rm -r -f $(DEBFOLDER)
	@echo Creating package
	@mkdir -p $(DEBFOLDER)/DEBIAN
	@mkdir -p $(DEBFOLDER)/usr/include/ttl/
	@echo "Package: $(OUTNAME)" >> $(DEBFOLDER)/DEBIAN/control
	@echo "Version: $(DEBVERSION)" >> $(DEBFOLDER)/DEBIAN/control
	@echo "Section: devel" >> $(DEBFOLDER)/DEBIAN/control
	@echo "Priority: optional" >> $(DEBFOLDER)/DEBIAN/control
	@echo "Architecture: all" >> $(DEBFOLDER)/DEBIAN/control
	@echo "Maintainer: Dominik Thalhammer <dominik@thalhammer.it>" >> $(DEBFOLDER)/DEBIAN/control
	@echo "Description: Tiny Template Library" >> $(DEBFOLDER)/DEBIAN/control
	@echo "Homepage: https://github.com/Thalhammer/ttl" >> $(DEBFOLDER)/DEBIAN/control
	@cp include/ttl/* ${DEBFOLDER}/usr/include/ttl/ -R
	@fakeroot dpkg-deb --build $(DEBFOLDER)

-include $(OBJ:%=$(DEP_DIR)/%.d)
