######## Intel(R) SGX SDK Settings ########
SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= SIM
SGX_ARCH ?= x64

HOTCALL_BUNDLER_INCLUDE_PATH = /home/jakob/ovs-sgx/hotcall_bundler/include
HOTCALL_BUNDLER_LIB_PATH := /home/jakob/ovs-sgx/hotcall_bundler/src

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

Enclave_C_Files := trusted/enclave.c \
										 trusted/lib/util.c \
										 trusted/lib/rand.c \
										 trusted/lib/hmap.c \
										 trusted/lib/list.c \
										 trusted/lib/classifier.c \
										 trusted/lib/flow.c \
										 trusted/lib/hash.c \
										 trusted/lib/match.c \
										 trusted/enclave-utils.c \
										 trusted/shared-memory-trusted.c \
										 trusted/cache-trusted.c \
										 trusted/lib/meta-flow.c \
										 trusted/lib/sgx_htons.c \
										 trusted/lib/ofpacts.c \
										 trusted/eviction/eviction.c \
										 trusted/cls_rule/cls-rule.c \
										 trusted/dpif/dpif.c \
										 trusted/ofproto/ofproto.c \
										 trusted/stats/stats.c \
										 trusted/oftable/oftable.c \
										 trusted/enclave-batch-allocator.c \
										 trusted/lib/heap.c \


Enclave_Include_Paths := -Iinclude \
						-Itrusted \
						-I$(SGX_SDK)/include \
						-I$(SGX_SDK)/include/tlibc \
						-I$(SGX_SDK)/include/libcxx \
						-Itrusted/lib \
						-Itrusted/include \
						-I$(HOTCALL_BUNDLER_LIB_PATH)/lib \
						-I$(HOTCALL_BUNDLER_LIB_PATH)/trusted/static_trusted \
						-I$(HOTCALL_BUNDLER_INCLUDE_PATH)


Flags_Just_For_C := -Wno-implicit-function-declaration -std=c11
Common_C_Cpp_Flags := $(SGX_COMMON_CFLAGS) -nostdinc -fvisibility=hidden -fpie -fstack-protector $(Enclave_Include_Paths) -fno-builtin-printf -I.
Enclave_C_Flags := $(Flags_Just_For_C) $(Common_C_Cpp_Flags) $(LFLAGS)

Enclave_Link_Flags := $(SGX_COMMON_CFLAGS) -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L$(SGX_LIBRARY_PATH) \
	-Wl,--whole-archive -l$(Trts_Library_Name) -Wl,--no-whole-archive \
	-Wl,--start-group -lsgx_tstdc -lsgx_tcxx -l$(Crypto_Library_Name) -l$(Service_Library_Name) \
	-L$(HOTCALL_BUNDLER_LIB_PATH)/trusted -lhotcall_bundler_trusted -Wl,--end-group \
	-Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
	-Wl,--defsym,__ImageBase=0 \
	-Wl,--version-script=trusted/enclave.lds

Enclave_C_Objects := $(Enclave_C_Files:.c=.o)

ifeq ($(SGX_MODE), HW)
ifneq ($(SGX_DEBUG), 1)
ifneq ($(SGX_PRERELEASE), 1)
Build_Mode = HW_RELEASE
endif
endif
endif


.PHONY: all run

ifeq ($(Build_Mode), HW_RELEASE)
all: enclave.so
	@echo "Build enclave enclave.so [$(Build_Mode)|$(SGX_ARCH)] success!"
	@echo
	@echo "*********************************************************************************************************************************************************"
	@echo "PLEASE NOTE: In this mode, please sign the enclave.so first using Two Step Sign mechanism before you run the app to launch and access the enclave."
	@echo "*********************************************************************************************************************************************************"
	@echo
else
all: enclave.signed.so
endif

run: all
ifneq ($(Build_Mode), HW_RELEASE)
	@$(CURDIR)/app
	@echo "RUN  =>  app [$(SGX_MODE)|$(SGX_ARCH), OK]"
endif


######## enclave Objects ########

trusted/enclave_t.c: $(SGX_EDGER8R) ./trusted/enclave.edl
	@cd ./trusted && $(SGX_EDGER8R) --trusted ../trusted/enclave.edl --search-path ../trusted --search-path $(SGX_SDK)/include \
	--search-path $(HOTCALL_BUNDLER_LIB_PATH)/trusted/static_trusted
	@echo "GEN  =>  $@"

trusted/enclave_t.o: ./trusted/enclave_t.c
	@$(CC) $(Enclave_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

trusted/%.o: trusted/%.c
	@$(CC) $(Enclave_C_Flags) -c $< -o $@
	@echo "CC  <=  $<"

enclave.so: trusted/enclave_t.o $(Enclave_C_Objects)
	@$(CXX) $^ -o $@ $(Enclave_Link_Flags)
	@echo "LINK =>  $@"

enclave.signed.so: enclave.so
	@$(SGX_ENCLAVE_SIGNER) sign -key trusted/enclave_private.pem -enclave enclave.so -out $@ -config trusted/enclave.config.xml
	@echo "SIGN =>  $@"
clean:
	@rm -f enclave.* trusted/enclave_t.*  $(Enclave_C_Objects)
