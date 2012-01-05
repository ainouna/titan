/* pkg_vec.h - the itsy package management system

   Steven M. Ayer
   
   Copyright (C) 2002 Compaq Computer Corporation

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
*/

#ifndef PKG_VEC_H
#define PKG_VEC_H

typedef struct pkg pkg_t;
typedef struct abstract_pkg abstract_pkg_t;

struct pkg_vec
{
    pkg_t **pkgs;
    int len;
};
typedef struct pkg_vec pkg_vec_t;

struct abstract_pkg_vec
{
    abstract_pkg_t **pkgs;
    int len;
};
typedef struct abstract_pkg_vec abstract_pkg_vec_t;


pkg_vec_t * pkg_vec_alloc(void);
void pkg_vec_free(pkg_vec_t *vec);
void marry_two_packages(pkg_t * newpkg, pkg_t * oldpkg);

void pkg_vec_add(pkg_vec_t *vec, pkg_t *pkg);
/* pkg_vec_insert_merge: might munge pkg.
*  returns the pkg that is in the pkg graph */
pkg_t *pkg_vec_insert_merge(pkg_vec_t *vec, pkg_t *pkg, int set_status, ipkg_conf_t *conf);
/* this one never munges pkg */
void pkg_vec_insert(pkg_vec_t *vec, const pkg_t *pkg);
int pkg_vec_contains(pkg_vec_t *vec, pkg_t *apkg);
void pkg_vec_sort(pkg_vec_t *vec, int (*compar)(pkg_t *, pkg_t *));

int pkg_vec_clear_marks(pkg_vec_t *vec);
int pkg_vec_mark_if_matches(pkg_vec_t *vec, const char *pattern);

abstract_pkg_vec_t * abstract_pkg_vec_alloc(void);
void abstract_pkg_vec_free(abstract_pkg_vec_t *vec);
void abstract_pkg_vec_insert(abstract_pkg_vec_t *vec, abstract_pkg_t *pkg);
abstract_pkg_t * abstract_pkg_vec_get(abstract_pkg_vec_t *vec, int i);
int abstract_pkg_vec_contains(abstract_pkg_vec_t *vec, abstract_pkg_t *apkg);
void abstract_pkg_vec_sort(pkg_vec_t *vec, int (*compar)(abstract_pkg_t *, abstract_pkg_t *));
#endif

