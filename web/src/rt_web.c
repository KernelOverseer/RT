/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rt_web.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: KernelOverseer <marvin@42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+            */
/*   Created: 2026/09/02 by KernelOverseer        #+#    #+#              */
/*   Updated: 2026/09/02 by KernelOverseer       ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

/*
**	WebAssembly driver for the RT engine.
**
**	Replaces main.c + the minilibX window: the render core is unchanged and
**	still writes into the plain int* pixel buffer, which the JS side reads
**	through rt_web_pixels() and blits to a canvas. One instance of this
**	module runs per Web Worker; the main thread keeps the authoritative
**	camera/options and broadcasts them before every pass, mirroring the
**	per-thread band split the pthread build used (min_w/max_w).
*/

#include "rtv1.h"
#include <emscripten.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static t_rtv	g_rtv;

/*
** scene defaults, kept in sync with main.c ft_init_default_scene/camera
*/

static void	rt_web_default_camera(t_cam *cam)
{
	cam->position = (t_vector){0, 0, 0};
	cam->look_at = (t_vector){0, 0, -1};
	cam->fov = 65;
	cam->translation = (t_vector){0, 0, 0};
}

static void	rt_web_default_scene(t_scene *scene)
{
	scene->ambiant = 0.4;
	scene->filter = 0;
	scene->aa = 0;
	scene->dof = 0;
	scene->dof_focus = 10;
	scene->dof_aperture = 0.5;
	scene->reflection_depth = 3;
	scene->refraction_depth = 3;
	scene->width = 1.7777777778 * 720;
	scene->height = 720;
}

EMSCRIPTEN_KEEPALIVE
int		rt_web_init(void)
{
	ft_bzero(&g_rtv, sizeof(g_rtv));
	ttslist_init(&g_rtv.textures);
	ttslist_init(&g_rtv.buttons);
	srand(time(NULL));
	rt_web_default_camera(&g_rtv.cam);
	rt_web_default_scene(&g_rtv.scene);
	ft_memset(&g_rtv.options, 1, sizeof(g_rtv.options));
	g_rtv.anti_aliasing = 0;
	g_rtv.render_offset = 0;
	g_rtv.render_y_offset = 0;
	g_rtv.pixel_size = 1;
	g_rtv.min_h = 0;
	g_rtv.max_h = g_rtv.scene.height;
	return (0);
}

static int	rt_web_alloc_image(void)
{
	free(g_rtv.mlx.img.data);
	g_rtv.mlx.img.width = g_rtv.scene.width;
	g_rtv.mlx.img.height = g_rtv.scene.height;
	g_rtv.mlx.img.size_l = g_rtv.scene.width * 4;
	g_rtv.mlx.img.data = ft_memalloc(sizeof(int) *
		g_rtv.scene.width * g_rtv.scene.height);
	g_rtv.min_h = 0;
	g_rtv.max_h = g_rtv.scene.height;
	return (g_rtv.mlx.img.data ? 0 : -1);
}

EMSCRIPTEN_KEEPALIVE
int		rt_web_load_scene(const char *path)
{
	t_xml_data	*data;

	data = ft_read_xml((char *)path);
	if (!data)
		return (-1);
	ft_load_shapes(data, &g_rtv);
	ft_init_cam(&g_rtv.cam, g_rtv);
	g_rtv.cam.translation = (t_vector){0, 0, 0};
	return (rt_web_alloc_image());
}

EMSCRIPTEN_KEEPALIVE
int		rt_web_set_resolution(int height)
{
	if (height < 50)
		height = 50;
	g_rtv.scene.height = height;
	g_rtv.scene.width = 1.77777777778 * height;
	ft_init_cam(&g_rtv.cam, g_rtv);
	return (rt_web_alloc_image());
}

/*
** frees everything load_scene allocated, so a worker can switch scenes
** without leaking (the native binary simply exits instead)
*/

