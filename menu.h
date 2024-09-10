#include <SFML/Graphics.hpp>

using namespace sf;

enum MenuChoice {
    SOLO,
    MULTI,
    EXIT
};

MenuChoice menu(sf::RenderWindow& window) {
    sf::Image img1;
    img1.loadFromFile("images/menu.png");
    img1.createMaskFromColor(sf::Color::White);

    sf::Texture menuTexture1, menuTexture2, menuTexture3, menuBackground, menuTexture4;
    menuTexture1.loadFromFile("images/solo.png");
    menuTexture2.loadFromFile("images/multi.png");
    menuTexture3.loadFromFile("images/exit.png");
    menuBackground.loadFromFile("images/fon.png");
    menuTexture4.loadFromImage(img1);

    sf::Sprite menu1(menuTexture1), menu2(menuTexture2), menu3(menuTexture3), menu4(menuTexture4), menuBg(menuBackground);
    menu1.setPosition(700, 450);
    menu2.setPosition(700, 570);
    menu3.setPosition(700, 690);
    menu4.setPosition(730, 300);
    menuBg.setPosition(-5, -5);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return EXIT;
            }

            menu1.setColor(Color::White);
            menu2.setColor(Color::White);
            menu3.setColor(Color::White);

            if (IntRect(700, 450, 300, 50).contains(Mouse::getPosition(window))) { menu1.setColor(Color::Blue); }
            if (IntRect(700, 570, 300, 50).contains(Mouse::getPosition(window))) { menu2.setColor(Color::Blue); }
            if (IntRect(700, 690, 300, 50).contains(Mouse::getPosition(window))) { menu3.setColor(Color::Blue); }

            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                if (menu1.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                    menu1.setColor(Color::Blue);
                    return SOLO;
                }
                else if (menu2.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                    menu1.setColor(Color::Blue);
                    return MULTI;
                }
                else if (menu3.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                    menu1.setColor(Color::Blue);
                    window.close();
                    return EXIT;
                }
            }
        }

        window.clear();
        window.draw(menuBg);
        window.draw(menu1);
        window.draw(menu2);
        window.draw(menu3);
        window.draw(menu4);
        window.display();
    }

    return EXIT;
}