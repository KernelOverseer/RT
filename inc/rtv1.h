/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rtv1.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azarzor <azarzor@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/17 16:13:19 by abiri             #+#    #+#             */
/*   Updated: 2024/06/16 20:01:04 by abiri            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RTV1_H
# define RTV1_H
# include "libft.h"
# include "ttslist.h"
# include "parser.h"
# include "objects.h"
# include <mlx.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <time.h>
# include <pthread.h>
# include <stdio.h>
# include <limits.h>

/*
** raytracing macros
*/

# define SPOT_LIGHT 1
# define NORMAL_LIGHT 0
# define MAX_D 1e30
# define MIN_D 1e-6

/*
** math macros
*/

# define FT_SQR(X) ((X) * (X))
# define FT_RAD(X) (((X) * M_PI) / 180)
# define FT_INTERPOLATION(A, B, C) (B + A * (C - B))
# define FT_FADE(N) (N * N * N * (N * (N * 6 - 15) + 10))
# define FT_QUAR(X) ((X) * (X) * (X) * (X))
# define FT_CUB(X) ((X) * (X) * (X))
# define EQN_EPS 1e-9
# define IS_ZERO(x) ((x) > -EQN_EPS && (x) < EQN_EPS)
# define POW1(x) (pow((double)(x), 1.0 / 3.0))
# define POW2(x) (pow((double)-(x), 1.0 / 3.0))
# define CBRT(x) ((x) > 0.0 ? POW1(x) : ((x) < 0.0 ? -POW2(x) : 0.0))

/*
** code comprehension macros
*/

# define WORLD_FACT refraction_fractors[0]
# define OBJECT_FACT refraction_fractors[1]
# define DOT_N_R refraction_fractors[2]
# define SWITCH_FACT refraction_fractors[3]
# define INTERNAL_REFRACTION refraction_fractors[4]
# define NORMAL_REF refraction_vectors[0]
# define SCALED_RAY refraction_vectors[1]
# define REFRACTED_RAY refraction_vectors[2]
# define RESULT refraction_vectors[3]
# define CAM_FOREWORD cam_vects[0]
# define CAM_RIGHT cam_vects[1]
# define CAM_UP cam_vects[2]
# define RATIO cam_utils[0]
# define HALF_HEIGHT cam_utils[1]
# define HALF_WIDTH cam_utils[2]
# define LIGHT_VECTOR light_vect[0]
# define REFLECTED_LIGHT_VECTOR  light_vect[1]
# define CENTER cube_utils[0]
# define LENGTHS cube_utils[1]
# define NEW_COLOR color[1]
# define OLD_COLOR color[0]
# define DIFFUSE shader[0]
# define SPECULAR shader[1]
# define A_ sides[0]
# define AA sides[1]
# define AB sides[2]
# define B_ sides[3]
# define BA sides[4]
# define BB sides[5]
# define RAY_DIR cam->ray_direction

/*
** bmp save macos
*/

# define BMP_MAGIC_HEADER 19778
# define BMP_INFO_HEADER_SIZE 40
# define BMP_PLANES_NUMBER 1
# define BMP_BITS_PER_PIXEL 32
# define BMP_ALL_HEADERS_SIZE 54

/*
**	event macros
*/

# define EXIT 53
# define FOREWORD 12
# define BACKWARD 14
# define UP 13
# define LEFT 0
# define RIGHT 2
# define DOWN 1
# define KEY_R 15
# define KEY_T 17
# define SAVE 36
# define PIXEL_SIZE 5
# define NUM_THREAD 64

/*
**	buttons macros
*/

# define BUTTON_ACTIVE_COLOR (t_color){0.4, 0.7, 0.4}
# define BUTTON_INACTIVE_COLOR (t_color){0.7, 0.7, 0.7}
# define BUTTON_WIDTH 120
# define BUTTON_HEIGHT 20
# define BUTTON_TEXT_COLOR 0x0

typedef struct			s_img
{
	void				*mlx_win;
	void				*mlx_ptr;
	void				*mlx_img;
	void				*img_ptr;
	int					*data;
	int					size_l;
	int					bpp;
	int					endian;
	int					width;
	int					height;
}						t_img;