EMSCRIPTEN_KEEPALIVE
void	rt_web_dispose(void)
{
	t_object_list	*object;
	t_object_list	*object_next;
	t_light_list	*light;
	t_light_list	*light_next;
	t_texture		*texture;

	object = g_rtv.objects;
	while (object)
	{
		object_next = object->next;
		free(object);
		object = object_next;
	}
	light = g_rtv.lights;
	while (light)
	{
		light_next = light->next;
		free(light);
		light = light_next;
	}
	while (g_rtv.textures.size)
	{
		texture = g_rtv.textures.pop(&g_rtv.textures);
		free(texture->pixels);
		free(texture);
	}
	free(g_rtv.mlx.img.data);
	g_rtv.mlx.img.data = NULL;
	rt_web_init();
}

/*
** authoritative camera is owned by the JS main thread
*/

EMSCRIPTEN_KEEPALIVE
void	rt_web_set_camera(double px, double py, double pz,
			double lx, double ly, double lz, double fov)
{
	g_rtv.cam.position = (t_vector){px, py, pz};
	g_rtv.cam.look_at = (t_vector){lx, ly, lz};
	if (fov > 0)
		g_rtv.cam.fov = fov;
	ft_init_cam(&g_rtv.cam, g_rtv);
}

EMSCRIPTEN_KEEPALIVE
void	rt_web_get_camera(double *out)
{
	out[0] = g_rtv.cam.position.x;
	out[1] = g_rtv.cam.position.y;
	out[2] = g_rtv.cam.position.z;
	out[3] = g_rtv.cam.look_at.x;
	out[4] = g_rtv.cam.look_at.y;
	out[5] = g_rtv.cam.look_at.z;
	out[6] = g_rtv.cam.fov;
}

EMSCRIPTEN_KEEPALIVE
void	rt_web_set_dof_focus(double focus)
{
	g_rtv.scene.dof_focus = focus;
}

EMSCRIPTEN_KEEPALIVE
double	rt_web_get_dof_focus(void)
{
	return (g_rtv.scene.dof_focus);
}

/*
** click-to-lookat: casts one ray like the native ft_change_lookat, returns
** the distance to the hit (or -1 when nothing was hit)
*/

EMSCRIPTEN_KEEPALIVE
double	rt_web_lookat(int x, int y)
{
	g_rtv.column = y;
	g_rtv.row = x;
	g_rtv.min = MAX_D;
	ft_specific_ray_shoot(&g_rtv);
	if (g_rtv.min >= MAX_D)
		return (-1);
	g_rtv.cam.look_at = g_rtv.cam.hit.position;
	ft_init_cam(&g_rtv.cam, g_rtv);
	g_rtv.scene.dof_focus = ft_vector_size(ft_sub_vector(
		g_rtv.cam.ray_origin, g_rtv.cam.look_at));
	return (g_rtv.scene.dof_focus);
}

/*
** render options, in the same order as t_options
*/

EMSCRIPTEN_KEEPALIVE
void	rt_web_set_options(int anti_aliasing, int ambiant, int diffuse,
			int specular, int refraction, int reflection, int soft_shadows,
			int depth_of_field)
{
	g_rtv.options.anti_aliasing = anti_aliasing;
	g_rtv.options.ambiant = ambiant;
	g_rtv.options.diffuse = diffuse;
	g_rtv.options.specular = specular;
	g_rtv.options.refraction = refraction;
	g_rtv.options.reflection = reflection;
	g_rtv.options.soft_shadows = soft_shadows;
	g_rtv.options.depth_of_field = depth_of_field;
}

EMSCRIPTEN_KEEPALIVE
void	rt_web_set_quality(int light_samples, int reflection_depth,
			int refraction_depth, int dof_samples)
{
	g_rtv.scene.light_samples = (light_samples < 1) ? 1 : light_samples;
	g_rtv.scene.reflection_depth = reflection_depth;
	g_rtv.scene.refraction_depth = refraction_depth;
	g_rtv.scene.dof = dof_samples;
}

