#include <string>

namespace SableUI
{
	void CreateWindow(const std::string& title, int width, int height, int x = -1, int y = -1);

	bool PollEvents();
	void Draw();
	void SetMaxFPS(int fps);

	void Destroy();
}