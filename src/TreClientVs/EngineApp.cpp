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

//EngineApp.cpp
// Engine application for tressette game
#include "StdAfx.h"
#include "win_type_global.h"

#include <SDL.h>
#include <SDL_ttf.h>


#include "EngineApp.h"
#include <SDL_thread.h>
#include <SDL_mixer.h>
#include <SDL_image.h>


#include <string>

#include "lang_gen.h"
#include "cHightScoreMgr.h"
#include "cMusicManager.h"


#include "cSettings.h"
#include "cDelayNextAction.h"
#include "cGameMainGfx.h"
#include "cTipoDiMazzo.h"
#include "cMenuMgr.h"

#include "credits.h"
#include "fading.h"
#include "OptionGfx.h"
#include "ErrorMsg.h"
#include "EnterNameGfx.h"
#include "OptionDeckGfx.h"
#include "OptionGameGfx.h"

#ifdef WIN32
#include "shellapi.h"
#endif
#include "TraceService.h"

//#include "font.h"

static const char* lpszIconRes = "data/images/icona_asso.bmp";

#ifdef WIN32
    static const char* lpszIniFileOptions = "Software\\BredaSoft\\tressette";
#else
    static const char* lpszIniFileOptions = "tressette.ini";
#endif

static const char* lpszXmlFielName = "tressette.xml";


static const char* lpszIniFontAriblk = "data/font/ariblk.ttf";
static const char* lpszIniFontVera = "data/font/vera.ttf"; 
static const char* lpszImageDir = "data/images/";
static const char* lpszImageSplash = "im000740.jpg";
static const char* lpszCreditsTitle = "data/images/titlecredits.png";
static const char* lpszHelpFileName = "data/help/tre.chm";


static const char* lpszaBackGrounds_filenames[cEngineApp::NUM_BACKGRIMAGES] =
{
    "data/images/tavolino.jpg",
    "data/images/fiore7_back.jpg"
};



cEngineApp* g_MainApp = 0;



//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// class cEngineApp ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////
//       cEngineApp
/*! constructor
*/
cEngineApp::cEngineApp()
{
    m_pScreen = NULL;
    m_pWxDrawScreen = NULL;
    m_iScreenW = APP_WIN_WIDTH ; 
    m_iScreenH = APP_WIN_HEIGHT ;
#ifdef WIN32
    // applicazione supporta al massimo 800 x 600
    long min_w_poss = min(800, ::GetSystemMetrics(SM_CXFULLSCREEN));
    if(m_iScreenW < min_w_poss)
    {
        m_iScreenW = min_w_poss;
    }
    long min_h_poss = min(600, ::GetSystemMetrics(SM_CYFULLSCREEN));
    if(m_iScreenH < min_h_poss)
    {
       m_iScreenH = min_h_poss;
    }
#endif

    m_iBpp = 0;
    m_pMusicManager = 0;
    m_pLanString = 0;
    m_pMenuMgr = 0;
    m_pfontAriblk = 0;
    m_pfontVera = 0;
    m_pSlash = 0;
    m_pMxAccOptions = 0;
    m_pOptCond = 0;
    g_MainApp = this;
    m_bOpzWinRunning = FALSE;
    m_pTitleCredits = 0;
    for (int i = 0; i < cEngineApp::NUM_BACKGRIMAGES; i++)
    {
        m_pBackgrImg[i] = 0;
    }
    m_pTracer = 0;
    m_bIsWx_client = FALSE;
}


////////////////////////////////////////
//       ~cEngineApp
/*! destructor
*/
cEngineApp::~cEngineApp()
{
    terminate();
}


////////////////////////////////////////
//       loadProfile
/*! Load profiles in g_Options
*/
void cEngineApp::loadProfile()
{
    SpacecSettings::GetProfile(lpszXmlFielName);
    if(!g_Options.IsUsingXmlFile())
    {
        // settings di default non caricati dal file
#ifdef WIN32
        // la lingua di default viene stabilita dall'sistema operativo dell'utente   
        LCID lcid = ::GetUserDefaultLCID();
        if(lcid == 1040 || lcid == 2064)
        {
            g_Options.All.iLanguageID = cLanguages::LANG_ITA;
        }
        else
        {
            g_Options.All.iLanguageID = cLanguages::LANG_ENG;
        }
       

#endif
    }
}