typedef struct			s_mlx
{
	void				*mlx_ptr;
	void				*win;
	t_img				img;
}						t_mlx;

typedef struct			s_light
{
	t_vector			center;
	t_vector			light_vect;
	t_vector			reflected_light_vect;
	t_vector			light_direction;
	t_color				color;
	double				intensity;
	double				radius;
	int					light_type;
	t_sphere			light_shape;
}						t_light;

typedef struct			s_light_list
{
	t_light				light;
	struct s_light_list	*next;
}						t_light_list;

typedef struct			s_intersection
{
	double				soluces[2];
	t_vector			position;
	t_vector			normal;
	t_color				color;
	double				reflection;
	double				refraction;
	double				transparency;
	t_object			*object;
	t_coor				uv;
}						t_intersection;

typedef struct			s_cam
{
	t_vector			position;
	t_vector			look_at;
	t_vector			ray_direction;
	t_vector			ray_origin;
	t_intersection		hit;
	t_vector			translation;
	t_vector			bottom_left;
	t_vector			w_scalar;
	t_vector			h_scalar;
	t_vector			right;
	t_vector			up;
	double				fov;
}						t_cam;

typedef	struct			s_actions
{
	char				mouvement;
}						t_actions;

typedef struct			s_scene
{
	double				ambiant;
	int					width;
	int					height;
	int					filter;
	int					effect;
	int					aa;
	int					reflection_depth;
	int					refraction_depth;
	int					light_samples;
	int					dof;
	int					dof_focus;
}						t_scene;

typedef struct			s_options
{
	unsigned int		anti_aliasing;
	unsigned int		ambiant;
	unsigned int		diffuse;
	unsigned int		specular;
	unsigned int		refraction;
	unsigned int		reflection;
	unsigned int		soft_shadows;
	unsigned int		depth_of_field;
}						t_options;

typedef	struct			s_rtv
{
	t_cam				cam;
	t_object_list		*objects;
	t_object_list		*last_object;
	t_light_list		*lights;
	t_light_list		*last_light;
	t_list_head			textures;
	t_list_head			buttons;
	t_actions			actions;
	t_options			options;
	t_mlx				mlx;
	double				min;
	double				row;
	double				column;
	double				min_w;
	double				max_w;
	double				min_h;
	double				max_h;
	t_scene				scene;
	int					effects;
	int					render_offset;
	int					render_y_offset;
	int					anti_aliasing;
	int					pixel_size;
}						t_rtv;

typedef struct			s_cubic
{
	double				a;
	double				b;
	double				c;
	double				d;
	double				sq_a;
	double				p;
	double				q;
	double				cb_p;
}						t_cubic;

typedef struct			s_quartic
{
	double				a;
	double				b;
	double				c;
	double				d;
	double				z;
	double				u;
	double				v;
	double				sub;
	double				sq_a;
	double				p;
	double				q;
	double				r;
}						t_quartic;

typedef int				t_xml_element(t_xml_tag *tag, t_rtv *env);

typedef int				t_button_handler(void *element, int status, t_rtv *env);

typedef	struct			s_xml_element_parse
{
	t_xml_element		*function;
	char				*type_name;
}						t_xml_element_parse;

typedef struct			s_button
{
	char				*text;
	int					status;
	t_button_handler	*handler;
	void				*arg;
	t_coor				position;
}						t_button;

/*
**  functions to add shapes
*/
int						ft_add_sphere(t_xml_tag *tag, t_rtv *env);
int						ft_add_cylinder(t_xml_tag *tag, t_rtv *env);
int						ft_add_plane(t_xml_tag *tag, t_rtv *env);
int						ft_add_disk(t_xml_tag *tag, t_rtv *env);
int						ft_add_cone(t_xml_tag *tag, t_rtv *env);
int						ft_add_demi_sphere(t_xml_tag *tag, t_rtv *env);
int						ft_add_ellipsoid(t_xml_tag *tag, t_rtv *env);
int						ft_add_hyperboloid(t_xml_tag *tag, t_rtv *env);
int						ft_add_paraboloid(t_xml_tag *tag, t_rtv *env);
int						ft_add_triangle(t_xml_tag *tag, t_rtv *env);
int						ft_add_fractal(t_xml_tag *tag, t_rtv *env);
int						ft_add_pill(t_xml_tag *tag, t_rtv *env);
int						ft_add_box(t_xml_tag *tag, t_rtv *env);
int						ft_add_parallelepiped(t_xml_tag *tag, t_rtv *env);
int						ft_add_holo_cube(t_xml_tag *tag, t_rtv *env);
int						ft_add_torus(t_xml_tag *tag, t_rtv *env);
int						ft_add_cube(t_xml_tag *tag, t_rtv *env);
/*
**  shapes intersections functions
*/
double					ft_sphere_intersection(t_cam *cam,
		t_sphere *sphere, double min);
