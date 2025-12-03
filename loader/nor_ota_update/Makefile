
# make 编译并下载
# make VERBOSE=1 显示编译详细过程
# make clean 清除编译临时文件
#
# 注意： Linux 下编译方式：
#     1. 从 http://pkgman.jieliapp.com/doc/all 处找到下载链接
#     2. 下载后，解压到 /opt/jieli 目录下，保证
#       /opt/jieli/common/bin/clang 存在（注意目录层次）
#     3. 确认 ulimit -n 的结果足够大（建议大于8096），否则链接可能会因为打开文件太多而失败
#       可以通过 ulimit -n 8096 来设置一个较大的值
#

# 工具路径设置
ifeq ($(OS), Windows_NT)
# Windows 下工具链位置
TOOL_DIR := C:/JL/pi32/bin
CC    := clang.exe
CXX   := clang.exe
LD    := pi32v2-lto-wrapper.exe
AR    := llvm-ar.exe
MKDIR := mkdir_win -p
RM    := rm -rf

SYS_LIB_DIR := C:/JL/pi32/pi32v2-lib/r3
SYS_INC_DIR := C:/JL/pi32/pi32v2-include
EXT_CFLAGS  := # Windows 下不需要 -D__SHELL__
export PATH:=$(TOOL_DIR);$(PATH)

## 后处理脚本
POST_SCRIPT     := cpu/br35/output/download.bat
RUN_POST_SCRIPT := cpu\br35\output\download.bat
else
# Linux 下工具链位置
TOOL_DIR := /opt/jieli/pi32v2/bin
CC    := clang
CXX   := clang
LD    := lto-wrapper
AR    := lto-ar
MKDIR := mkdir -p
RM    := rm -rf
export OBJDUMP := $(TOOL_DIR)/objdump
export OBJCOPY := $(TOOL_DIR)/objcopy
export OBJSIZEDUMP := $(TOOL_DIR)/objsizedump

SYS_LIB_DIR := $(TOOL_DIR)/../lib/r3
SYS_INC_DIR := $(TOOL_DIR)/../include
EXT_CFLAGS  := -D__SHELL__ # Linux 下需要这个保证正确处理 download.c
export PATH:=$(TOOL_DIR):$(PATH)

## 后处理脚本
POST_SCRIPT     := cpu/br35/output/download.sh
RUN_POST_SCRIPT := bash $(POST_SCRIPT)
endif

CC  := $(TOOL_DIR)/$(CC)
CXX := $(TOOL_DIR)/$(CXX)
LD  := $(TOOL_DIR)/$(LD)
AR  := $(TOOL_DIR)/$(AR)
# 输出文件设置
OUT_ELF   := cpu/br35/output/ota_loader.elf
OBJ_FILE  := $(OUT_ELF).objs.txt
# 编译路径设置
BUILD_DIR := objs

# 编译参数设置
CFLAGS := \
	-target pi32v2 \
	-mcpu=r3 \
	-integrated-as \
	-flto \
	-integrated-as \
	-fallow-pointer-null \
	-fno-common \
	-flto \
	-g \
	-Oz \
	-mllvm -pi32v2-large-program=true \
	-Wno-format \
	-Wno-unused-value \
	-Wno-unused-function \
	-Wno-unused-comparison \
	-Wno-parentheses \
	-Wno-tautological-compare \
	-Wno-incompatible-library-redeclaration \
	-Wno-invalid-noreturn \
	-Wno-visibility \
	-Wno-integer-overflow \
	-fms-extensions \
	-Wno-shift-negative-value \


# C++额外的编译参数
CXXFLAGS :=


