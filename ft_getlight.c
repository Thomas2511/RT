/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_getlight.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afaucher <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/02/14 16:07:00 by afaucher          #+#    #+#             */
/*   Updated: 2014/03/26 19:19:23 by afaucher         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "raytracer.h"

static t_vect	*ft_get_lvect(t_light *llist, t_point *point)
{
	t_vect		*vect;

	vect = ft_vectornew(llist->point->x - point->x,
						llist->point->y - point->y,
						llist->point->z - point->z);
	return (vect);
}

static double	ft_get_dis(t_light *llist, t_point *point)
{
	double		dist;

	dist = sqrt(pow(llist->point->x - point->x, 2)
				+ pow(llist->point->y - point->y, 2)
				+ pow(llist->point->z - point->z, 2));
	return (dist);
}

static t_vect	*ft_get_shadow(t_obj *minobj, t_obj *olist,
						t_light *llist, t_point *point)
{
	double		dist;
	t_vect		*vect;
	double		ret;
	int			i;

	vect = ft_get_lvect(llist, point);
	dist = ft_get_dis(llist, point);
	while (olist)
	{
		i = 0;
		while (olist != minobj && i < OBJ_SIZE)
		{
			if (g_objtab[i].type == olist->type
				&& ((ret = g_objtab[i].f_inter(olist->obj, point, vect)) > 0)
				&& ret < dist)
				return (NULL);
			i++;
		}
		olist = olist->next;
	}
	return (vect);
}

static int		ft_refraction(t_line *line, t_obj *obj, int depth)
{
	double		n;
	double		cosi;
	double		sint2;
	double		cost;
	t_vect		*normal;
	int			i;
	int			color;

	depth = 0;
	color = 0;
	i = -1;
	while (++i < OBJ_SIZE)
		if (g_objtab[i].type == obj->type)
			normal = g_objtab[i].f_getnorm(obj->obj, line->origin, line->dir);
	n = 1 / obj->refl;
	cosi = -ft_getangle(line->dir, normal);
	sint2 = n * n * (1.0 - cosi * cosi);
	if (sint2 > 1.0)
		return (0);
	cost = sqrt(1.0 - sint2);
	line->dir->x = n * line->dir->x + (n * cosi - cost) * normal->x;
	line->dir->y = n * line->dir->y + (n * cosi - cost) * normal->y;
	line->dir->z = n * line->dir->z + (n * cosi - cost) * normal->z;
	if (obj->refl > 0.0)
		color = ft_getinter(line->origin, line->dir, 0, obj);
	((u_char*)&color)[0] *= obj->refl;
	((u_char*)&color)[1] *= obj->refl;
	((u_char*)&color)[2] *= obj->refl;
	return (color);
}

int				ft_reflection(t_line *line, t_obj *obj, int depth)
{
	t_vect		*normal;
	double		cosa;
	int			i;
	int			color;
	t_color		*color2;

	i = -1;
	color = 0;
	color2 = obj->color;
	while (++i < OBJ_SIZE)
		if (g_objtab[i].type == obj->type)
			normal = g_objtab[i].f_getnorm(obj->obj, line->origin, line->dir);
	cosa = -ft_getangle(line->dir, normal);
	line->dir->x += 2 * cosa * normal->x;
	line->dir->y += 2 * cosa * normal->y;
	line->dir->z += 2 * cosa * normal->z;
	ft_normalize(line->dir);
	if (depth > 0 && obj->refl > 0.0)
		color = ft_getinter(line->origin, line->dir, depth - 1, obj);
	((u_char*)&color)[0] *= obj->refl;
	((u_char*)&color)[1] *= obj->refl;
	((u_char*)&color)[2] *= obj->refl;
	return (color);
}

int				ft_getlight(t_obj *minobj, t_line *lineo, int depth)
{
	t_color		*final_color;
	int			color;
	t_vect		*vect;
	t_light		*llist;
	t_line		line;

	llist = g_scene->lights;
	final_color = ft_colornew(0, 0, 0);
	while (llist)
	{
		if ((vect = ft_get_shadow(minobj, g_scene->objs,
									llist, lineo->origin)) != NULL)
		{
			line.dir = vect;
			line.origin = lineo->origin;
			color = ft_lightcolor(minobj, llist, &line, lineo->dir);
			ft_addcolor(final_color, color);
		}
		llist = llist->next;
	}
	ft_addcolor(final_color, ft_refraction(lineo, minobj, depth));
	return (ft_colorstoi(final_color));
}
