######## Intel(R) SGX SDK Settings ########
SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= SIM
SGX_ARCH ?= x64

HOTCALL_BUNDLER_INCLUDE_PATH = /home/jakob/ovs-sgx/hotcall_bundler/include
HOTCALL_BUNDLER_LIB_PATH := /home/jakob/ovs-sgx/hotcall_bundler/src/trusted

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

ifneq ($(SGX_MODE), HW)
	Trts_Library_Name := sgx_trts_sim
	Service_Library_Name := sgx_tservice_sim
else
	Trts_Library_Name := sgx_trts
	Service_Library_Name := sgx_tservice
endif

Crypto_Library_Name := sgx_tcrypto

Hotcall_bundler_sample_C_Files := trusted/hotcall_bundler_sample.c trusted/ecalls.c

Hotcall_bundler_sample_Include_Paths := -Iinclude -Itrusted -I$(SGX_SDK)/include -I$(SGX_SDK)/include/tlibc -I$(SGX_SDK)/include/libcxx \
 -I$(HOTCALL_BUNDLER_INCLUDE_PATH)


Flags_Just_For_C := -Wno-implicit-function-declaration -std=c11
Common_C_Cpp_Flags := $(SGX_COMMON_CFLAGS) -nostdinc -fvisibility=hidden -fpie -fstack-protector $(Hotcall_bundler_sample_Include_Paths) -fno-builtin-printf -I.
Hotcall_bundler_sample_C_Flags := $(Flags_Just_For_C) $(Common_C_Cpp_Flags)

Hotcall_bundler_sample_Link_Flags := $(SGX_COMMON_CFLAGS) -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L$(SGX_LIBRARY_PATH) \
	-Wl,--whole-archive -l$(Trts_Library_Name) -Wl,--no-whole-archive \
	-Wl,--start-group -lsgx_tstdc -lsgx_tcxx -l$(Crypto_Library_Name) -l$(Service_Library_Name) -L$(HOTCALL_BUNDLER_LIB_PATH) -lhotcall_bundler_trusted -lgtest -Wl,--end-group \
	-Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
	-Wl,--defsym,__ImageBase=0 \
	-Wl,--version-script=trusted/hotcall_bundler_sample.lds

Hotcall_bundler_sample_C_Objects := $(Hotcall_bundler_sample_C_Files:.c=.o)

ifeq ($(SGX_MODE), HW)
ifneq ($(SGX_DEBUG), 1)
ifneq ($(SGX_PRERELEASE), 1)
Build_Mode = HW_RELEASE
endif
endif
endif


.PHONY: all run

ifeq ($(Build_Mode), HW_RELEASE)
all: hotcall_bundler_sample.so
	@echo "Build enclave hotcall_bundler_sample.so [$(Build_Mode)|$(SGX_ARCH)] success!"
	@echo
	@echo "*********************************************************************************************************************************************************"
	@echo "PLEASE NOTE: In this mode, please sign the hotcall_bundler_sample.so first using Two Step Sign mechanism before you run the app to launch and access the enclave."
	@echo "*********************************************************************************************************************************************************"
	@echo
else
all: hotcall_bundler_sample.signed.so
endif

run: all
ifneq ($(Build_Mode), HW_RELEASE)
	@$(CURDIR)/app
	@echo "RUN  =>  app [$(SGX_MODE)|$(SGX_ARCH), OK]"
endif


######## hotcall_bundler_sample Objects ########
trusted/hotcall_bundler_sample_t.c: $(SGX_EDGER8R) ./trusted/hotcall_bundler_sample.edl
	@cd ./trusted && $(SGX_EDGER8R) --trusted ../trusted/hotcall_bundler_sample.edl --search-path ../trusted --search-path $(SGX_SDK)/include \
	--search-path $(HOTCALL_BUNDLER_LIB_PATH)/static_trusted
	@echo "GEN  =>  $@"

trusted/hotcall_bundler_sample_t.o: ./trusted/hotcall_bundler_sample_t.c
	@$(CC) $(Hotcall_bundler_sample_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

trusted/%.o: trusted/%.c
	@$(CC) $(Hotcall_bundler_sample_C_Flags) -c $< -o $@
	@echo "CC  <=  $<"

hotcall_bundler_sample.so: trusted/hotcall_bundler_sample_t.o $(Hotcall_bundler_sample_C_Objects)
	@$(CXX) $^ -o $@ $(Hotcall_bundler_sample_Link_Flags)
	@echo "LINK =>  $@"

hotcall_bundler_sample.signed.so: hotcall_bundler_sample.so
	@$(SGX_ENCLAVE_SIGNER) sign -key trusted/hotcall_bundler_sample_private.pem -enclave hotcall_bundler_sample.so -out $@ -config trusted/hotcall_bundler_sample.config.xml
	@echo "SIGN =>  $@"
clean:
	@rm -f hotcall_bundler_sample.* trusted/hotcall_bundler_sample_t.*  $(Hotcall_bundler_sample_C_Objects)