////////////////////////////////////////
//       writeProfile
/*! Write profiles g_Options in ini file
*/
void cEngineApp::writeProfile()
{
    SpacecSettings::WriteProfile(lpszXmlFielName);
}



////////////////////////////////////////
//       Init
/*! Init application
*/
void cEngineApp::Init()
{
    
    loadProfile();
    CHAR ErrBuff[512];
    // Initialize SDL 
    if ( SDL_Init(0) < 0 ) 
    {
        sprintf(ErrBuff, "Couldn't initialize SDL: %s\n",SDL_GetError());
        throw Error::Init(ErrBuff);
    }

    if(m_bIsWx_client)
    {
        setVideoResolutionForWx();
        // m_pScreen
        m_iBpp = 24;
        int Rmask = 255;
        int Gmask = 65280;
        int Bmask = 16711680;
        // on wx wxample Rmask, Gmask, Bmask was set to 0, but so we have problem with image resources.
        m_pScreen = SDL_CreateRGBSurface(SDL_SWSURFACE, m_iScreenW, m_iScreenH, m_iBpp, Rmask, Gmask, Bmask, 0); 
        m_pWxDrawScreen = SDL_CreateRGBSurface(SDL_SWSURFACE, m_iScreenW, m_iScreenH, m_iBpp, Rmask, Gmask, Bmask, 0); 
    }
    else
    {
        setVideoResolution();
    }
    m_pMusicManager = new cMusicManager;
    m_pMusicManager->Init(); 
    
    
    m_pHmiGfx = new cGameMainGfx(this);
    m_pLanString = cLanguages::Instance();
    m_pHScore = new cHightScoreMgr;
     
    // set application language
    m_pLanString->SetLang((cLanguages::eLangId)g_Options.All.iLanguageID);
    
    // caption
    SDL_WM_SetCaption(m_pLanString->GetCStringId(cLanguages::ID_GAMENAME) ,NULL);	
    
    // hight score
    //m_HScore.Load(); 
    
    //trasparent icon
    SDL_Surface * psIcon = SDL_LoadBMP(lpszIconRes);
    if (psIcon == 0)
    {
        sprintf(ErrBuff, "Icon not found");
        throw Error::Init(ErrBuff);
    }
    SDL_SetColorKey(psIcon, SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(psIcon->format, 0, 128, 0));
    SDL_WM_SetIcon(psIcon, NULL);
    
    //HICON  hicon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_ICON1));
    
    // font TTF
    if (TTF_Init() == -1) 
    {
        sprintf(ErrBuff, "Font init error");
        throw Error::Init(ErrBuff);
    }
    // font Ariblk
    std::string strFileFontStatus = lpszIniFontAriblk; 
    m_pfontAriblk = TTF_OpenFont(strFileFontStatus.c_str()  , 22);
    if (m_pfontAriblk == 0)
    {
        sprintf(ErrBuff, "Unable to load font %s, error: %s\n", strFileFontStatus.c_str() , SDL_GetError());
        throw Error::Init(ErrBuff);
    }
    // font Vera
    strFileFontStatus = lpszIniFontVera;
    m_pfontVera = TTF_OpenFont(strFileFontStatus.c_str(), 11);
    if (m_pfontVera == 0)
    {
        sprintf(ErrBuff, "Unable to load font %s, error: %s\n", strFileFontStatus.c_str() , SDL_GetError());
        throw Error::Init(ErrBuff);
    }

    // game invido app
    m_pHmiGfx->SetMainApp(this);
    
    // menu manager
    m_pMenuMgr = new cMenuMgr(this, m_pHmiGfx);
    m_pMenuMgr->Init(m_pScreen); 

    // set main menu
    m_Histmenu.push(cMenuMgr::QUITAPP);
    m_Histmenu.push(cMenuMgr::MENU_ROOT);

    loadSplash();
    drawSplash();

    // credits title
    m_pTitleCredits = IMG_Load(lpszCreditsTitle);
    if (m_pTitleCredits == 0)
    {
        CHAR ErrBuff[512];
        sprintf(ErrBuff, "Unable to load %s  image\n" , lpszCreditsTitle);
        throw Error::Init(ErrBuff);
    }

    // load music
    // music manager initialization is a long process, update also a progress bar
    if(m_pMusicManager)
    {
        m_pMusicManager->LoadMusicRes(); 
    }

    m_pMxAccOptions = SDL_CreateMutex();
    m_pOptCond = SDL_CreateCond();

    // trace
    m_pTracer = TraceService::Instance();
    if (g_Options.All.iDebugLevel <= 1)
    {
        m_pTracer->EnableChannel(TR_ALPHABETA_CH, FALSE);
        m_pTracer->EnableChannel(TR_CORE_CH, FALSE);
        m_pTracer->EnableChannel(TR_ALG_DEF_CH, FALSE);
    }
    else if (g_Options.All.iDebugLevel == 2  )
    {
        m_pTracer->EnableChannel(TR_ALPHABETA_CH, FALSE);
        m_pTracer->EnableChannel(TR_CORE_CH, TRUE);
        m_pTracer->EnableChannel(TR_ALG_DEF_CH, FALSE);
    }
    else if (g_Options.All.iDebugLevel > 3  )
    {
        m_pTracer->EnableChannel(TR_ALPHABETA_CH, TRUE);
        m_pTracer->EnableChannel(TR_CORE_CH, TRUE);
        m_pTracer->EnableChannel(TR_ALG_DEF_CH, FALSE);
    }
    else if (g_Options.All.iDebugLevel > 5 )
    {
        m_pTracer->EnableChannel(TR_ALPHABETA_CH, TRUE);
        m_pTracer->EnableChannel(TR_CORE_CH, TRUE);
        m_pTracer->EnableChannel(TR_ALG_DEF_CH, TRUE);
    }
    if (g_Options.All.iDebugLevel > 0)
    {
        // trace channel 1 on file
        m_pTracer->SetOutputChannel(TR_CORE_CH, TraceService::OT_FILE, "trace_coregame.txt"); 
        // trace channel 2 on debugger
        m_pTracer->SetOutputChannel(TR_ALG_DEF_CH, TraceService::OT_MSVDEBUGGER, ""); 
        m_pTracer->SetOutputChannel(TR_ALPHABETA_CH, TraceService::OT_MSVDEBUGGER, ""); 
        // trace channel 3 on window
        //m_pTracer->SetOutputChannel(3, TraceService::OT_CUSTOMFN, ""); 
        //m_pTracer->SetCustomTacerInterface(this);
    }


}


