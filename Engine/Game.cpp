/****************************************************************************************** 
 *	Chili DirectX Framework Version 16.07.20											  *	
 *	Game.cpp																			  *
 *	Copyright 2016 PlanetChili.net <http://www.planetchili.net>							  *
 *																						  *
 *	This file is part of The Chili DirectX Framework.									  *
 *																						  *
 *	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
 *	it under the terms of the GNU General Public License as published by				  *
 *	the Free Software Foundation, either version 3 of the License, or					  *
 *	(at your option) any later version.													  *
 *																						  *
 *	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
 *	GNU General Public License for more details.										  *
 *																						  *
 *	You should have received a copy of the GNU General Public License					  *
 *	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
 ******************************************************************************************/
#include "MainWindow.h"
#include "Game.h"
#include "SpriteCodex.h"

Game::Game( MainWindow& wnd )
	:
	wnd( wnd ),
	gfx( wnd ),
	brd( settings,gfx ),
	rng( std::random_device()() ),
	snek( {2,2} ),
	nPoison( settings.GetPoisonAmount() ),
	nFood( settings.GetFoodAmount() ),
	snekSpeedupFactor( settings.GetSpeedupRate() )
{
	for( int i = 0; i < nPoison; i++ )
	{
		brd.SpawnContents( rng,snek,Board::CellContents::Poison );
	}
	for( int i = 0; i < nFood; i++ )
	{
		brd.SpawnContents( rng,snek,Board::CellContents::Food );
	}
	sndTitle.Play( 1.0f,1.0f );
}

void Game::Go()
{
	gfx.BeginFrame();	
	UpdateModel();
	ComposeFrame();
	gfx.EndFrame();
}

void Game::UpdateModel()
{
	const float dt = ft.Mark();
	
	if( gameIsStarted )
	{
		if( !gameIsOver )
		{
			if( wnd.kbd.KeyIsPressed( VK_UP ) )
			{
				const Location new_delta_loc = { 0,-1 };
				if( delta_loc != -new_delta_loc || snek.GetLength() <= 2 )
				{
					delta_loc = new_delta_loc;
				}
			}
			else if( wnd.kbd.KeyIsPressed( VK_DOWN ) )
			{
				const Location new_delta_loc = { 0,1 };
				if( delta_loc != -new_delta_loc || snek.GetLength() <= 2 )
				{
					delta_loc = new_delta_loc;
				}
			}
			else if( wnd.kbd.KeyIsPressed( VK_LEFT ) )
			{
				const Location new_delta_loc = { -1,0 };
				if( delta_loc != -new_delta_loc || snek.GetLength() <= 2 )
				{
					delta_loc = new_delta_loc;
				}
			}
			else if( wnd.kbd.KeyIsPressed( VK_RIGHT ) )
			{
				const Location new_delta_loc = { 1,0 };
				if( delta_loc != -new_delta_loc || snek.GetLength() <= 2 )
				{
					delta_loc = new_delta_loc;
				}
			}

			float snekModifiedMovePeriod = snekMovePeriod;
			if( wnd.kbd.KeyIsPressed( VK_CONTROL ) )
			{
				snekModifiedMovePeriod = std::min( snekMovePeriod,snekMovePeriodSpeedup );
			}

			snekMoveCounter += dt;
			if( snekMoveCounter >= snekModifiedMovePeriod )
			{
				snekMoveCounter -= snekModifiedMovePeriod;
				const Location next = snek.GetNextHeadLocation( delta_loc );
				const Board::CellContents contents = brd.IsInsideBoard( next ) ? brd.GetContents( next ) 
					: Board::CellContents::Empty;
				if( !brd.IsInsideBoard( next ) ||
					snek.IsInTileExceptEnd( next ) ||
					contents == Board::CellContents::Obstacle )
				{
					gameIsOver = true;
					sndFart.Play( rng,1.2f );
					sndMusic.StopAll();
				}
				else if( contents == Board::CellContents::Food )
				{
					snek.GrowAndMoveBy( delta_loc );
					brd.ConsumeContents( next );
					brd.SpawnContents( rng,snek,Board::CellContents::Obstacle );
					brd.SpawnContents( rng,snek,Board::CellContents::Food );
					sfxEat.Play( rng,0.8f );
				}
				else if( contents == Board::CellContents::Poison )
				{
					snek.MoveBy( delta_loc );
					brd.ConsumeContents( next );
					snekMovePeriod = std::max( snekMovePeriod * snekSpeedupFactor,snekMovePeriodMin );
					sndFart.Play( rng,0.6f );
				}
				else
				{
					snek.MoveBy( delta_loc );
					sfxSlither.Play( rng,0.08f );
				}
			}
		}
	}
	else
	{
		if( wnd.kbd.KeyIsPressed( VK_RETURN ) )
		{
			sndMusic.Play( 1.0f,0.6f );
			gameIsStarted = true;
		}
	}
}

void Game::ComposeFrame()
{
	if( gameIsStarted )
	{
		snek.Draw( brd );
		brd.DrawCells();
		if( gameIsOver )
		{
			SpriteCodex::DrawGameOver( 350,265,gfx );
		}
		brd.DrawBorder();
	}
	else
	{
		SpriteCodex::DrawTitle( 290,225,gfx );
	}
}

//I do not understand a single thing in here
//life of a programmer is fine
