/*
Copyright (C) 2005 Matthias Braun <matze@braunis.de>

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
#include "PhysfsSDL.hpp"

#include <physfs.h>

#include <stdexcept>
#include <sstream>
#include <iostream>

#include <assert.h>

static int funcSeek(struct SDL_RWops* context, int offset, int whence)
{
    PHYSFS_file* file = (PHYSFS_file*) context->hidden.unknown.data1;
    int res;
    switch(whence) {
        case SEEK_SET:
            res = PHYSFS_seek(file, offset);
            break;
        case SEEK_CUR:
            res = PHYSFS_seek(file, PHYSFS_tell(file) + offset);
            break;
        case SEEK_END:
            res = PHYSFS_seek(file, PHYSFS_fileLength(file) + offset);
            break;
        default:
            res = 0;
            assert(false);
            break;
    }
    if(res == 0) {
        std::cerr << "Error seeking in file: " << PHYSFS_getLastError() << "\n";
        return -1;
    }

    return (int) PHYSFS_tell(file);
}

static int funcRead(struct SDL_RWops* context, void* ptr, int size, int maxnum)
{
    PHYSFS_file* file = (PHYSFS_file*) context->hidden.unknown.data1;

    int res = PHYSFS_read(file, ptr, size, maxnum);
    return res;
}

static int funcClose(struct SDL_RWops* context)
{
    PHYSFS_file* file = (PHYSFS_file*) context->hidden.unknown.data1;
    
    PHYSFS_close(file);
    delete context;

    return 0;
}

SDL_RWops* getPhysfsSDLRWops(const std::string& filename)
{
    PHYSFS_file* file = (PHYSFS_file*) PHYSFS_openRead(filename.c_str());
    if(!file) {
        std::stringstream msg;
        msg << "Couldn't open '" << filename << "': "
            << PHYSFS_getLastError();
        throw std::runtime_error(msg.str());
    }
    
    SDL_RWops* ops = new SDL_RWops();
    ops->type = 0;
    ops->hidden.unknown.data1 = file;
    ops->seek = funcSeek;
    ops->read = funcRead;
    ops->write = 0;
    ops->close = funcClose;
    return ops;
}
