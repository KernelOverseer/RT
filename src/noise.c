/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   noise.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenaiss <abenaiss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/25 21:29:47 by abenaiss          #+#    #+#             */
/*   Updated: 2020/02/28 22:03:43 by abenaiss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rtv1.h"

static int g_permutation[] = {
	151, 160, 137, 91, 90, 15,
	131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8,
	99, 37, 240, 21, 10, 23,
	190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117,
	35, 11, 32, 57, 177, 33,
	88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
	134, 139, 48, 27, 166,
	77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92,
	41, 55, 46, 245, 40, 244,
	102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208,
	89, 18, 169, 200, 196,
	135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52,
	217, 226, 250, 124, 123,
	5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16,
	58, 17, 182, 189, 28, 42,
	223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101,
	155, 167, 43, 172, 9,
	129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112,
	104, 218, 246, 97, 228,
	251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145,
	235, 249, 14, 239, 107,
	49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50,
	45, 127, 4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66,
	215, 61, 156, 180,
	151, 160, 137, 91, 90, 15,
	131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8,
	99, 37, 240, 21, 10, 23,
	190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117,
	35, 11, 32, 57, 177, 33,
	88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
	134, 139, 48, 27, 166,
	77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92,
	41, 55, 46, 245, 40, 244,
	102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208,
	89, 18, 169, 200, 196,
	135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52,
	217, 226, 250, 124, 123,
	5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16,
	58, 17, 182, 189, 28, 42,
	223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101,
	155, 167, 43, 172, 9,
	129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112,
	104, 218, 246, 97, 228,
	251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145,
	235, 249, 14, 239, 107,
	49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50,
	45, 127, 4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66,
	215, 61, 156, 180,
};

double			ft_perlin_grad(int hash, float x, float y, float z)
{
	double grad;

	grad = (hash & 15);
	grad = (grad == 0) ? x + y : grad;
	grad = (grad == 1) ? -x + y : grad;
	grad = (grad == 2) ? x - y : grad;
	grad = (grad == 3) ? -x - y : grad;
	grad = (grad == 4) ? x + z : grad;
	grad = (grad == 5) ? -x + z : grad;
	grad = (grad == 6) ? x - z : grad;
	grad = (grad == 7) ? -x - z : grad;
	grad = (grad == 8) ? y + z : grad;
	grad = (grad == 9) ? -y + z : grad;
	grad = (grad == 10) ? y - z : grad;
	grad = (grad == 11) ? -y - z : grad;
	grad = (grad == 12) ? y + x : grad;
	grad = (grad == 13) ? -y + z : grad;
	grad = (grad == 14) ? y - x : grad;
	grad = (grad == 15) ? -y - z : grad;
	return (grad);
}

static double	ft_cubic_interpolation(t_vector left_side,
	t_vector right_side, t_vector point)
{
	int sides[8];

	A_ = g_permutation[((int)right_side.x) % 512] + right_side.y;
	AA = g_permutation[((int)A_) % 512] + right_side.z;
	AB = g_permutation[((int)A_ + 1) % 512] + right_side.z;
	B_ = g_permutation[((int)right_side.x + 1) % 512] + right_side.y;
	BA = g_permutation[((int)B_) % 512] + right_side.z;
	BB = g_permutation[((int)B_ + 1) % 512] + right_side.z;
	return (FT_INTERPOLATION(left_side.z, FT_INTERPOLATION(left_side.y,
	FT_INTERPOLATION(left_side.x, ft_perlin_grad(g_permutation[(AA) % 512],
	point.x, point.y, point.z), ft_perlin_grad(g_permutation[(BA) % 512],
	point.x - 1, point.y, point.z)), FT_INTERPOLATION(left_side.x,
	ft_perlin_grad(g_permutation[(AB) % 512], point.x, point.y - 1, point.z),
	ft_perlin_grad(g_permutation[(BB) % 512],
	point.x - 1, point.y - 1, point.z))), FT_INTERPOLATION(left_side.y,
	FT_INTERPOLATION(left_side.x, ft_perlin_grad(g_permutation[(AA + 1) % 512],
	point.x, point.y, point.z - 1),
	ft_perlin_grad(g_permutation[(BA + 1) % 512],
	point.x - 1, point.y, point.z - 1)), FT_INTERPOLATION(left_side.x,
	ft_perlin_grad(g_permutation[(AB + 1) % 512],
	point.x, point.y - 1, point.z - 1),
	ft_perlin_grad(g_permutation[(BB + 1) % 512], point.x - 1,
	point.y - 1, point.z - 1)))));
	return (0);
}

static double	ft_noise_3(t_vector point)
{
	t_vector	right_side;
	t_vector	left_side;

	right_side.x = (int)(point.x) % 256;
	right_side.y = (int)(point.y) % 256;
	right_side.z = (int)(point.z) % 256;
	point.x -= (int)(point.x);
	point.y -= (int)(point.y);
	point.z -= (int)(point.z);
	left_side.x = FT_FADE(point.x);
	left_side.y = FT_FADE(point.y);
	left_side.z = FT_FADE(point.z);
	return (ft_cubic_interpolation(left_side, right_side, point));
}

double			ft_turbulence(double x, double y, double z, double size)
{
	double value;
	double intital_size;

	value = 0.0;
	intital_size = size;
	while (size >= 1)
	{
		value += ft_noise_3((t_vector){fabs(x) / size,
			fabs(y) / size, fabs(z) / size}) * size;
		size /= 2.0;
	}
	return (value / intital_size);
}