/*
** advanced per-frame settings, mirrored in the UI's Advanced panel
*/

EMSCRIPTEN_KEEPALIVE
void	rt_web_set_dof_aperture(double aperture)
{
	if (aperture < 0)
		aperture = 0;
	g_rtv.scene.dof_aperture = aperture;
}

EMSCRIPTEN_KEEPALIVE
double	rt_web_get_dof_aperture(void)
{
	return (g_rtv.scene.dof_aperture);
}

EMSCRIPTEN_KEEPALIVE
int		rt_web_scene_reflection_depth(void)
{
	return (g_rtv.scene.reflection_depth);
}

EMSCRIPTEN_KEEPALIVE
int		rt_web_scene_refraction_depth(void)
{
	return (g_rtv.scene.refraction_depth);
}

/*
**	one progressive pass over a vertical band: the JS driver owns the
**	coarse-to-fine pass ladder, each worker renders band i of n
*/

static int	rt_web_band_start(int index, int count, int width)
{
	return ((width / count) * index);
}

static int	rt_web_band_end(int index, int count, int width)
{
	if (index == count - 1)
		return (width);
	return ((width / count) * (index + 1));
}

EMSCRIPTEN_KEEPALIVE
void	rt_web_begin_pass(int pixel_size, int offset, int aa)
{
	g_rtv.pixel_size = pixel_size;
	g_rtv.render_offset = offset;
	g_rtv.render_y_offset = offset;
	g_rtv.anti_aliasing = aa;
	g_rtv.min_h = 0;
	g_rtv.max_h = g_rtv.scene.height;
}

EMSCRIPTEN_KEEPALIVE
void	rt_web_render_band(int index, int count)
{
	g_rtv.min_w = rt_web_band_start(index, count, g_rtv.scene.width);
	g_rtv.max_w = rt_web_band_end(index, count, g_rtv.scene.width);
	ft_render_band(&g_rtv);
}

/*
** RTBench-style tile render for the benchmark page: renders tile `index`
** of a tiles_x * tiles_y grid at full resolution into the shared buffer.
** Edge tiles are clamped so uneven divisions cover the frame exactly.
*/

EMSCRIPTEN_KEEPALIVE
void	rt_web_render_tile(int index, int tiles_x, int tiles_y)
{
	t_color	rgb;
	int		tile_w;
	int		tile_h;
	int		x0;
	int		y0;
	int		x1;
	int		y1;

	tile_w = g_rtv.scene.width / tiles_x;
	tile_h = g_rtv.scene.height / tiles_y;
	x0 = (index % tiles_x) * tile_w;
	y0 = (index / tiles_x) * tile_h;
	x1 = (index % tiles_x == tiles_x - 1) ? g_rtv.scene.width : x0 + tile_w;
	y1 = (index / tiles_x == tiles_y - 1) ? g_rtv.scene.height : y0 + tile_h;
	g_rtv.column = y0;
	while (g_rtv.column < y1)
	{
		g_rtv.row = x0;
		while (g_rtv.row < x1)
		{
			rgb = (t_color){0, 0, 0};
			if (g_rtv.scene.dof && g_rtv.options.depth_of_field)
				ft_color_best_node_dof(&g_rtv, rgb);
			ft_color_best_node(&g_rtv, rgb);
			g_rtv.row++;
		}
		g_rtv.column++;
	}
}

/*
** serial anaglyph stereo, band-limited version of ft_shoot_stero: the
** neighbouring bands belong to other workers, so the eye buffers are only
** summed inside this worker's band to avoid doubling stale pixels
*/