double					ft_plane_intersection(t_cam *cam,
		t_plane *plane, double min);
double					ft_cylinder_intersection(t_cam *cam,
		t_cylinder *cylinder, double min);
double					ft_cone_intersection(t_cam *cam,
		t_cone *cone, double min);
double					ft_ellipsoid_intersection(t_cam *cam,
		t_ellipsoid *ellipsoid, double min);
double					ft_hyperboloid_intersection(t_cam *cam,
		t_hyperboloid *hyperboloid, double min);
double					ft_paraboloid_intersection(t_cam *cam,
		t_paraboloid *paraboloid, double min);
double					ft_triangle_intersection(t_cam *cam,
		t_triangle *triangle, double min);
double					ft_holo_cube_intersection(t_cam *cam,
		t_holo_cube *holo_cube, double min);
double					ft_torus_intersection(t_cam *cam,
		t_torus *torus, double min);
double					ft_cube_intersection(t_cam *cam,
		t_cube *cube, double min);
double					trace(t_vector from, t_vector direction,
		t_fractal *fractal, int *status);
double					ft_mandelbrot_distance_estimator(t_vector pos,
		int *iter, t_fractal fractal);
/*
**  shapes normals calculations functions
*/
void					ft_sphere_normal(t_cam *cam,
		t_sphere *sphere, double distance);
void					ft_cylinder_normal(t_cam *cam,
		t_cylinder *cylinder, double distance);
void					ft_cone_normal(t_cam *cam,
		t_cone *cone, double distance);
void					ft_ellipsoid_normal(t_cam *cam,
		t_ellipsoid *ellipsoid, double distance);
void					ft_hyperboloid_normal(t_cam *cam,
		t_hyperboloid *hyperboloid, double distance);
void					ft_paraboloid_normal(t_cam *cam,
		t_paraboloid *paraboloid, double distance);
void					ft_plane_normal(t_cam *cam,
		t_plane *plane, double i);
void					ft_holo_cube_normal(t_cam *cam,
		t_holo_cube *holo_cube);
int						ft_axis_limit(t_vector intersection,
		t_limit limits);
void					ft_torus_normal(t_cam *cam,
		t_torus *torus);
void					ft_cube_normal(t_cam *cam,
		t_cube *box);
/*
**  limited objects calculations functions
*/
void					ft_define_limits(t_xml_tag *tag,
	t_limit *limit, int *status);
void					ft_sphere_cut(t_rtv *env,
	t_xml_tag *tag, t_object *object, int *status);
void					ft_cylinder_cut(t_rtv *env,
	t_xml_tag *tag, t_object *object, int *status);
void					ft_cone_cut(t_rtv *env,
	t_xml_tag *tag, t_object *object, int *status);
void					ft_demi_sphere_cut(t_rtv *env,
	t_xml_tag *tag, t_object *object, int *status);
void					ft_sides_handle(t_xml_tag *tag,
	t_object *object, int *status);
double					ft_sphere_limit(t_sphere sphere, t_cam cam);
double					ft_cone_limit(t_cone cone, t_cam cam);
double					ft_cylinder_limit(t_cylinder cylinder, t_cam cam);
double					ft_plane_limit(t_plane plane, t_cam cam);
/*
**	clipping functions
*/
double					ft_clip_min(int min, double value);
double					ft_clip_max(int max, double value);
double					ft_clip_min_max(int min, int max, double value);
/*
**	reflection, reflaction and shaders
*/
t_color					ft_specular(t_light light, t_vector normal,
	t_vector reflected_light_vect);
