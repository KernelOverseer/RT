/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fractals.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abiri <abiri@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/07 18:54:44 by abiri             #+#    #+#             */
/*   Updated: 2024/06/16 21:12:42 by abiri            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rtv1.h"

int g_fractal_colors[] = {
	80, 15132394, 14344935, 13623012, 12835810, 12113887, 11326685, 10604762, 9817560, 9095637, 8308435, 7586512, 6799053, 6077387, 5289928, 4568262, 3780803, 3059137, 2271678, 1550012, 762553, 40887, 827826, 1680558, 2533290, 3320486, 4173218, 5025694, 5812890, 6665622, 7518354, 8371086, 9158026, 10010758, 10863490, 11650686, 12503418, 13355894, 14143090, 14995822, 15848554, 16701286, 16699236, 16697443, 16695649, 16693856, 16692062, 16690269, 16688475, 16686682, 16684888, 16683095, 16681302, 16679508, 16677715, 16675921, 16674128, 16672334, 16670541, 16668747, 16666954, 16665161, 16535889, 16472409, 16408929, 16345449, 16281969, 16152697, 16089217, 16025737, 15962257, 15898777, 15769505, 15706025, 15642545, 15579065, 15515585, 15386313, 15322833, 15259353, 15195873};

int			old_g_fractal_colors[] = {
	192, 873, 198251, 461421, 659055, 922226, 1185396, 1383030, 1646201,
	1909371, 2107005, 2370175, 2633346, 2830980, 3094150, 3291785, 3554955,
	3818125, 4015504, 4278674, 4541844, 4739478, 5002649, 5265819, 5463453,
	5726624, 5989794, 6187428, 6450599, 6648233, 6911403, 7174573, 7372208,
	7635378, 7898292, 8095927, 8359097, 8622267, 8819901, 9083072, 9280706,
	9543876, 9807047, 10004681, 10267851, 10531022, 10728656, 10991826,
	11254996, 11452631, 11715545, 11978715, 12176350, 12439520, 12637154,
	12900325, 13163495, 13361129, 13624299, 13887470, 14085104, 14348274,
	14611445, 14809079, 15072249, 15335420, 15334648, 15334132, 15333616,
	15398636, 15398120, 15397604, 15397088, 15462108, 15461592, 15461076,
	15526096, 15525580, 15525064, 15524548, 15589568, 15589053, 15588281,
	15653301, 15652785, 15652269, 15651753, 15716773, 15716257, 15715741,
	15780761, 15780245, 15779729, 15779213, 15844233, 15843717, 15843201,
	15908222, 15907450, 15906934, 15906418, 15971438, 15970922, 15970406,
	15969890, 16034910, 16034394, 16033878, 16098898, 16098382, 16097866,
	16097350, 16162370, 16161855, 16161083, 16226103, 16225587, 16225071,
	16224555, 16289575, 16289059, 16288543, 16353563, 16353047, 16352531,
	16352015, 16417035, 16416519, 16416003, 16481024, 16218369, 15955715,
	15693060, 15430406, 15167752, 14905097, 14642443, 14380045, 14117390,
	13854736, 13592082, 13329427, 13132309, 12869654, 12607000, 12344602,
	12081947, 11819293, 11556639, 11293984, 11031330, 10768676, 10506021,
	10243623, 9980969, 9783850, 9521196, 9258541, 8995887, 8733233, 8470578,
	8208180, 7945526, 7682871, 7420217, 7157563, 6894908, 6632254, 6435135,
	6172737, 5910083, 5647428, 5384774, 5122120, 4859465, 4596811, 4334157,
	4071758, 3809104, 3546450, 3283795, 3086677, 2824022, 2561368, 2298714,
	2036315, 1773661, 1511007, 1248352, 985698, 723044, 460389, 197735
};

double		ft_mandelbrot_distance_estimator(t_vector pos,
	int *iter, t_fractal fractal)
{
	t_vector	z;

	*iter = 0;
	z = pos;
	fractal.r = 0.0;
	fractal.dr = 1.0;
	while (*iter < fractal.max_iter)
	{
		fractal.r = ft_vector_size(z);
		if (fractal.r > fractal.bailout)
			break ;
		fractal.theta = acos(z.z / fractal.r);
		fractal.phi = atan2(z.y, z.x);
		fractal.dr = pow(fractal.r, fractal.power - 1) *
			fractal.power * fractal.dr + 1.0;
		fractal.zr = pow(fractal.r, fractal.power);
		fractal.theta = fractal.theta * fractal.power;
		fractal.phi = fractal.phi * fractal.power;
		z = ft_add_vector(ft_scale_vector((t_vector){sin(fractal.theta)
			* cos(fractal.phi), sin(fractal.phi) *
			sin(fractal.theta), cos(fractal.theta)}, fractal.zr), pos);
		(*iter)++;
	}
	return (0.5 * log(fractal.r) * fractal.r / fractal.dr);
}

t_color		ft_int_to_color(int color)
{
	double r;
	double g;
	double b;

	r = (color >> 16) & 0xFF;
	g = (color >> 8) & 0xFF;
	b = color & 0xFF;
	r /= 255.0;
	g /= 255.0;
	b /= 255.0;
	return ((t_color){r, g, b});
}

t_vector	ft_fractal_normal(t_vector pos, double distance, t_fractal *fractal)
{
	int			status;
	t_fractal	fract;

	fract = *fractal;
//	fract.max_iter = 10;
	return ((t_vector){0, 0, 0});
	return ((t_vector){
		ft_mandelbrot_distance_estimator((t_vector){pos.x + distance,
			pos.y, pos.z}, &status, fract)
		- ft_mandelbrot_distance_estimator((t_vector){pos.x - distance,
			pos.y, pos.z}, &status, fract),
		ft_mandelbrot_distance_estimator((t_vector){pos.x, pos.y + distance,
			pos.z}, &status, fract)
		- ft_mandelbrot_distance_estimator((t_vector){pos.x, pos.y - distance,
			pos.z}, &status, fract),
		ft_mandelbrot_distance_estimator((t_vector){pos.x, pos.y,
			pos.z + distance}, &status, fract)
		- ft_mandelbrot_distance_estimator((t_vector){pos.x, pos.y,
			pos.z - distance}, &status, fract)});
}

double		ft_fractal_intersection(t_cam *cam,
	t_fractal *fractal, double min)
{
	int			status;
	double		distance;
	t_color		color;
	t_vector	pos;

	pos = ft_sub_vector(cam->ray_origin, fractal->center);
	distance = trace(pos, cam->ray_direction, fractal, &status);
	if (!status || distance >= min)
		return (0);
	cam->hit.uv = (t_coor){0, 0};
	cam->hit.soluces[0] = distance;
	pos = ft_add_vector(pos, ft_scale_vector(
		ft_normalise_vector(cam->ray_direction), distance));
	color = ft_int_to_color(g_fractal_colors[(int)((double)status
		* 20.0) % 193]);
	ft_get_hit_info(ft_fractal_normal(pos, distance, fractal),
		(t_point *)fractal, cam);
	if (cam->hit.soluces[0])
		cam->hit.color = color;
	return (cam->hit.soluces[0]);
}

int			ft_add_fractal(t_xml_tag *tag, t_rtv *env)
{
	t_object	object;
	int			status;

	status = 1;
	object.fractal.center = ft_parse_vector(ft_xml_get_value(tag,
	"center", "(0,0,0)"), &status);
	object.fractal.max_iter = abs((int)ft_parse_float(ft_xml_get_value(tag,
	"iterations", "10"), &status));
	object.fractal.max_steps = abs((int)ft_parse_float(ft_xml_get_value(tag,
	"steps", "10"), &status));
	object.fractal.power = abs((int)ft_parse_float(ft_xml_get_value(tag,
	"power", "2"), &status));
	if (object.fractal.max_iter == 0)
		object.fractal.max_iter = 1;
	object.fractal.normal = ft_normalise_vector(object.fractal.normal);
	ft_add_material(tag, &object, &status, env);
	object.fractal.bailout = 2.0;
	object.fractal.function = &ft_fractal_intersection;
	status &= ft_object_push(env, object, TYPE_PLANE);
	return (status);
}
