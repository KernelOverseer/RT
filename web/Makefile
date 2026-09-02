# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    web/Makefile                                       :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: KernelOverseer <marvin@42.fr>               +#+  +:+       +#+        #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/09/02 by KernelOverseer        #+#    #+#              #
#    Updated: 2026/09/02 by KernelOverseer       ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# WebAssembly build of the RT engine (Emscripten).
#
#   make -C web          -> web/dist/rt.js + rt.wasm (+ copied scenes)
#   make -C web test     -> node smoke test (renders every bundled scene)
#
# Requires emcc in PATH, e.g: source ~/emsdk/emsdk_env.sh

CC = emcc
NAME = dist/rt.js

ROOT = ..
SRC_DIR = $(ROOT)/src
INC_DIR = $(ROOT)/inc

# engine sources: everything except the native window/main entry points,
# the pthread scheduler (replaced by the worker pool) and the mlx button UI
EXCLUDE = main.c headless_rendering.c slave_rendering.c slave_main.c \
		  draw_buttons.c user_interface.c bottom_buttons.c key_managing.c

SRC = $(filter-out $(addprefix $(SRC_DIR)/,$(EXCLUDE)),\
	  $(wildcard $(SRC_DIR)/*.c)) src/rt_web.c

# mirrors libs/libft/Makefile, which does not compile ft_unescape_args.c
# (it would duplicate ft_strunescape already defined in ft_strunescape.c)
LIBFT_SRC = $(filter-out $(ROOT)/libs/libft/ft_unescape_args.c,\
		   $(wildcard $(ROOT)/libs/libft/*.c))
FTMATHS_SRC = $(wildcard $(ROOT)/libs/ft_maths/src/*.c)
TTSLIST_SRC = $(wildcard $(ROOT)/libs/ttslist/srcs/*.c)

INCLUDES = -I inc -I $(INC_DIR) -I $(ROOT)/libs/libft \
		   -I $(ROOT)/libs/ft_maths/inc -I $(ROOT)/libs/ttslist/includes

CFLAGS = -O3 -Wall -msimd128 -Du_int32_t=uint32_t \
		 -sMODULARIZE=1 -sEXPORT_NAME=RTModule \
		 -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=64MB \
		 -sEXPORTED_RUNTIME_METHODS=FS,HEAPU8,HEAPF64,allocateUTF8,UTF8ToString \
		 -sEXPORTED_FUNCTIONS=_malloc,_free \
		 --no-entry

SCENES = $(notdir $(wildcard $(ROOT)/rt-scenes/*.xml))

all: $(NAME) stage

$(NAME): $(SRC) $(LIBFT_SRC) $(FTMATHS_SRC) $(TTSLIST_SRC)
	@mkdir -p dist
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) $(LIBFT_SRC) $(FTMATHS_SRC) \
		$(TTSLIST_SRC) -o $(NAME)

VERSION := $(shell git rev-parse --short HEAD 2>/dev/null || date +%s)

stage:
	@mkdir -p dist/scenes
	@for scene in $(SCENES); do \
		cp "$(ROOT)/rt-scenes/$$scene" dist/scenes/ ; \
	done
	@if [ -f main.js ]; then cp main.js worker.js scenes.js index.html style.css dist/ ; fi
	@sed -i "s/BUILD_VERSION = 'dev'/BUILD_VERSION = '$(VERSION)'/" dist/scenes.js
	@sed -i -E 's/(src|href)="([^"?]+\.js)"/\1="\2?v=$(VERSION)"/; s/(src|href)="([^"?]+\.css)"/\1="\2?v=$(VERSION)"/' dist/index.html
	@echo "web build ready in web/dist/ (version $(VERSION))"

test: all
	node smoke_test.js

clean:
	rm -rf dist

fclean: clean

re: fclean all

.PHONY: all stage test clean fclean re
