/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dump_bitmap.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abiri <abiri@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/16 18:15:46 by abiri             #+#    #+#             */
/*   Updated: 2020/01/30 16:25:55 by abiri            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rtv1.h"
#include <sys/time.h>

int		ft_write_header(int fd, t_img *image)
{
	uint16_t	magic;
	uint32_t	size;
	int			bytes_size;

	magic = BMP_MAGIC_HEADER;
	bytes_size = image->width * image->height * 4;
	size = BMP_ALL_HEADERS_SIZE + bytes_size;
	if (write(fd, &magic, 2) != 2)
		return (ERROR);
	if (write(fd, &size, 4) != 4)
		return (ERROR);
	if (write(fd, "\0\0\0\0", 4) != 4)
		return (ERROR);
	size = BMP_ALL_HEADERS_SIZE;
	if (write(fd, &size, 4) != 4)
		return (ERROR);
	return (SUCCESS);
}

int		ft_write_bitmap_info(int fd, t_img *image)
{
	uint16_t	int_2bytes;
	uint32_t	int_4bytes;

	int_4bytes = BMP_INFO_HEADER_SIZE;
	if (write(fd, &int_4bytes, 4) != 4)
		return (ERROR);
	int_4bytes = image->width;
	if (write(fd, &int_4bytes, 4) != 4)
		return (ERROR);
	int_4bytes = image->height * -1;
	if (write(fd, &int_4bytes, 4) != 4)
		return (ERROR);
	int_2bytes = BMP_PLANES_NUMBER;
	if (write(fd, &int_2bytes, 2) != 2)
		return (ERROR);
	int_2bytes = BMP_BITS_PER_PIXEL;
	if (write(fd, &int_2bytes, 2) != 2)
		return (ERROR);
	if (write(fd, "\0\0\0\0\0\0\0\0\0\0\
		0\0\0\0\0\0\0\0\0\0\0\0\0\0", 24) != 24)
		return (ERROR);
	return (SUCCESS);
}

char	*ft_get_image_name(void)
{
	char			*result;
	char			*sec_str;
	char			*mil_str;
	struct timeval	creation;

	gettimeofday(&creation, NULL);
	sec_str = ft_itoa(creation.tv_sec);
	mil_str = ft_itoa(creation.tv_usec);
	result = ft_strjoin(sec_str, mil_str);
	free(sec_str);
	free(mil_str);
	sec_str = result;
	result = ft_strjoin(result, ".bmp");
	free(sec_str);
	return (result);
}

int		ft_dump_bitmap(t_img *image)
{
	int		fd;
	int		size;
	char	*screenshot;

	screenshot = ft_get_image_name();
	size = image->width * image->height * 4;
	if ((fd = open(screenshot, O_WRONLY |
		O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO)) < 0)
		return (ERROR);
	if (!ft_write_header(fd, image))
		return (ERROR);
	if (!ft_write_bitmap_info(fd, image))
		return (ERROR);
	if (write(fd, image->data, size) != size)
		return (ERROR);
	ft_strdel(&screenshot);
	return (SUCCESS);
}

int		ft_save_bitmap(t_img *image, char *filename)
{
	int		fd;
	int		size;

	size = image->width * image->height * 4;
	if ((fd = open(filename, O_WRONLY |
		O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO)) < 0)
		return (ERROR);
	if (!ft_write_header(fd, image))
		return (ERROR);
	if (!ft_write_bitmap_info(fd, image))
		return (ERROR);
	if (write(fd, image->data, size) != size)
		return (ERROR);
	return (SUCCESS);
}
