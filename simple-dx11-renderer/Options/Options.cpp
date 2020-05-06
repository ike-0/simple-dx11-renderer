#include "Options.h"

// Defaults
bool	Options::Graphics::ConservativeRasterization = 1;	// Not really used
bool	Options::Graphics::RasterizerOrderViews = 1;	// Not used
bool	Options::Graphics::Tiled3DResources = 1;		// Not used
bool	Options::Graphics::Tiled2DResources = 1;		// Not used
int		Options::Graphics::MaxAnisotropy = 8;			// Not used
bool	Options::Graphics::VariableRefreshRate = true;
bool	Options::Graphics::Vsync = false; 