# 宏定义
DEFINES := \
	-D__GCC_PI32V2__ \
	-DCONFIG_CPU_BR35 \
	-DOTA_LOADER_TYPE=nor_ota \
	-D__APP_IS_OTA \
	-D__DEBUG \
	-D__ST_DEBUG \
	-D__NOR_DEBUG \
	-D__JLFS_DEBUG \
	-D__UP_DEBUG \
	-DCONFIG_BTCTRLE_V2_ENABLE \
	-DEX_FLASH_UPDATE_SUPPORT_EN=1 \
	-D__USE_MASK_API__ \
	-D__MASK_LZ4 \
	-DCONFIG_NEW_BREDR_ENABLE \
	-DCONFIG_NEW_MODEM_ENABLE \
	-D__USE_MASK_INI_DEC \
	-D__FLASH_SUPPORT_4BIT_MODE \
	-D__FLASH_SUPPORT_CONTINUE_READ_MODE \
	-D__SUPPORT_OUTSIDE_FLASH \
	-D__UPDATE_RUN_RAM \


DEFINES += $(EXT_CFLAGS) # 额外的一些定义

# 头文件搜索路径
INCLUDES := \
	-Iapp \
	-Iapp/inc \
	-Iapp/inc/cpu/br35 \
	-Icpu \
	-Icpu/br35 \
	-Iinclude_lib \
	-Iinclude_lib/common \
	-Iinclude_lib/driver \
	-Iinclude_lib/driver/common \
	-Iinclude_lib/driver/cpu \
	-Iinclude_lib/driver/cpu/br35 \
	-Iinclude_lib/driver/cpu/br35/power \
	-Iinclude_lib/driver/device \
	-Iinclude_lib/driver/device/dev_mg \
	-Iinclude_lib/driver/device/eeprom \
	-Iinclude_lib/driver/device/norflash \
	-Iinclude_lib/driver/device/sdmmc \
	-Iinclude_lib/driver/device/usb \
	-Iinclude_lib/driver/jlfs \
	-Iinclude_lib/driver/jlup \
	-Iinclude_lib/system \
	-Iinclude_lib/system/common \
	-Iinclude_lib/system/cpu \
	-Iinclude_lib/system/cpu/br35 \
	-Iinclude_lib/system/cpu/br35/asm \
	-Iinclude_lib/system/cpu/br35/asm/power \
	-Iinclude_lib/system/fs \
	-Iinclude_lib/system/os \
	-Iinclude_lib/system/generic \
	-Iinclude_lib/system/fs/lite_file_sys \
	-Iinclude_lib/update \
	-Iinclude_lib/jl_rcsp \
	-Iinclude_lib/lib_mutual_loader \
	-Iinterface/btctrler \
	-Iinterface/btctrler/port/br35 \
	-I$(SYS_INC_DIR) \


# 需要编译的 .c 文件
c_SRC_FILES := \
	app/src/common/cbuf.c \
	app/src/common/lbuf.c \
	app/src/common/msg.c \
	app/src/common/sys_timer.c \
	app/src/common/version.c \
	app/src/lib_btctrler_config.c \
	app/src/loader_main.c \
	app/src/log_config.c \
	app/src/update/ex_flash_update.c \
	app/src/update/update_main.c \


# 需要编译的 .S 文件
S_SRC_FILES :=


# 需要编译的 .s 文件
s_SRC_FILES :=


# 需要编译的 .cpp 文件
cpp_SRC_FILES :=


# 需要编译的 .cc 文件
cc_SRC_FILES :=


# 需要编译的 .cxx 文件
cxx_SRC_FILES :=


