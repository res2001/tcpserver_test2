.PHONY: all clean cleanobj

BASE_DIR = .
BUILD_DIR ?= $(BASE_DIR)/build

# release | debug
TYPEBUILD ?= release

TARGET_DIR := $(BUILD_DIR)/bin
SRC_DIR := $(BASE_DIR)/src
DEPS_DIR := $(BASE_DIR)/deps
OBJ_DIR := $(BUILD_DIR)/obj

TARGET := $(TARGET_DIR)/tcpserver_test2

SRC := $(wildcard $(SRC_DIR)/*.cpp)

OBJS := $(SRC:.cpp=.o)
OBJS := $(subst $(BASE_DIR)/,$(OBJ_DIR)/,$(OBJS))
DEPS := $(OBJS:.o=.d)

CFLAGS += -std=c++14
LDFLAGS += -lstdc++ -lpthread
#Нормальный уровень предупреждений
WARNNORMALOPTS := -Wall -Wextra -pedantic -Wno-missing-field-initializers -Wno-ignored-qualifiers\
                 -Werror=implicit-int -Werror=implicit-function-declaration -Werror=return-type\
                 -Wformat-security -Winit-self -Wstrict-aliasing -Wundef \
                 -Wcast-align -Wwrite-strings -Wlogical-op \
                 -Wno-format-zero-length -Wshadow
# -Wno-pedantic-ms-format -Waggregate-return
#повышенный уроень предупреждения компилятора
WARNEXTRAOPTS := -Wunsafe-loop-optimizations -Winline -Werror=strict-prototypes -Wfloat-equal

ifeq ($(TYPEBUILD), release)
  CFLAGS += -O2
else
  CFLAGS += -g -O0 -fno-inline -fno-omit-frame-pointer
endif

CFLAGS += $(WARNNORMALOPTS) $(WARNEXTRAOPTS) -I$(SRC_DIR)
  
DEPFLAGS = -MMD -MP -MF $(OBJ_DIR)/$*.d

define target_pre_cmd
  echo ""
  echo Make $@
  echo $^ 
  @mkdir -p $(dir $@)
endef

get-lib-name = $(subst lib,,$(notdir $(basename $1)))

all: $(TARGET)$(TARGET_EXT)

cleanobj:
	@$(RM) -f $(OBJS) $(DEPS) 

clean: cleanobj
	@$(RM) -f $(TARGET)$(TARGET_EXT)

$(TARGET)$(TARGET_EXT): $(OBJS)
	@$(target_pre_cmd)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(BASE_DIR)/%.cpp
$(OBJ_DIR)/%.o: $(BASE_DIR)/%.cpp $(OBJ_DIR)/%.d
	@$(target_pre_cmd)
	$(CC) $(DEPFLAGS) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.d: ;
.PRECIOUS: $(OBJ_DIR)/%.d

include $(DEPS)
