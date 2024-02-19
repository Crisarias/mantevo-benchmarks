TRILINOS_INSTALLATION=../EPETRA_OPENMP_BUILD
ROOTDIR=..
ROOT=EpetraBenchmarkTest
 
include $(TRILINOS_INSTALLATION)/include/Makefile.export.Trilinos
 
INCDIR=$(Trilinos_INCLUDE_DIRS) $(Trilinos_TPL_INCLUDE_DIRS)
LIBDIR=$(Trilinos_LIBRARY_DIRS) $(Trilinos_TPL_LIBRARY_DIRS)
LIBS=$(Trilinos_LIBRARIES) $(Trilinos_TPL_LIBRARIES)
 
CXX=$(Trilinos_CXX_COMPILER)
CXXFLAGS=$(INCDIR) $(Trilinos_CXX_COMPILER_FLAGS)
 
TARGET=$(ROOT).exe
OBJ=$(ROOT).o
 
$(TARGET): $(OBJ)
	$(CXX) -o $(TARGET) $(CXXFLAGS) $(OBJ) $(LIBDIR) $(LIBS)

$(ROOT).o: $(ROOTDIR)/$(ROOT).cpp
	$(CXX) -c $(CXXFLAGS) $(ROOTDIR)/$(ROOT).cpp
 
clean:
	rm -rf $(OBJ) $(TARGET) *~
 
again: clean $(TARGET)
