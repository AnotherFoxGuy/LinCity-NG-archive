/*
Copyright (C) 2004 by Matthias Braun <matze@braunis.de>
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef __PHYSFSSTREAM_HPP__
#define __PHYSFSSTREAM_HPP__

#include <stdio.h>
#include <physfs.h>
#include <string>
#include <streambuf>
#include <iostream>

/** This class implements a C++ streambuf object for physfs files.
 * So that you can use normal istream operations on them
 */
class IFileStreambuf : public std::streambuf
{
public:
    IFileStreambuf(const std::string& filename);
    ~IFileStreambuf();
    
protected:
    virtual int underflow();

private:
    PHYSFS_file* file;
    char buf[1024];
};

class OFileStreambuf : public std::streambuf
{
public:
    OFileStreambuf(const std::string& filename);
    ~OFileStreambuf();

protected:
    virtual int overflow(int c);
    virtual int sync();

private:
    PHYSFS_file* file;
    char buf[1024];
};

class IFileStream : public std::istream
{
public:
    IFileStream(const std::string& filename);
    ~IFileStream();
};

class OFileStream : public std::ostream
{
public:
    OFileStream(const std::string& filename);
    ~OFileStream();
};

#endif

