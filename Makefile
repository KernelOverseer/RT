# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: azarzor <azarzor@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2019/11/17 15:42:28 by abiri             #+#    #+#              #
#    Updated: 2020/03/04 12:23:10 by abiri            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

_lCYAN = \x1b[96m
_lYELLOW = \x1b[93m
_lGREEN = \x1b[92m
_lBLUE = \x1b[94m
_RED = \x1b[31m
_BOLD = \x1b[1m
_END = \x1b[0m
CC = gcc
NAME = rt
SLAVE_NAME = slave_rt

FILENAMES = basics_constructors.c\
			basics_intersection.c\
			basics_normals.c\
			cam_utils.c\
			colors.c\
			colors_calcul.c\
			colors_operations.c\
			colors_utils.c\
			cut_helper.c\
			destroy.c\
			dump_bitmap.c\
			filter.c\
			ft_atof.c\
			intersection_utils.c\
			key_managing.c\
			limited_constructors.c\
			loading_functions.c\
			ray_marching.c\
			noise.c\
			parser_main.c\
			parser_tools.c\
			parser_types.c\
			procedural_textures.c\
			quadrics_constructors.c\
			quadrics_intersection.c\
			quadrics_normals.c\
			reflection.c\
			refraction.c\
			shapes_cut.c\
			shapes_limit.c\
			string_parsers.c\
			uv_mapping_utils.c\
			world_constructors.c\
			fractals.c\
			user_interface.c\
			box_constructor.c\
			pill_constructor.c\
			parallelepiped_constructor.c\
			image_filtering.c\
			textures_constructors.c\
			stereoscopie.c\
			shadows.c\
			material.c\
			texture_maps.c\
			hit_texture_color.c\
			color_intersection.c\
			mlx_managing.c\
			bottom_buttons.c\
			draw_buttons.c\
			holo_cube.c\
			complex_constructors.c\
			xml_parser_tools.c \
			quartic_solver.c \
			cubic_solver.c\
			torus.c\
			cube.c\
			depth_of_field.c

HEADER_FILE = objects.h\
			  parser.h\
			  rtv1.h

SRC_DIR = ./src
OBJ_DIR = ./obj
INC_DIR = ./inc

MAIN_RENDER_SRC = headless_rendering.c\
				  main.c

SLAVE_RENDER_SRC = slave_rendering.c\
				   slave_main.c

OBJ = $(addprefix $(OBJ_DIR)/, $(FILENAMES:.c=.o))

MAIN_OBJ = $(addprefix $(OBJ_DIR)/, $(MAIN_RENDER_SRC:.c=.o))

SLAVE_OBJ = $(addprefix $(OBJ_DIR)/, $(SLAVE_RENDER_SRC:.c=.o))

HEADER_FILES = $(addprefix $(INC_DIR)/, $(HEADER_FILE))

FLAGS = -g -Wall -Werror -Wextra

FTMATHS = ./libs/ft_maths

LIBFTMATHS = $(FTMATHS)/libftmaths.a

LIBFTDIR = ./libs/libft

LIBFT = $(LIBFTDIR)/libft.a

LIBTTSLISTDIR = ./libs/ttslist

LIBTTSLIST = $(LIBTTSLISTDIR)/libttslist.a

INCLUDES = -I ./inc -I $(LIBFTDIR) -I ./libs/ft_maths/inc -I $(LIBTTSLISTDIR)/includes -I ./libs/MinilibX/elcapitan

# detecting the os and linking with the good minilibx

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
	LIBRARIES = -L $(LIBFTDIR) -lft -L ./minilibx -lmlx -lX11 -lXext -L $(FTMATHS) -lftmaths -lm -L $(LIBTTSLISTDIR) -lttslist -pthread
else
	LIBRARIES = -L $(LIBFTDIR) -lft -L ./libs/MinilibX -lmlx -framework OpenGL -framework AppKit -L $(FTMATHS) -lftmaths -L $(LIBTTSLISTDIR) -lttslist
endif

DELAY = 0

all : $(NAME)
$(NAME): $(MAIN_OBJ) $(OBJ) $(LIBFT) $(LIBFTMATHS) $(LIBTTSLIST)
	@echo "$(CC) $(_lYELLOW)$(FLAGS)$(_END) $(_lCYAN)$(OBJ)$(_END)\n$(_lGREEN)$(LIBRARIES)$(_END) -I$(_RED)$(INC_DIR)$(_END)$(_RED)$(INCLUDES)$(_END) -o $(_lBLUE)$(NAME)$(_lEND)$(_RED)\n"
	@$(CC) $(FLAGS) $(MAIN_OBJ) $(OBJ) $(LIBRARIES) $(INCLUDES) -o $(NAME)

$(SLAVE_NAME): $(SLAVE_OBJ) $(OBJ) $(LIBFT) $(LIBFTMATHS) $(LIBTTSLIST)
	@echo "$(CC) $(_lYELLOW)$(FLAGS)$(_END) $(_lCYAN)$(OBJ)$(_END)\n$(_lGREEN)$(LIBRARIES)$(_END) -I$(_RED)$(INC_DIR)$(_END)$(_RED)$(INCLUDES)$(_END) -o $(_lBLUE)$(NAME)$(_lEND)$(_RED)\n"
	@$(CC) $(FLAGS) $(SLAVE_OBJ) $(OBJ) $(LIBRARIES) $(INCLUDES) -o $(SLAVE_NAME)

$(OBJ): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c $(HEADER_FILES) | $(OBJ_DIR)
	$(CC) $(FLAGS) -c $< -o $@ -I $(INC_DIR) $(INCLUDES)

$(SLAVE_OBJ): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c $(HEADER_FILES) | $(OBJ_DIR)
	$(CC) $(FLAGS) -c $< -o $@ -I $(INC_DIR) $(INCLUDES)

$(MAIN_OBJ): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c $(HEADER_FILES) | $(OBJ_DIR)
	$(CC) $(FLAGS) -c $< -o $@ -I $(INC_DIR) $(INCLUDES)

$(OBJ_DIR):
	@mkdir $(OBJ_DIR)

$(LIBFTMATHS):
	@make -C $(FTMATHS)

$(LIBFT):
	@make -C $(LIBFTDIR)

$(LIBTTSLIST):
	@make -C $(LIBTTSLISTDIR)

clean:
	@echo "\n$(_lCYAN)Makefile :$(_END) will delete $(_RED)$(OBJ) $(VIS_OBJ)$(_END)"
	@echo "starting in $(DELAY) sec, $(_RED)press Ctrl-c to abort$(_END)"
	@sleep $(DELAY)
	@make -C $(LIBFTDIR) clean
	@make -C $(FTMATHS) clean
	@make -C $(LIBTTSLISTDIR) clean
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "\n$(_lCYAN)Makefile :$(_END) will delete $(_RED)$(NAME)$(_END)"
	@echo "starting in $(DELAY) sec, $(_RED)press Ctrl-c to abort$(_END)"
	@sleep $(DELAY)
	@make -C $(LIBFTDIR) fclean
	@make -C $(FTMATHS) fclean
	@make -C $(LIBTTSLISTDIR) fclean
	@rm -f $(NAME)
	@rm -f $(SLAVE_NAME)
re: fclean all