t_color					ft_cartoon_specular(t_light light, t_vector normal,
	t_vector reflected_light_vect);
t_color					ft_diffuse(t_light light, t_vector light_vect,
	t_vector normal, t_color color);
t_color					ft_cartoon_diffuse(t_light light, t_vector light_vect,
	t_vector normal, t_color color);
t_color					ft_reflect_ray(t_rtv rtv, int depth);
t_color					ft_get_node_color(t_rtv rtv, int depth);
t_color					ft_refract_ray(t_rtv rtv, int depth);
t_color					ft_mix_colors(t_rtv *rtv,
	t_vector normal, t_color color);
void					ft_add_material(t_xml_tag *tag,
	t_object *object, int *status, t_rtv *env);
/*
** color operations
*/
t_color					ft_add_colors(t_color first, t_color second);
t_color					ft_scale_colors(t_color first, double scalar);
int						ft_diff_color(t_color c1, t_color c2);
int						ft_rgb_to_int(t_color color);
t_color					ft_int_to_rgb(int color);
t_color					ft_merge_color(t_color first, t_color second);
/*
**	filter and effects
*/
t_color					ft_select_filter(t_rtv rtv,
	t_object object, t_color color);
t_color					ft_assign_color(double r, double g, double b);
void					ft_filtring_select(t_rtv *rtv);
/*
**	parsing related functions
*/
t_color					ft_parse_color(char *string, int *status);
t_vector				ft_parse_vector(char *string, int *status);
t_coor					ft_parse_coor(char *string, int *status);
double					ft_atof(char *string, int *size);
int						ft_int_len(char *string);
t_color					ft_parse_hex(char *string, int *status);
t_xml_element			*ft_get_tag_parsing_function(char *name);
int						ft_load_camera(t_xml_tag *tag, t_rtv *env);
int						ft_load_scene(t_xml_tag *tag, t_rtv *env);
int						ft_object_push(t_rtv *env, t_object object,
		int type);
/*
**	OTHER FUNCTION
*/
double					ft_choose_intersection(t_object_list *object_node,
	t_rtv *rtv, double *min);
int						ft_light_push(t_rtv *env, t_light light);
int						ft_load_shapes(t_xml_data *data, t_rtv *env);
int						ft_key_stroke(int key, t_rtv *rtv);
void					ft_create_ray(t_rtv *rtv, int sample);
void					ft_map_coordinates(t_rtv *rtv);
int						ft_shoot_stero(t_rtv *rtv);
int						ft_ray_shooter(t_rtv *rtv);
void					ft_update_offset(t_rtv *rtv);
void					ft_intersection_position(t_cam *cam,
	double first_intersection);
void					ft_put_pixel(t_rtv *rtv, int color);
void					ft_init_win(t_rtv *rtv);
t_vector				ft_reflected_light_ray(t_vector light_vect,
	t_vector normal);
int						ft_add_light(t_xml_tag *tag, t_rtv *env);
size_t					ft_escape_whitespaces(char *str);
int						ft_antialiasing(t_rtv *rtv,
	t_vector normal, t_color color);
t_plane					ft_define_plane(t_vector center, t_vector normal,
	t_object *object, double radius);
int						ft_check_min_distance(double *x1,
	double x2, double min);
t_vector				ft_get_refracted_ray(t_rtv rtv);
int						ft_headless_raytracer(t_rtv	*rtv, char *filename);
void					ft_init_rendrering(t_rtv *rtv);
void					ft_init_cam(t_cam *cam, t_rtv rtv);
void					ft_depth_of_field(t_rtv *rtv);
void					ft_toogle_dof(t_list_head *buttons, t_rtv *env);
void					ft_change_lookat(int mouse_button,
			int x, int y, t_rtv *env);
double					ft_smallest_double(double *tab, int size);
double					ft_min_sol(double s[4], double t, int res);
void					ft_specific_ray_shoot(t_rtv *rtv);

/*
**	bmp image saving functions
*/

int						ft_dump_bitmap(t_img *image);
int						ft_save_bitmap(t_img *image, char *filename);

