SOURCE_DIR = $(PWD)
INCLUDE_DIR = $(SOURCE_DIR)/include
LIBRARY_DIR = $(SOURCE_DIR)/library
BUILD_DIR = $(SOURCE_DIR)/build

#   sqlite3 - 3.7.14.1
SQLITE_DIR = $(SOURCE_DIR)/sqlite
SQLITE_SRC =                    \
    $(SQLITE_DIR)/sqlite3.c
SQLITE_INC =                    \
    $(SQLITE_DIR)/sqlite3.h
SQLITE_CFLAGS = -I$(SQLITE_DIR)
SQLITE_LDFLAGS = -lpthread -ldl

#   all
ALL_SRC =                       \
    $(SQLITE_SRC)               \
    libcommon.c                 \
    api_sql.c
    
ALL_INC =                       \
    $(SQLITE_INC)               \
    api_sql.h                   \
    sql.h                       \
    common.h                    \
    libcommon.h
    
ALL_CFLAGS =                    \
    $(SQLITE_CFLAGS)            \
    $(DATABASE_CFLAGS)          \
    -I./                        \
    -g -Wall

ALL_LDFLAGS =                   \
    -L$(LIBRARY_DIR)            \
    $(SQLITE_LDFLAGS)           \
    $(DATABASE_LDFLAGS)
    
ALL_OBJ = $(notdir $(patsubst %.c, %.o, $(ALL_SRC)))

$(CC) = gcc

all: libsql.a

  
libsql.a: $(addprefix $(BUILD_DIR)/, $(ALL_OBJ))
	$(AR) rcs $@ $^


$(addprefix $(BUILD_DIR)/, $(ALL_OBJ)): $(ALL_SRC)
	$(CC) $^ -c -I$(INCLUDE_DIR) $(ALL_CFLAGS)
	mkdir -p $(BUILD_DIR)
	mv *.o $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
    rm ./*.o
    rm libsql.a

debug_msg:
	@echo $(ALL_OBJ)