////////////////////////////////////////
//       loadSplash
/*! load splash screen
*/
void cEngineApp::loadSplash()
{
    // load background
    SDL_Surface *Temp;
    std::string strFileName = lpszImageDir;
    strFileName += lpszImageSplash;

    SDL_RWops *srcBack = SDL_RWFromFile(strFileName.c_str(), "rb");
    if (srcBack==0)
    {
        CHAR ErrBuff[512];
        sprintf(ErrBuff, "Unable to load %s background image\n" , strFileName.c_str());
        throw Error::Init(ErrBuff);
    }
    Temp = IMG_LoadJPG_RW(srcBack);
    if (m_bIsWx_client)
    {
        // we can't use SDL_DisplayFormat beacuse we have no video settings, but only surfaces
        m_pSlash = SDL_ConvertSurface(Temp, Temp->format, SDL_SWSURFACE);
        /*
        m_pSlash = SDL_CreateRGBSurface(SDL_SWSURFACE, Temp->w, Temp->h, Temp->format->BitsPerPixel, Temp->format->Rmask, Temp->format->Gmask, Temp->format->Bmask, Temp->format->Amask);
        SDL_BlitSurface(Temp, NULL, m_pSlash, NULL);
        */

    }
    else
    {
        m_pSlash = SDL_DisplayFormat(Temp);
    }
    SDL_FreeSurface(Temp);

    // load a other backgorund  images
    for (int i = 0; i < cEngineApp::NUM_BACKGRIMAGES; i++)
    {
        strFileName = lpszaBackGrounds_filenames[i];

        SDL_RWops *srcBack = SDL_RWFromFile(strFileName.c_str(), "rb");
        if (srcBack==0)
        {
            CHAR ErrBuff[512];
            sprintf(ErrBuff, "Unable to load %s background image\n" , strFileName.c_str());
            throw Error::Init(ErrBuff);
        }
        Temp = IMG_LoadJPG_RW(srcBack);
        if (m_bIsWx_client)
        {
            m_pBackgrImg[i] = SDL_ConvertSurface(Temp, Temp->format, SDL_SWSURFACE);
        }
        else
        {
            m_pBackgrImg[i] = SDL_DisplayFormat(Temp);
        }
        
        SDL_FreeSurface(Temp);
    }
    
}


