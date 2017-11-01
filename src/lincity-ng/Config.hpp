/*
Copyright (C) 2005 Wolfgang Becker <uafr@gmx.de>

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
#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <string>
#include <limits.h>
#include <float.h>

class Config 
{
public:
    Config();
    ~Config();
    
    bool useOpenGL;
    bool useFullScreen;
    bool restartOnChangeScreen;
    int videoX, videoY;
    int monthgraphW, monthgraphH;
    
    // sound volume 0..100 (0=silent)
    int soundVolume;
    // music volume 0..100
    int musicVolume;
    bool soundEnabled;
    bool musicEnabled;
    //std::string lincityHome;
    int skipMonthsFast;
    // how fast is fast_time_for_year
    int quickness;

    void save();
    std::string playSongName;
private:
    void load( const std::string& filename );
    int parseInt(const char* value, int defaultValue, int minValue = INT_MIN, 
            int maxValue = INT_MAX );
    bool parseBool(const char* value, bool defaultvalue);
};

Config* getConfig();

#endif