EMSCRIPTEN_KEEPALIVE
void	rt_web_stereo_band(int index, int count)
{
	t_cam	clone;
	int		*image_clone;
	int		x;
	int		y;
	int		x0;
	int		x1;

	clone = g_rtv.cam;
	x0 = rt_web_band_start(index, count, g_rtv.scene.width);
	x1 = rt_web_band_end(index, count, g_rtv.scene.width);
	image_clone = (int *)malloc(sizeof(int) *
		g_rtv.scene.width * g_rtv.scene.height);
	g_rtv.scene.filter = 8;
	g_rtv.cam.position = ft_add_vector(clone.position,
		ft_scale_vector(clone.right, -0.6));
	ft_init_cam(&g_rtv.cam, g_rtv);
	ft_render_band(&g_rtv);
	ft_memcpy(image_clone, g_rtv.mlx.img.data, 4 *
		g_rtv.scene.width * g_rtv.scene.height);
	g_rtv.scene.filter = 7;
	g_rtv.cam.position = ft_add_vector(clone.position,
		ft_scale_vector(clone.right, 0.6));
	ft_init_cam(&g_rtv.cam, g_rtv);
	ft_render_band(&g_rtv);
	y = -1;
	while (++y < g_rtv.scene.height)
	{
		x = x0 - 1;
		while (++x < x1)
			g_rtv.mlx.img.data[x + g_rtv.scene.width * y] =
				ft_rgb_to_int(ft_add_colors(
				ft_scale_colors(ft_int_to_rgb(image_clone[x +
				g_rtv.scene.width * y]), 1.0 / 255.0),
				ft_scale_colors(ft_int_to_rgb(g_rtv.mlx.img.data[x +
				g_rtv.scene.width * y]), 1.0 / 255.0)));
	}
	free(image_clone);
	g_rtv.cam = clone;
	ft_init_cam(&g_rtv.cam, g_rtv);
	g_rtv.scene.filter = 0;
}

/*
** pixel buffer access for the JS compositor
*/

EMSCRIPTEN_KEEPALIVE
void	*rt_web_pixels(void)
{
	return ((void *)g_rtv.mlx.img.data);
}

EMSCRIPTEN_KEEPALIVE
int		rt_web_pixels_len(void)
{
	return (4 * g_rtv.scene.width * g_rtv.scene.height);
}

/*
** full-image post ops (kernel filters) run on a single worker: the main
** thread uploads the composited frame back before the call
*/

EMSCRIPTEN_KEEPALIVE
void	rt_web_upload_pixels(void *buffer, int len)
{
	if (len > rt_web_pixels_len())
		len = rt_web_pixels_len();
	ft_memcpy(g_rtv.mlx.img.data, buffer, len);
}

EMSCRIPTEN_KEEPALIVE
void	rt_web_set_filter(int filter)
{
	g_rtv.scene.filter = filter;
}

EMSCRIPTEN_KEEPALIVE
int		rt_web_get_filter(void)
{
	return (g_rtv.scene.filter);
}

EMSCRIPTEN_KEEPALIVE
void	rt_web_set_effect(int effect)
{
	g_rtv.scene.effect = effect;
}

EMSCRIPTEN_KEEPALIVE
void	rt_web_apply_effect(void)
{
	ft_filtring_select(&g_rtv);
}

EMSCRIPTEN_KEEPALIVE
int		rt_web_save_bmp(const char *path)
{
	return (ft_save_bitmap(&g_rtv.mlx.img, (char *)path));
}

/*
** getters for the driver UI
*/

EMSCRIPTEN_KEEPALIVE
int		rt_web_scene_width(void)
{
	return (g_rtv.scene.width);
}

EMSCRIPTEN_KEEPALIVE
int		rt_web_scene_height(void)
{
	return (g_rtv.scene.height);
}

EMSCRIPTEN_KEEPALIVE
int		rt_web_scene_aa(void)
{
	return (g_rtv.scene.aa);
}

EMSCRIPTEN_KEEPALIVE
int		rt_web_scene_dof(void)
{
	return (g_rtv.scene.dof);
}

EMSCRIPTEN_KEEPALIVE
int		rt_web_scene_light_samples(void)
{
	return (g_rtv.scene.light_samples);
}
