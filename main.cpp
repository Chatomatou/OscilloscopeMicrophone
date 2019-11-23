#include <SDL2/SDL.h>
#include <vector>
#include <functional>
#include <string>
#include <cstdlib>



template<typename T>
constexpr T WINDOW_WIDTH{ 800 };

template<typename T>
constexpr T WINDOW_HEIGHT{ 600 };

template<typename T>
constexpr int SAMPLES{ 1024 };

static struct WindowManager
{
	SDL_Window* p_Window;
	SDL_Renderer* p_Renderer;

	~WindowManager()
	{
		SDL_DestroyRenderer(p_Renderer);
		SDL_DestroyWindow(p_Window);
	}
}hWindow;

static struct Oscilloscope
{
	SDL_AudioSpec spec;
	SDL_AudioDeviceID devID;
	std::vector<Uint16> buffer;

	std::function<void()> initialize = [this]() -> void
	{
		for (auto i = 0; i < SAMPLES<int>; i++)
			buffer.push_back(0);

		SDL_zero(spec);

		spec.freq = 44100;
		spec.format = AUDIO_S16SYS;
		spec.channels = 1;
		spec.samples = SAMPLES<int>;
		spec.userdata = this;
		spec.callback = [](void* userdata, Uint8* stream, int length)
		{
			for (auto i = 0; i < length / 2; i++)
				reinterpret_cast<Oscilloscope*>(userdata)->buffer[i] = static_cast<Uint16>(stream[i]);		 
		};

		SDL_Log("%s\n", SDL_GetAudioDeviceName(0, 1));

		devID = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, 1), 1, &spec, &spec, 0);

		SDL_PauseAudioDevice(devID, 0);

		if (devID == 0)
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed To Open Audio Device : %s\n", SDL_GetError());
		
 
	};

	std::function<void(WindowManager&, const SDL_Color&)>draw = [this](WindowManager& windowManager, const SDL_Color& color = { 0, 255, 0, 255 }) -> void
	{
		Uint32 zoom = color.a;
		SDL_SetRenderDrawColor(windowManager.p_Renderer, color.r, color.g, color.b, 255);

		for (auto i = 0; i < SAMPLES<int>; i++)
			SDL_RenderDrawPoint(windowManager.p_Renderer, i, WINDOW_HEIGHT<int> / 4 +  buffer[i] * zoom);

	};
}hOscilloscope;

 

int main(int argc, char* argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init : %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	if (SDL_CreateWindowAndRenderer(WINDOW_WIDTH<int>, WINDOW_HEIGHT<int>, SDL_WINDOW_SHOWN, &hWindow.p_Window, &hWindow.p_Renderer) < 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindowAndRenderer : %s\n", SDL_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}

	SDL_SetWindowTitle(hWindow.p_Window, "Oscilloscope");

	SDL_Event pumpEvents;
	SDL_bool closeApp = SDL_FALSE;

	hOscilloscope.initialize();

	Uint8 zoom = 1;
 
	while (!closeApp)
	{
		while (SDL_PollEvent(&pumpEvents))
		{
			switch (pumpEvents.type)
			{
			case SDL_QUIT:
				closeApp = SDL_TRUE;
				break;
			case SDL_MOUSEWHEEL:
				if (pumpEvents.wheel.y > 0)
					zoom++;
				if (pumpEvents.wheel.y < 0)
					zoom--;
				break;
			}
		}

		SDL_SetRenderDrawColor(hWindow.p_Renderer, 0, 0, 0, 255);
		SDL_RenderClear(hWindow.p_Renderer);
		hOscilloscope.draw(hWindow, SDL_Color{ 0, 255, 0, zoom });

		SDL_RenderPresent(hWindow.p_Renderer);
	}

	 

	SDL_Quit();
	return EXIT_SUCCESS;
}