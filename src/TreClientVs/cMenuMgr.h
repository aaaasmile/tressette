/*
    Tressette
    Copyright (C) 2005  Igor Sarzi Sartori

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Igor Sarzi Sartori
    www.invido.it
    6colpiunbucosolo@gmx.net
*/


// cMenuMgr.h: interface for the cMenuMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CMENUMGR_H__BE9C3990_5883_489E_943E_3D9832DBC369__INCLUDED_)
#define AFX_CMENUMGR_H__BE9C3990_5883_489E_943E_3D9832DBC369__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <SDL.h>
#include <SDL_ttf.h>

class cEngineApp;
class cGameMainGfx;
class cLanguages;
class cLabelLinkGfx;
class cLabelGfx;
class cImageLinkGfx;

//! class cMenuMgr  
class cMenuMgr  
{
public:
    //! menu state
    enum
    {
        //! root menu
        MENU_ROOT,
        //! options menu
        MENU_OPTIONS,
        //! credits menu
        MENU_CREDITS,
        //! game menu
        MENU_GAME,
        //! help
        MENU_HELP,
        //! quit application
        QUITAPP,
        OPT_GENERAL,
        OPT_DECK,
        OPT_GAME
    };
    enum 
    {
        MYIDLABELURL = 0,
        MYIDLABELVER = 1
    };

	cMenuMgr(cEngineApp* pApp, cGameMainGfx* pGfx);
	virtual ~cMenuMgr();
    //! Init
    void   Init(SDL_Surface *s);
    //! menu root handler
    void   HandleRootMenu();
    //! set background
    void   SetBackground(SDL_Surface* pVal){m_pScene_background = pVal;}
    //! menu option
    void   HandleOptionMenu();
    //! callback label clicked
    void   LabelClicked(int iButID);

private:
    void   drawBackground();
    void   drawStringSH(const char* tmp, int x, int y, SDL_Color& color, TTF_Font* customfont);
    void   drawStaticSpriteEx(int src_x, int src_y, int src_dx, int src_dy, int dst_x, int dst_y, SDL_Surface* sprite); 
    void   drawRect(int x, int y, int dx, int dy, SDL_Color c);
    void   drawStaticLine(int x0, int y0, int x1, int y1, SDL_Color color);
    void   setPixel( SDL_Surface* pSurface , int x , int y , SDL_Color color ); 
    void   fillRect(int x0, int y0, int width, int height, Uint32 color) ;
    void   rootMenuNext();
    void   optionMenuNext();
    


private:
    static const SDL_Color staColor_on;
    static const SDL_Color staColor_off;
    static const SDL_Color staColor_white;
    static const SDL_Color staColor_ombre;
    static const SDL_Color staColor_black;
    static const SDL_Color staColor_gray;

private:
    //! engine app
    cEngineApp*      m_pApp;
    //! get strings on differnt languages
    cLanguages*      m_pLanString;
    //! menu font
    TTF_Font*        m_pfont1;
    //! menu font
    TTF_Font*        m_pfont2;
    //! label link  font
    TTF_Font*        m_pfont3;
    //! invido grafic engine
    cGameMainGfx*    m_pInvidoGfx;
    //! main screen surface
    SDL_Surface*     m_pScreen;
    //! focus value
    int              m_ifocus_valuesM_A;
    //! upper y position in menu box
    int              m_iDeby;
    //! upper left position in box menu
    int              m_iDebx;
    int              m_iSx;
    int              m_iSy;
    //! menu panel 
    SDL_Surface*     m_pSurf_Bar;
    //! background
    SDL_Surface*     m_pScene_background;

    //! rect of the menu
    SDL_Rect         m_rctPanel;
    //! link url
    cLabelLinkGfx*   m_phomeUrl;
    cImageLinkGfx*   m_pdonateUrl;
    //! mouse position flag
    BOOL             m_bMouseInside;
    //! label version
    cLabelGfx*       m_pLabelVersion;

   
};

#endif // !defined(AFX_CMENUMGR_H__BE9C3990_5883_489E_943E_3D9832DBC369__INCLUDED_)
