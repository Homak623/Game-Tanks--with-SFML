#pragma once
#include <SFML/Graphics.hpp>

using namespace sf;

View view;

View getplayercoordinats(float x, float y) {
	if (x < 950)x = 950;
	if (y < 950)y = 950;
	if (y >2950)y = 2950;
	if (x > 4150)x = 4150;
	view.setCenter(x, y);
	return view;
}