////////////////////////////////////////
//       drawSplash
/*! 
*/
void cEngineApp::drawSplash()
{
    SDL_BlitSurface(m_pSlash, NULL, m_pScreen, NULL);
    //SDL_Flip(m_pScreen);
    FlipScreen(m_pScreen);
}


////////////////////////////////////////
//       FlipScreen
/*! Flip or write into surface for wxclient
// \param SDL_Surface *s : source surface
*/
void cEngineApp::FlipScreen(SDL_Surface *s)
{
    if(m_bIsWx_client)
    {
        if (SDL_MUSTLOCK(m_pWxDrawScreen)) {
            if (SDL_LockSurface(m_pWxDrawScreen) < 0) {
                return;
            }
        }
        SDL_BlitSurface(s, NULL, m_pWxDrawScreen, NULL);
        if (SDL_MUSTLOCK(m_pWxDrawScreen)) {
            SDL_UnlockSurface(m_pWxDrawScreen);
        }
    }
    else
    {
        SDL_Flip(s);
    }
}


////////////////////////////////////////
//       terminate
/*! Terminate stuff
*/
void cEngineApp::terminate()
{
    writeProfile();

    SDL_ShowCursor( SDL_ENABLE );
    
    if (m_pScreen!=NULL)
    {
        SDL_FreeSurface(m_pScreen);
        m_pScreen=NULL;
    }
    if (m_pSlash)
    {
        SDL_FreeSurface(m_pSlash);
        m_pSlash = 0;
    }
    if (m_pTitleCredits)
    {
        SDL_FreeSurface(m_pTitleCredits);
        m_pTitleCredits = 0;
    }

    for (int i = 0; i < cEngineApp::NUM_BACKGRIMAGES; i++)
    {
        if (m_pBackgrImg[i])
        {
            SDL_FreeSurface(m_pBackgrImg[i]);
            m_pBackgrImg[i] = 0;
        }
    }

    delete m_pLanString;
    delete m_pHmiGfx;
    delete m_pMusicManager;
    delete m_pHScore;

    SDL_Quit();
}



////////////////////////////////////////
//       MainLoop
/*! Main loop
*/
void cEngineApp::MainLoop()
{
    bool bquit = false;
    
    // set background of menu
    m_pMenuMgr->SetBackground(m_pSlash);
    
    
    while (!bquit && !m_Histmenu.empty()) 
    {
        switch (m_Histmenu.top()) 
        {
            case cMenuMgr::MENU_ROOT:
                if (m_pMusicManager && g_Options.All.bMusicOn && !m_pMusicManager->IsPLayingMusic()  )
                {
                    m_pMusicManager->PlayMusic(cMusicManager::MUSIC_INIT_SND, cMusicManager::LOOP_ON);
                }
                m_pMenuMgr->HandleRootMenu();

                break;

            case cMenuMgr::MENU_GAME:
                PlayGame();
                break;

            case cMenuMgr::MENU_HELP:
                ShowHelp();
                break;

            case cMenuMgr::MENU_CREDITS:
                ShowCredits();
                break;

            case cMenuMgr::MENU_OPTIONS:
                m_pMenuMgr->HandleOptionMenu();
                break;

            case cMenuMgr::OPT_GENERAL:
                ShowOptionsGeneral();
                break;

            case cMenuMgr::OPT_DECK:
                ShowOptionsDeck();
                break;

            case cMenuMgr::OPT_GAME:
                ShowOptionsGame();
                break;

            case cMenuMgr::QUITAPP:
            default:
                bquit = true;
                break;
            
        }
    
        // actualize display
        //SDL_Flip(m_pScreen);
        FlipScreen(m_pScreen);

    }
}