# 链接参数
LFLAGS := \
	--gc-sections \
	--plugin-opt=-dont-used-symbol-list=malloc,free,sprintf,printf,puts,putchar,perror,vprintf,printi,fopen,fread,fwrite,fseek,strlen,memset,memcpy,strcpy,memmove,strcmp \
	-M=cpu/br35/output/map.txt \
	-Tcpu/br35/output/ram.ld \
	--plugin-opt=-inline-threshold=5 \
	--plugin-opt=-enable-ipra=true \
	--plugin-opt=-pi32v2-always-use-itblock \
	--plugin-opt=-pi32v2-merge-max-offset=64 \
	--plugin-opt=-global-merge-on-const \
	--plugin-opt=-pi32v2-enable-simd=true \
	--plugin-opt=-pi32v2-enable-rep-memop=true \
	--plugin-opt=mcpu=r3 \
	--plugin-opt=save-temps \
	--plugin-opt=-pi32v2-large-program=true \
	--start-group \
	--whole-archive \
	include_lib/liba/br35/ota_loader_driver.a \
	include_lib/liba/br35/system.a \
	include_lib/liba/br35/update.a \
	--no-whole-archive \
	--end-group \
	--plugin-opt=mcpu=r3 \
	--plugin-opt=-mattr=+fprev1 \


LIBPATHS := \
	-L$(SYS_LIB_DIR) \


LIBS := \
	$(SYS_LIB_DIR)/libm.a \
	$(SYS_LIB_DIR)/libc.a \
	$(SYS_LIB_DIR)/libm.a \
	$(SYS_LIB_DIR)/libcompiler-rt.a \



c_OBJS    := $(c_SRC_FILES:%.c=%.c.o)
S_OBJS    := $(S_SRC_FILES:%.S=%.S.o)
s_OBJS    := $(s_SRC_FILES:%.s=%.s.o)
cpp_OBJS  := $(cpp_SRC_FILES:%.cpp=%.cpp.o)
cxx_OBJS  := $(cxx_SRC_FILES:%.cxx=%.cxx.o)
cc_OBJS   := $(cc_SRC_FILES:%.cc=%.cc.o)

OBJS      := $(c_OBJS) $(S_OBJS) $(s_OBJS) $(cpp_OBJS) $(cxx_OBJS) $(cc_OBJS)
DEP_FILES := $(OBJS:%.o=%.d)


OBJS      := $(addprefix $(BUILD_DIR)/, $(OBJS))
DEP_FILES := $(addprefix $(BUILD_DIR)/, $(DEP_FILES))


VERBOSE ?= 0
ifeq ($(VERBOSE), 1)
QUITE :=
else
QUITE := @
endif

# 一些旧的 make 不支持 file 函数，需要 make 的时候指定 LINK_AT=0 make
LINK_AT ?= 1

# 表示下面的不是一个文件的名字，无论是否存在 all, clean, pre_build 这样的文件
# 还是要执行命令
# see: https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean pre_build

# 不要使用 make 预设置的规则
# see: https://www.gnu.org/software/make/manual/html_node/Suffix-Rules.html
.SUFFIXES:

all: pre_build $(OUT_ELF)
	$(info +POST-BUILD)
	$(QUITE) $(RUN_POST_SCRIPT) ota_loader

pre_build:
	$(info +PRE-BUILD)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P cpu/br35/output/ram_ld.c -o cpu/br35/output/ram.ld

clean:
	$(QUITE) $(RM) $(OUT_ELF)
	$(QUITE) $(RM) $(BUILD_DIR)



ifeq ($(LINK_AT), 1)
$(OUT_ELF): $(OBJS)
	$(info +LINK $@)
	$(shell $(MKDIR) $(@D))
	$(file >$(OBJ_FILE), $(OBJS))
	$(QUITE) $(LD) -o $(OUT_ELF) @$(OBJ_FILE) $(LFLAGS) $(LIBPATHS) $(LIBS)
else
$(OUT_ELF): $(OBJS)
	$(info +LINK $@)
	$(shell $(MKDIR) $(@D))
	$(QUITE) $(LD) -o $(OUT_ELF) $(OBJS) $(LFLAGS) $(LIBPATHS) $(LIBS)
endif


$(BUILD_DIR)/%.c.o : %.c
	$(info +CC $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.S.o : %.S
	$(info +AS $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.s.o : %.s
	$(info +AS $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.cpp.o : %.cpp
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.cxx.o : %.cxx
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.cc.o : %.cc
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

-include $(DEP_FILES)
