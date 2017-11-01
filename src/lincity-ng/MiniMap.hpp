/*
Copyright (C) 2005 David Kamphausen <david.kamphausen@web.de>

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
#ifndef __lc_minimap_h__
#define __lc_minimap_h__

#include "gui/Component.hpp"
#include "gui/Color.hpp"
#include "lincity/lin-city.h"
#include "gui/Texture.hpp"
#include "MapPoint.hpp"

#include <memory>

class XmlReader;
class Button;
class CheckButton;

class MiniMap:public Component
{
public:
    enum DisplayMode {NORMAL,POLLUTION,UB40,STARVE,POWER,FIRE,CRICKET,HEALTH,COAL,TRAFFIC,MAX};

    MiniMap();
    ~MiniMap();

    void parse(XmlReader& reader);
  
    virtual void draw(Painter &painter);
    virtual void event(const Event& event);
  
    void setGameViewCorners(const MapPoint& upperLeft,
            const MapPoint& upperRight, const MapPoint& lowerRight,
            const MapPoint& lowerLeft);

    Color getColor(int x,int y) const;
    Color getColorNormal(int x,int y) const;
    void showMpsEnv( MapPoint tile );
    void hideMpsEnv();

    void switchView(const std::string& viewname);

private:
    void mapViewButtonClicked(CheckButton* button, int);
    void speedButtonClicked(CheckButton* button, int);
    void zoomInButtonClicked(Button* button);
    void zoomOutButtonClicked(Button* button);
    
    void switchButton(CheckButton* button, int);
    void switchMapViewButton(const std::string &pName);
    
    void attachButtons();
    Component *findRoot(Component *c);
    Vector2 mapPointToVector(MapPoint p);
  
    Vector2 gameViewPoints[ 4 ];
    short mappointoldtype[WORLD_SIDE_LEN][WORLD_SIDE_LEN];

    DisplayMode mMode;

    int tilesize;
    int border;

    std::vector<CheckButton*> switchButtons;
    std::auto_ptr<Texture> mTexture;
    
    int mpsXOld, mpsYOld, mpsStyleOld;

    bool mFullRefresh;
    bool alreadyAttached;
    bool inside;
    // used for the middle mouse button popup to remember last visible tab
    std::string lastTabName;
};

MiniMap* getMiniMap();

#endif