////////////////////////////////////////
//       ShowHelp
/*! Show help menu
*/
void cEngineApp::ShowHelp()
{
    PickHelp();
    LeaveMenu();
}


////////////////////////////////////////
//       PickHelp
/*! 
*/
void cEngineApp::PickHelp()
{
#ifdef WIN32
    std::string strFileName = lpszHelpFileName;
    STRING strCompleteHelpPath = m_strApplicationDir + "\\" + strFileName;
    ::ShellExecute(NULL, TEXT("open"), strCompleteHelpPath.c_str() , 0, 0, SW_SHOWNORMAL);
#endif
}


////////////////////////////////////////
//       ShowCredits
/*! Show credits screen
*/
void cEngineApp::ShowCredits()
{
    cCredits aCred(m_pfontVera);

    aCred.Show(m_pScreen, m_pTitleCredits);

    LeaveMenu();
}

////////////////////////////////////////
//       setVideoResolution
/*! Set video resolution
*/
void cEngineApp::setVideoResolution()
{
    if (m_pScreen)
    {
        SDL_FreeSurface(m_pScreen);
    }
    m_pScreen = SDL_SetVideoMode(m_iScreenW, m_iScreenH, m_iBpp, SDL_SWSURFACE /*| SDL_RESIZABLE */);
    if ( m_pScreen == NULL )
    {
        fprintf(stderr, "Error setvideomode: %s\n", SDL_GetError());
        exit(1);
        
    }
    
}

////////////////////////////////////////
//       setVideoResolutionForWx
/*! Set video resolution for wx panel
*/
void cEngineApp::setVideoResolutionForWx()
{
    if (m_pScreen)
    {
        SDL_FreeSurface(m_pScreen);
        m_pScreen = NULL;
    }
    SDL_SetVideoMode(0, 0, 0, SDL_SWSURFACE);
}


////////////////////////////////////////
//       hightScoreMenu
/*! Shows the hight score menu
*/
void cEngineApp::hightScoreMenu()
{
    // to do
}


////////////////////////////////////////
//       PlayGame
/*! Play the game
*/
int  cEngineApp::PlayGame()
{
    if(m_pMusicManager)
    {
        m_pMusicManager->StopMusic();
    }
    //m_pMusicManager->PlayMusic(cMusicManager::MUSIC_ONPLAY, cMusicManager::LOOP_ON); 
    //m_pMusicManager->SetVolumeMusic(30);

    if (g_Options.All.strPlayerName == "Anonimo" )
    {
        // default name, show  a dialogbox to set the name
        showEditUserName();
    }
#ifndef NOPYTHON
    m_pHmiGfx->SetPythonInitScript(g_Options.All.bUsePythonAsInit, 
                        g_Options.All.strPythonInitScriptName.c_str() );
    
#endif


    // load and initialize background
    m_pHmiGfx->Initialize(m_pScreen);
    // init invido core stuff
    m_pHmiGfx->Init4PlayerGameVsCPU();
    // match main init
    m_pHmiGfx->NewMatch();
    
    // match main loop
    m_pHmiGfx->MatchLoop();
    
    // game terminated, free stuff
    m_pHmiGfx->Dispose(); 
    
    LeaveMenu();
    
    return 0;
}


////////////////////////////////////////
//       WxClient_InitGame
/*! 
*/
void cEngineApp::WxClient_InitGame()
{
    // load and initialize background
    m_pHmiGfx->Initialize(m_pScreen);
    // init invido core stuff
    m_pHmiGfx->Init4PlayerGameVsCPU();
    // match main init
    m_pHmiGfx->NewMatch();
    
}