/*
** get color for the ray shooted
*/
double					ft_choose_intersection(t_object_list *object_node,
					t_rtv *rtv, double *min);
t_color					ft_get_node_color(t_rtv rtv, int depth);
void					ft_color_best_node(t_rtv *rtv, t_color rgb);
void					ft_color_best_node_dof(t_rtv *rtv, t_color rgb);
/*
**	texture related functions
*/
t_color					ft_get_texture_color(t_texture *texture,
	t_coor uv, t_color original, int mode);
int						ft_get_texture_mapping_type(t_xml_tag *tag);
t_texture				*ft_load_texture(char *filename, t_rtv *env,
	t_texture **texture, t_procedural_texture_function **function);
t_texture				*ft_load_image(char *filename, t_rtv *rtv);
t_texture				*ft_get_texture(char *filename, t_rtv *env);\
/*
**	Interface functions
*/
void					ft_load_interface(t_list_head *buttons, t_rtv *env);
void					ft_draw_buttons(t_rtv *env);
int						ft_click_buttons(int mouse_button,
	int x, int y, t_rtv *env);
void					ft_bottom_buttons(t_list_head *buttons, t_rtv *env);
t_button				*ft_new_button(char *text, t_button_handler *handler,
	void *arg, t_coor position);
void					ft_draw_buttons(t_rtv *env);
int						ft_button_toggle_boolean(void *arg,
	int status, t_rtv *env);
/*
** get shapes informations
*/
void					ft_get_plane_axis(t_xml_tag *tag,
	t_plane *plane, int *status, t_coor lengths);
/*
** object mapping and texture type mapping
*/
t_coor					ft_cart_to_sphere(t_vector vect, t_sphere *sphere);
t_coor					ft_cart_to_cone(t_vector vect, t_cone *cone);
t_coor					ft_cart_to_plane(t_vector vect, t_plane *plane);
t_coor					ft_cart_to_cylinder(t_vector vect,
				t_cylinder *cylinder, t_vector scaled_axis);
void					ft_bump_map(t_point *point, t_cam *cam);
void					ft_transparency_map(t_point *point, t_cam *cam);
void					ft_reflection_map(t_point *point, t_cam *cam);
void					ft_material_maps(t_point *point, t_cam *cam);
/*
**	get the hit color and texture informatioon
*/
t_color					ft_get_texture_color(t_texture *texture,
	t_coor uv, t_color original, int mode);
t_color					ft_get_procedural_texture_color(
	t_procedural_texture_function *texture, t_coor uv);
int						ft_is_transparent(t_material *material, t_coor uv);
int						ft_get_texture_cut(t_material *material, t_coor uv);
void					ft_get_hit_info(t_vector normal,
	t_point *point, t_cam *cam);
/*
** noise and procedural textures
*/
double					ft_turbulence(double x,
	double y, double z, double size);
t_color					ft_checker_texture(double x, double y);
t_color					ft_brick_texture(double x, double y);
t_color					ft_wood(double x, double y);
t_color					ft_marble(double x, double y);
t_color					ft_pastel(double x, double y);
t_color					ft_noise(t_cam *cam);
t_color					ft_get_procedural_texture_color(
	t_procedural_texture_function *texture, t_coor uv);
/*
** shadows related functions
*/
t_light					ft_get_shadow_light(t_rtv rtv, t_light light,
	t_vector light_vec, int depth);
t_object				*ft_get_intersection_object(t_rtv *rtv, double *min);
double					ft_basic_sphere_intersection(t_cam *cam,
		t_sphere *sphere);
double					ft_check_shadow(t_rtv rtv, t_light *light,
	t_vector light_vec);
/*
**	MathSolvers
*/
int						ft_solve_quartic(double w[5], double s[4]);
int						ft_solve_cubic(double w[4], double s[3]);
/*
** mlx functions
*/
void					ft_init_win(t_rtv *rtv);
void					ft_init_rendrering(t_rtv *rtv);
void					ft_put_pixel(t_rtv *rtv, int color);
int						ft_key_stroke(int key, t_rtv *rtv);
void					ft_clear_mlx(t_mlx *mlx, t_rtv *rtv);
int						ft_exit(t_rtv *rtv);
#endif
