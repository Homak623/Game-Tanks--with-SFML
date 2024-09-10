#include <SFML/Graphics.hpp>
#include<iostream>
using namespace sf;

class Texts {
	Font font;
	Text text;
public:
	Texts(){ font.loadFromFile("font.otf"); }
	Texts(String file) { font.loadFromFile(file); }
	void SetText(String txt,Color color,Vector2f Scale, Vector2f pos) {
		text.setString(txt);
		text.setFillColor(color);
		text.setScale(Scale);
		text.setPosition(pos);
		text.setFont(font);
	}
	Text& GetText(){ return text; }
	void draw(RenderWindow& window,String name,String score=".")
	{
		if (name == "Score") {
			Vector2f center = window.getView().getCenter();
			text.setString(score);
			text.setPosition(window.getView().getCenter().x + 350, window.getView().getCenter().y - 400);
		}
		if (name == "Score name") {
			Vector2f center = window.getView().getCenter();
			text.setPosition(window.getView().getCenter().x + 120, window.getView().getCenter().y - 400);
		}
		window.draw(text);
	}
};