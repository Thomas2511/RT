/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_raytracer.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afaucher <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/02/11 19:41:11 by afaucher          #+#    #+#             */
/*   Updated: 2014/03/26 13:11:33 by afaucher         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include			"raytracer.h"

t_point				*ft_getdistpoint(t_point *origin, t_vect *dir, double dist)
{
	t_point			*new;

	if ((new = ft_pointnew(0, 0, 0)) == NULL)
		return (NULL);
	new->x = origin->x + dir->x * dist;
	new->y = origin->y + dir->y * dist;
	new->z = origin->z + dir->z * dist;
	return (new);
}

int					ft_getinter(t_scene *scene, t_point *origin,
								t_vect *dir, int depth, t_obj *obj)
{
	double			min;
	double			value;
	t_obj			*list;
	int				i;
	t_line			line;
	t_obj			*minobj;

	list = scene->objs;
	min = -1;
	minobj = NULL;
	while (list)
	{
		value = 0;
		i = -1;
		if (obj != list)
		{
			while (++i < OBJ_SIZE)
				if (list->type == g_objtab[i].type)
					value = g_objtab[i].f_inter(list->obj, origin, dir);
			if (value > 0 && (value < min || min == -1) && (minobj = list))
				min = value;
		}
		list = list->next;
	}
	if (min != -1)
	{
		line.origin = ft_getdistpoint(origin, dir, min);
		line.dir = dir;
		return (ft_getlight(minobj, scene, &line, depth));
	}
	return (0x000000);
}

static void			ft_stackcolor(t_env *env, t_vect *dirv, double rgb[3])
{
	int				tmp;

	tmp = ft_getinter(env->scene, env->scene->camera->origin, dirv, 5, NULL);
	rgb[0] += ((u_char*)&tmp)[0] / ANTIALIASING;
	rgb[1] += ((u_char*)&tmp)[1] / ANTIALIASING;
	rgb[2] += ((u_char*)&tmp)[2] / ANTIALIASING;
}

static int			ft_antialiasing(t_env *env, int x, int y, t_vect *dirv)
{
	int				idx[2];
	static double	sqrta = 0;
	int				color;
	double			rgb[3];

	idx[0] = -1;
	rgb[0] = 0;
	rgb[1] = 0;
	rgb[2] = 0;
	if (sqrta == 0)
		sqrta = sqrt(ANTIALIASING);
	while (++idx[0] < sqrta)
	{
		idx[1] = -1;
		while (++idx[1] < sqrta)
		{
			dirv = ft_getdirvector(env->scene->vpupleft, env->scene->camera,
					x - idx[0] / sqrta, y - idx[1] / sqrta);
			ft_stackcolor(env, dirv, rgb);
		}
	}
	((u_char*)&color)[0] = rgb[0];
	((u_char*)&color)[1] = rgb[1];
	((u_char*)&color)[2] = rgb[2];
	return (color);
}

void				*ft_raytracer(void *ptr_env)
{
	int				x;
	int				y;
	int				color;
	t_vect			*dirv;
	t_env			*env;

	env = (t_env*)ptr_env;
	y = 0;
	dirv = NULL;
	while (y < SIZE_Y)
	{
		x = env->xstart;
		while (x < SIZE_X)
		{
			color = ft_antialiasing(env, x, y, dirv);
			pixel_to_img(env->img, x, y, color);
			free(dirv);
			x += NB_THREAD;
		}
		y++;
	}
	printf("thread end\n");
	return (NULL);
}
