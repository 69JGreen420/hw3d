#include "App.h"

App::App()
	:
	wnd( 800,600,"The Donkey Fart Box" )
{}

int App::Go()
{
	while( true )
	{
		// process all messages pending, but to not block for new messages
		if( const auto ecode = Window::ProcessMessages() )
		{
			// if return optional has value, means we're quitting so return exit code
			return *ecode;
		}
		DoFrame();
	}
}

void App::DoFrame()
{
	const float b = sin( timer.Peek() ) / 2.0f + 0.5f;
	wnd.Gfx().ClearBuffer( 0.0f,0.0f,0.0f );
	// Animate rotation angle
	wnd.Gfx().DrawTestTriangle
	(
		// Since mouse position isn't normalised, let's temporarily
		// hardcode it so we can get accurate mouse movement (so we have between -1 and +1)
		timer.Peek(), 
		wnd.mouse.GetPosX() / 400.0f - 1.0f, 
		-wnd.mouse.GetPosY() / 300.0f + 1.0f // We need to -ve the Y-coord since graphics coordinates invert the y-axis
	);
	wnd.Gfx().EndFrame();
}