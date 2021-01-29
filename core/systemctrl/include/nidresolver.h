/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#ifndef NIDRESOLVER_H
#define NIDRESOLVER_H

#include "module2.h"

// Missing NID
typedef struct NidMissingEntry {
    unsigned int nid;
    unsigned int fp;
} NidMissingEntry;

// Missing NID Resolver
typedef struct NidMissingResolver {
    const char * libname;
    NidMissingEntry * entry;
    unsigned int size;
} NidMissingResolver;

// NID Resolver Entry
typedef struct NidResolverEntry {
    unsigned int old;
    unsigned int new;
} NidResolverEntry;

// NID Resolver Library
typedef struct NidResolverLib {
    char * name;
    unsigned int nidcount;
    NidResolverEntry * nidtable;
    unsigned int enabled;
} NidResolverLib;

// NID Table
extern NidResolverLib * nidTable;
extern unsigned int nidTableSize;

// NID Table from from nid_660_data.c
extern NidResolverLib nidTable660[];
extern unsigned int nidTableSize660;

// Unknown NID Dummy
#define UNKNOWNNID 0xDEADBEEF

// Table Entry Macro
#define NID_ENTRY(libname) \
    { #libname, NELEMS(libname##_nid), libname##_nid, 1, }

// Get NID Resolver for Library
NidResolverLib * getNidResolverLib(const char *libName);

// Resolve Library NID
unsigned int getNidReplacement(const NidResolverLib *lib, unsigned int nid);

// Missing syscon NID
void resolve_syscon_driver(SceModule2*);

// Initialize NID Resolver
void setupNidResolver(SceModule2* loadcore);

#endif