////////////////////////////////////////
//       WxClient_GameLoop
/*! 
*/
void cEngineApp::WxClient_GameLoop()
{
    // match main loop
    m_pHmiGfx->MatchLoop();
}


////////////////////////////////////////
//       LeaveMenu
/*! Leave the current menu
*/
void   cEngineApp::LeaveMenu()
{
    drawSplash();

    m_Histmenu.pop();
}


////////////////////////////////////////
//       showEditUserName
/*! Show a dialogbox to change the user name
*/
void cEngineApp::showEditUserName()
{
    EnterNameGfx Dlg;
    SDL_Rect rctWin;
    rctWin.w = 350;
    rctWin.h = 200;

    rctWin.x = (m_pScreen->w  - rctWin.w)/2;
    rctWin.y = (m_pScreen->h - rctWin.h) / 2;
    

    Dlg.Init(&rctWin, m_pScreen, m_pfontVera, m_pfontAriblk);
    STRING strTmp = m_pLanString->GetStringId(cLanguages::ID_CHOOSENAME);
    Dlg.SetCaption(strTmp); 
    Dlg.Show(m_pSlash);

    drawSplash();
}

////////////////////////////////////////
//       ShowOptionsGeneral
/*! Show the option general control
*/
void cEngineApp::ShowOptionsGeneral()
{
    OptionGfx Optio;

    SDL_Rect rctOptionWin;

    rctOptionWin.w = OPT_WIN_GENERAL_WIDTH;
    rctOptionWin.h = OPT_WIN_GENERAL_HEIGHT;

    rctOptionWin.x = (m_pScreen->w  - rctOptionWin.w)/2;
    rctOptionWin.y = (m_pScreen->h - rctOptionWin.h) / 2;
    

    Optio.Init(&rctOptionWin, m_pScreen, m_pfontVera, m_pfontAriblk);
    STRING strTmp = m_pLanString->GetStringId(cLanguages::ID_OPT_CONTRL_GENERAL);
    Optio.SetCaption(strTmp); 
    Optio.Show(m_pBackgrImg[IMG_BACKGR_FIORE7]);

    LeaveMenu();
}



////////////////////////////////////////
//       ShowOptionsDeck
/*! Show the option deck dialogbox
*/
void cEngineApp::ShowOptionsDeck()
{
    OptionDeckGfx Optio;

    SDL_Rect rctOptionWin;

    rctOptionWin.w = OPT_WIN_DECK_WIDTH;
    rctOptionWin.h = OPT_WIN_DECK_HEIGHT;

    rctOptionWin.x = (m_pScreen->w  - rctOptionWin.w)/2;
    rctOptionWin.y = (m_pScreen->h - rctOptionWin.h) / 2;
    

    Optio.Init(&rctOptionWin, m_pScreen, m_pfontVera, m_pfontAriblk);
    STRING strTmp = m_pLanString->GetStringId(cLanguages::ID_OPT_CONTRL_DECK);
    Optio.SetCaption(strTmp); 
    Optio.Show(m_pSlash);

    LeaveMenu();
}



////////////////////////////////////////
//       ShowOptionsGame
/*! Show a control to select game specific option
*/
void cEngineApp::ShowOptionsGame()
{
    OptionGameGfx Optio;

    SDL_Rect rctOptionWin;

    rctOptionWin.w = OPT_WIN_GAME_WIDTH;
    rctOptionWin.h = OPT_WIN_GAME_HEIGHT;

    rctOptionWin.x = (m_pScreen->w  - rctOptionWin.w)/2;
    rctOptionWin.y = (m_pScreen->h - rctOptionWin.h) / 2;
    

    Optio.Init(&rctOptionWin, m_pScreen, m_pfontVera, m_pfontAriblk);
    STRING strTmp = m_pLanString->GetStringId(cLanguages::ID_OPT_CONTRL_GAME);
    Optio.SetCaption(strTmp); 
    Optio.Show(m_pBackgrImg[IMG_BACKGR_TAVOLINO]);

    LeaveMenu();
}
