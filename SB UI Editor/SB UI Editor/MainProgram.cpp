#include "MainProgram.h"

#include "LoadLayout.h"

MainProgram::MainProgram()													{}

const bool MainProgram::Initialize()
{
	LoadLayoutFile("Assets/Assets/Layout/layout.xml");

	//  Rendering Order: Main Menu
	const char* MainMenuRenderingList[] = 
	{
		"Root.SuperBingo_FrontEnd.FrontEnd_BG",
		"Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.SuperBingo_SGB_Logo",
		"Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.SuperBingo_SCB_Logo",
		"Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.SuperBingo_SWB_Logo",
		"Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.SuperBingo_SBB_Logo",
		"Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.SuperBingo_SPB_Logo",
		"Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.SuperBingo_SProB_Logo",
		"Root.SuperBingo_FrontEnd.SB_UI_SP_SYGText",
		"Root.SuperBingo_FrontEnd.SB_UI_SP_Logo"
	};

	//  Rendering Order: Game Size Menu
	const char* SuperWildRenderingList[] = 
	{
		"Root.SuperBingo_Wild.SB_UI_SBW_BG",
		"Root.SuperBingo_Wild.SB_UI_SBW_Logo",
		"Root.BingoBoards.Board_BINGO.SB_UI_BoardBINGO",
		"Root.UI_Basic.MainHUD.SB_UI_ButtonFooter",
		"Root.UI_Basic.MainHUD.SB_UI_HelpSeePaysButton",
		"Root.UI_Basic.MainHUD.SB_UI_ChangeGameButton",
		"Root.UI_Basic.MainHUD.SB_UI_AddCardsButton",
		"Root.UI_Basic.MainHUD.SB_UI_MaxCardsButton",
		"Root.UI_Basic.MainHUD.SB_UI_CoinDenominationButton.NICKEL",
		"Root.UI_Basic.MainHUD.SB_UI_BetOneButton",
		"Root.UI_Basic.MainHUD.SB_UI_BetMaxButton",
		"Root.UI_Basic.MainHUD.SB_UI_PlayButton",
		"Root.UI_Basic.MainHUD.SB_UI_BallCalls",
		"Root.BingoCards.Card_1.Card_ONE.Card_BINGO.Proxy_BingoCard.SB_UI_CardBINGO"
	};

	SpecifyLayoutRenderingOrder(SuperWildRenderingList, 14);

	return true;
}

const bool MainProgram::Input( const float time_slice )
{
	LayoutInput();

	return true;
}

const bool MainProgram::Update( const float time_slice )			{ return true; }
const bool MainProgram::Render3D( void )								{ return true; }

const bool MainProgram::Render2D( void )
{
	RenderLayout();
	return true;
}

const bool MainProgram::Shutdown()
{
	UnloadLayoutFile();
	return true;
}