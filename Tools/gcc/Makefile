PLUGINDIR=$(shell $(CXX) -print-file-name=plugin)
CXXFLAGS=-fPIC -Wall -g -I$(PLUGINDIR)/include

pythonapi-plugin.so: pythonapi-plugin.c
	$(CXX) $(CXXFLAGS) -o $@ -shared $^
