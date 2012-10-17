#	Template example for using this file
#
#	TARGETS			:= main1
#	main1_DEPS		:= main1.cpp_o module1.cpp_o module2.cpp_o
#
#	SOURCES_CPP		:= module1.cpp module2.cpp main1.cpp
#	modul1.cpp_DEPS	:= module1.h
#	modul2.cpp_DEPS	:= module2.h
#
#	SOURCES_C		:= module1.c module2.c
#
#	include common.mk


# Project directories
SRCDIR  := src
OBJDIR  := obj
BINDIR  := bin

# Cuda
CUDA_INSTALL_PATH	:= /opt/cuda
CUDA_SDK_PATH		:= /opt/cuda/sdk

# Compiler settings
CC		:= gcc
CXX		:= g++
LINK	:= g++ -fPIC
NVCC	:= $(CUDA_INSTALL_PATH)/bin/nvcc 

# Libraries and include directories
LIBS	+= -L$(CUDA_INSTALL_PATH)/lib 
INCS	+= -I. -I$(CUDA_INSTALL_PATH)/include

# Compiler flags
CFLAGS		+= $(INCS) -Wall
CXXFLAGS	+= $(INCS) -Wall
NVCCFLAGS	+= $(INCS)
LINKFLAGS	+= 

# Default rule
PHONY += all
all: $(TARGETS)

mkdir	= @if [ ! -d $(1) ]; then mkdir -p $(1); fi;

# Objects rules
.SECONDEXPANSION:
$(addprefix $(OBJDIR)/,$(SOURCES_C:=_o)): $(OBJDIR)/%.c_o: $$(addprefix $(SRCDIR)/,%.c $$(%.c_DEPS)); $(call mkdir,$(OBJDIR))
	$(CC) $(CFLAGS) -c $< -o $@
.SECONDEXPANSION:
$(addprefix $(OBJDIR)/,$(SOURCES_CPP:=_o)): $(OBJDIR)/%.cpp_o: $$(addprefix $(SRCDIR)/,%.cpp $$(%.cpp_DEPS)); $(call mkdir,$(OBJDIR))
	$(CXX) $(CXXFLAGS) -c $< -o $@
.SECONDEXPANSION:
$(addprefix $(OBJDIR)/,$(SOURCES_CU:=_o)): $(OBJDIR)/%.cu_o: $$(addprefix $(SRCDIR)/,%.cu $$(%.cu_DEPS)); $(call mkdir,$(OBJDIR))
	$(NVCC) $(NVCCFLAGS) -c $< -o $@

# Targets rules
.SECONDEXPANSION:
$(TARGETS): $$(addprefix $(OBJDIR)/,$$($$@_DEPS)); $(call mkdir,$(BINDIR))
	$(LINK) $(LINKFLAGS) -o $(BINDIR)/$@ $^ $(LIBS)
	
# Clean
PHONY += clean
clean:
	rm -rf $(OBJDIR) $(BINDIR) 

.PHONY: $(PHONY)