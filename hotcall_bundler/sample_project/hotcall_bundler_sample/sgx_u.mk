######## Intel(R) SGX SDK Settings ########
SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= SIM
SGX_ARCH ?= x64
UNTRUSTED_DIR=untrusted

HOTCALL_BUNDLER_TRUSTED_LIB_PATH := /home/jakob/ovs-sgx/hotcall_bundler/src/trusted
HOTCALL_BUNDLER_UNTRUSTED_LIB_PATH := /home/jakob/ovs-sgx/hotcall_bundler/src/untrusted

HOTCALL_BUNDLER_INCLUDE_PATH = /home/jakob/ovs-sgx/hotcall_bundler/include

ifeq ($(shell getconf LONG_BIT), 32)
	SGX_ARCH := x86
else ifeq ($(findstring -m32, $(CXXFLAGS)), -m32)
	SGX_ARCH := x86
endif

ifeq ($(SGX_ARCH), x86)
	SGX_COMMON_CFLAGS := -m32
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x86/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x86/sgx_edger8r
else
	SGX_COMMON_CFLAGS := -m64
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib64
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x64/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x64/sgx_edger8r
endif

ifeq ($(SGX_DEBUG), 1)
ifeq ($(SGX_PRERELEASE), 1)
$(error Cannot set SGX_DEBUG and SGX_PRERELEASE at the same time!!)
endif
endif

ifeq ($(SGX_DEBUG), 1)
        SGX_COMMON_CFLAGS += -O0 -g
else
        SGX_COMMON_CFLAGS += -O2
endif

######## App Settings ########

ifneq ($(SGX_MODE), HW)
	Urts_Library_Name := sgx_urts_sim
else
	Urts_Library_Name := sgx_urts
endif

App_C_Files := $(UNTRUSTED_DIR)/sample.c \
			   $(UNTRUSTED_DIR)/examples.c \
			   $(UNTRUSTED_DIR)/test/test.c \
			   $(UNTRUSTED_DIR)/test/if.c \
			   $(UNTRUSTED_DIR)/test/for.c \
			   $(UNTRUSTED_DIR)/test/do_while.c \
			   $(UNTRUSTED_DIR)/test/while.c \
			   $(UNTRUSTED_DIR)/test/for_each.c \
			   $(UNTRUSTED_DIR)/test/map.c \
			   $(UNTRUSTED_DIR)/test/filter.c
App_Include_Paths := -Iinclude -I$(UNTRUSTED_DIR) -I$(SGX_SDK)/include -I$(HOTCALL_BUNDLER_TRUSTED_LIB_PATH)/include -I$(HOTCALL_BUNDLER_UNTRUSTED_LIB_PATH) -I/home/jakob/ovs-sgx/benchmark/include

App_C_Flags := $(SGX_COMMON_CFLAGS) -fPIC -Wno-attributes $(App_Include_Paths)

# Three configuration modes - Debug, prerelease, release
#   Debug - Macro DEBUG enabled.
#   Prerelease - Macro NDEBUG and EDEBUG enabled.
#   Release - Macro NDEBUG enabled.
ifeq ($(SGX_DEBUG), 1)
        App_C_Flags += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
        App_C_Flags += -DNDEBUG -DEDEBUG -UDEBUG
else
        App_C_Flags += -DNDEBUG -UEDEBUG -UDEBUG
endif

App_Link_Flags := $(SGX_COMMON_CFLAGS) -L$(HOTCALL_BUNDLER_UNTRUSTED_LIB_PATH) -lhotcall_bundler_untrusted  -L$(SGX_LIBRARY_PATH) -l$(Urts_Library_Name) -lpthread -lgtest

ifneq ($(SGX_MODE), HW)
	App_Link_Flags += -lsgx_uae_service_sim
else
	App_Link_Flags += -lsgx_uae_service
endif

App_C_Objects := $(App_C_Files:.c=.o)



ifeq ($(SGX_MODE), HW)
ifneq ($(SGX_DEBUG), 1)
ifneq ($(SGX_PRERELEASE), 1)
Build_Mode = HW_RELEASE
endif
endif
endif


.PHONY: all run

ifeq ($(Build_Mode), HW_RELEASE)
all: sample
	@echo "Build sample [$(Build_Mode)|$(SGX_ARCH)] success!"
	@echo
	@echo "*********************************************************************************************************************************************************"
	@echo "PLEASE NOTE: In this mode, please sign the hotcall_bundler_sample.so first using Two Step Sign mechanism before you run the app to launch and access the enclave."
	@echo "*********************************************************************************************************************************************************"
	@echo

else
all: sample
endif

run: all
ifneq ($(Build_Mode), HW_RELEASE)
	@$(CURDIR)/sample
	@echo "RUN  =>  sample [$(SGX_MODE)|$(SGX_ARCH), OK]"
endif

######## App Objects ########
$(UNTRUSTED_DIR)/hotcall_bundler_sample_u.c: $(SGX_EDGER8R) trusted/hotcall_bundler_sample.edl
	@cd $(UNTRUSTED_DIR) && $(SGX_EDGER8R) --untrusted ../trusted/hotcall_bundler_sample.edl --search-path ../trusted --search-path $(SGX_SDK)/include \
	--search-path $(HOTCALL_BUNDLER_TRUSTED_LIB_PATH)/static_trusted
	@echo "GEN  =>  $@"

$(UNTRUSTED_DIR)/hotcall_bundler_sample_u.o: $(UNTRUSTED_DIR)/hotcall_bundler_sample_u.c
	@$(CC) $(App_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

$(UNTRUSTED_DIR)/%.o: $(UNTRUSTED_DIR)/%.c
	@$(CXX) $(App_C_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

sample: $(UNTRUSTED_DIR)/hotcall_bundler_sample_u.o $(App_C_Objects)
	@$(CXX) $^ -o $@ $(App_Link_Flags)
	@echo "LINK =>  $@"


.PHONY: clean

clean:
	@rm -f sample  $(App_C_Objects) $(UNTRUSTED_DIR)/hotcall_bundler_sample_u.*
