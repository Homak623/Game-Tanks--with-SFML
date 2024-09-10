#include <SFML/Graphics.hpp>

using namespace sf;

class LifeBar
{
public:
    Image image;
    Texture t;
    Sprite s;
    int max;
    RectangleShape bar;

    LifeBar()
    {
        image.loadFromFile("images/newlife.png");
        image.createMaskFromColor(Color::White);
        t.loadFromImage(image);
        s.setTexture(t);
        s.setTextureRect(IntRect(3, 0, 450, 71));
        bar.setFillColor(Color(0, 0, 0, 128)); 
        max = 100;
    }

    void update(int k)
    {
        if (k <= 100 && k>=0)
        {
          //  bar.setTextureRect(IntRect(3,20, (max - k) * 347 / max, 50));
            bar.setSize(Vector2f((max - k) * 355 / max, 60));
   
        }
    }

    void draw(RenderWindow& window,String name)
    {
        Vector2f center = window.getView().getCenter();
        Vector2f size = window.getView().getSize();
        if (name == "Player1") {
            s.setPosition(center.x - size.x / 2 + 100, center.y - size.y / 2 + 30); 
            bar.setPosition(center.x - size.x / 2 + 170, center.y - size.y / 2 + 30);
          /*  bar.setTextureRect(IntRect())*/
        }
        if (name == "Player2") {
            s.setPosition(center.x + size.x / 2 - 730, center.y - size.y / 2 + 30); 
            bar.setPosition(center.x + size.x / 2 - 660, center.y - size.y / 2 + 30);
        }
        window.draw(s); 
        window.draw(bar); 
    }
};
