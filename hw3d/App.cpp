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
	// We get the sin of the timer to get a value between -1 and 1, 
	// then divide by 2 to get a value between -0.5 and 0.5, 
	// then add 0.5 to get a value between 0 and 1

	// Note that the sine number is used as the intensity of the RGB channel

	const float t = timer.Peek();

	const float r = sin(t) / 2.0f + 0.5f;
	const float g = sin(t + 2.0f) / 2.0f + 0.5f;
	const float b = sin(t + 4.0f) / 2.0f + 0.5f;

	// Clear the graphics buffer to a specific color (time-dependent sine wave)
	wnd.Gfx().ClearBuffer(r, g, b);
	// Present the frame - this writes over the garbage frame
	wnd.Gfx().EndFrame();
}