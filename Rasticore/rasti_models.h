#pragma once

namespace rasticore
{
	namespace cube
	{
		float vertex[] =
		{
		-1.0, -1.0,  1.0, 0.0f, 0.0f,
		 1.0, -1.0,  1.0, 1.0f, 0.0f,
		 1.0,  1.0,  1.0, 1.0f, 1.0f,
		-1.0,  1.0,  1.0, 0.0f, 1.0f,
		// back
		-1.0, -1.0, -1.0, 0.0f, 0.0f,
		 1.0, -1.0, -1.0, 1.0f, 0.0f,
		 1.0,  1.0, -1.0, 1.0f, 1.0f,
		-1.0,  1.0, -1.0, 0.0f, 1.0f
		};

		unsigned int indice[] =
		{
			// front colors
			0, 1, 2,
			2, 3, 0,
			// right
			1, 5, 6,
			6, 2, 1,
			// back
			7, 6, 5,
			5, 4, 7,
			// left
			4, 0, 3,
			3, 7, 4,
			// bottom
			4, 5, 1,
			1, 0, 4,
			// top
			3, 2, 6,
			6, 7, 3
		};
	}
}