#! /usr/bin/env python

"""This file is part of the KDE project
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
"""
import sys
import string
import time
import os

def write_header( f ):
	print >> f, '''//
// Created: ''' + time.ctime(time.time()) + '''
//      by: ''' + os.path.basename( sys.argv[0] ) + '''
//    from: ''' + os.path.basename( sys.argv[1] ) + '''
//
// WARNING! All changes made in this file will be lost!
'''

def write_h( f ):
	print >>f, '''
#ifndef ENTITIES_H
#define ENTITIES_H

#include "kformuladefs.h"

KFORMULA_NAMESPACE_BEGIN
	
struct entityMap {
    int operator<( const char* right ) const {
	    return strcmp( name, right ) < 0;
    }
	const char* name;
	const uint unicode;
};
	
extern const entityMap entities[];

KFORMULA_NAMESPACE_END

#endif // ENTITIES_H
'''

def write_cc( fr, fw ):
	print >> fw, '''
#include "entities.h"

KFORMULA_NAMESPACE_BEGIN

const entityMap entities[] = {'''

	parse( fr, fw )
	
	print >> fw, '''
};

KFORMULA_NAMESPACE_END
	'''
	
def parse( fr, fw ):
	line = fr.readline()
	while line != "" and string.find( line, '<pre>' ) == -1:
		line = fr.readline()
	pos = string.find( line, '<pre>' ) ### Ad-hoc detection
	if pos == -1:
		return
	line = line[pos + len('<pre>'):].strip() ### Ad-hoc detection
	entries = []
	while line != "" and string.find( line, ',' ) != -1:
		fields = line.split(',')
		name = fields[0].strip()
		number = fields[1].strip()
		###
		# TODO: Support multicharacter entities, should also be supported by
		# application. The best solution would probably to map to a single
		# character provided by the font in the private area of Unicode
		if string.find( number, '-' ) == -1:
			entries.append( [name, '0x' + number[1:]] )
		line = fr.readline().strip()

	entries.reverse()
	while True:
		e = entries.pop()
		print >> fw, '     {"' + e[0] + '", ' + e[1] + '}',
		if len( entries ) == 0:
			break
		print >> fw, ','
	
if __name__ == '__main__':
	fh = open( '../entities.h', 'w' )
	write_header( fh )
	write_h( fh )
	fh.close()
	fcc = open( '../entities.cc', 'w' )
	write_header( fcc )
	fr = open( sys.argv[1] )
	write_cc( fr , fcc )
	fcc.close()
	fr.close()
	
