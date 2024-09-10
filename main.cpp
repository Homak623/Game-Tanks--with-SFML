#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include"menu.h"
#include"View.h"
#include<iostream>
#include"Map.h"
#include "Life.h"
#include"level.h"
#include<vector>
#include<list>
#include<forward_list>
#include <thread>
#include"Score.h"
#include"addHealth.h"
#include <random>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include"ThreadPoll.h"
#include<future>

using namespace sf;

Image loadImage(const std::string& filename, const Color& maskColor) {
    Image img;
    if (!img.loadFromFile(filename)) {
       throw std::exception("Error: Failed to load image from file ");
    }
    img.createMaskFromColor(maskColor);
    return img;
}

Sound loadShoot(const std::string& filename, SoundBuffer& shootBuffer) {
    if (!shootBuffer.loadFromFile(filename)) {
        throw std::exception("Error: Failed to load sound from file ");
    }
    Sound shoot(shootBuffer);
    return shoot;
}

class Entity {
protected:
    enum state { left, right, up, down } dir;
    std::vector<Object> obj;
    float dx, dy, x, y, speed;
    int w, h, health;
    bool life;
    Texture texture;
    Sprite sprite;
    String name;
    bool CheckCollisionWithMap();
public:
    Entity(const Image& image,const float &x, const float &y, const int &w, const int &h, const String &name, Level &lvl);
        FloatRect getRect() { return FloatRect(x, y, w, h);}
        virtual void change(const float &time) = 0;
        Sprite GetSprite() const { return sprite; }
        String GetName() const { return name; }
        bool GetLife() const { return life; }
        int GetHealth() const { return health; }
        void SetHealth(const int health) { this->health = health; }
        int GetW() const { return w; }
        void SetW(const int w) { this->w = w; }
        int GetH() const { return h; }
        void SetH(const int h) { this->h = h; }
        float GetX() const { return x; }
        void SetX() { this->x = x; }
        float GetY() const { return y; }
        enum state GetState() const { return dir; }
        void handleCollision(Entity& otherEntity);      
};

bool Entity::CheckCollisionWithMap() {
    for (int i = 0; i < obj.size(); i++)
        if (getRect().intersects(obj[i].rect))
        {
            if (obj[i].name == "solid" || obj[i].name == "stop")
            {
                return  true;
            }
        }
    return false;
}

Entity::Entity(const Image& image, const  float &x, const float &y, const int &w, const int &h, const String &name, Level &lvl)
    :x{x}, 
    y{ y }, w{ w }, h{ h }, name{ name }, obj{ lvl.GetAllObjects() }, speed{}, health{ 100 }, dx{}, dy{}, life{ true }
{
    texture.loadFromImage(image);
    sprite.setTexture(texture);
}


void Entity::handleCollision(Entity& otherEntity) {
    if (this->GetSprite().getGlobalBounds().intersects(otherEntity.GetSprite().getGlobalBounds())) {
        float overlapX = (this->GetSprite().getGlobalBounds().width + otherEntity.GetSprite().getGlobalBounds().width) / 2.0f - std::abs(this->x - otherEntity.x);
        float overlapY = (this->GetSprite().getGlobalBounds().height + otherEntity.GetSprite().getGlobalBounds().height) / 2.0f - std::abs(this->y - otherEntity.y);
        Vector2f coordFirst(this->x, this->y);
        Vector2f coordSecond(otherEntity.x, otherEntity.y);
        if (overlapX < overlapY) {
            if (this->x < otherEntity.x) {
                this->x -= overlapX / 2.0f;
                otherEntity.x += overlapX / 2.0f;
            }
            else {
                this->x += overlapX / 2.0f;
                otherEntity.x -= overlapX / 2.0f;
            }
        }
        else {
            if (this->y < otherEntity.y) {
                this->y -= overlapY / 2.0f;
                otherEntity.y += overlapY / 2.0f;
            }
            else {
                this->y += overlapY / 2.0f;
                otherEntity.y -= overlapY / 2.0f;
            }
        }

        if (this->CheckCollisionWithMap()) {
            this->x = coordFirst.x;
            this->y = coordFirst.y;
        }
        if (otherEntity.CheckCollisionWithMap()) {
            otherEntity.x = coordSecond.x;
            otherEntity.y = coordSecond.y;
        }
    }
}



class Player :public Entity {
    int playerScore;
    bool isShoot;
    float CurrentFrame;
    float shootCooldown;
    SoundBuffer soundBuff;
    Sound soundObj;
public:
    Player(const Image& image,Level& lvl, const  float &x, const  float &y, const  int &w, const int &h, const String &name);
    const bool GetIsShoot() const{ return isShoot; }
    void SetIsShoot(const bool &isShoot){this->isShoot=isShoot;}
    int GetScore() const{ return playerScore; }
    void SetScore(const int &playerScore) { this->playerScore = playerScore; }
    void change(const float &time);
    void handleCollisionWithPlayer(Player& otherPlayer);
    void interactionWithMap();
    void control();
    bool canShoot();
    void CheckCollisionWithBuffs(std::vector<PickUP>& health);
};

void Player::CheckCollisionWithBuffs(std::vector<PickUP>& health) {
    for (int i = health.size() - 1; i >= 0; i--) {
        if (health[i].isSpawned()) {
            if (this->getRect().intersects(health[i].getPosition()) && this->GetName() == "Player1") {
                if (health[i].GetType() == HILL) {
                    this->health += health[i].GetValue();
                    if (this->health >= 100)
                        this->health = 100;
                    soundObj = loadShoot("coin.ogg", soundBuff);
                    soundObj.play();
                }
                else if (health[i].GetType() == COIN)
                {
                    this->SetScore(this->GetScore() + health[i].GetValue());
                    soundObj = loadShoot("addhealth.ogg", soundBuff);
                    soundObj.play();
                }
                health.erase(health.begin() + i);
            }
        }
        else {
            health.erase(health.begin() + i);
        }
    }
}

bool Player::canShoot() {
    if (shootCooldown >= 800) {
        shootCooldown = 0;
        return true;
    }
    return false;
}

Player::Player(const Image& image, Level& lvl, const float &x, const float &y,const int &w, const int &h, const String &name) :Entity(image, x, y, w, h, name, lvl), playerScore{ 0 }, CurrentFrame{ 0 }, shootCooldown{ 0 }{
    if (name == "Player1" || name == "Player2") {
        dir = right;
        sprite.setTextureRect(IntRect(89, 10, w, h));
    }
}

void Player::control() {
    std::map<sf::Keyboard::Key, state> player1KeyBindings = {
        {sf::Keyboard::Left, left},
        {sf::Keyboard::Right, right},
        {sf::Keyboard::Up, up},
        {sf::Keyboard::Down, down}
    };

    std::map<sf::Keyboard::Key, state> player2KeyBindings = {
        {sf::Keyboard::A, left},
        {sf::Keyboard::D, right},
        {sf::Keyboard::W, up},
        {sf::Keyboard::S, down}
    };

    if (name == "Player1") {
        for (const std::pair<sf::Keyboard::Key, state>& pair : player1KeyBindings) {
            const sf::Keyboard::Key key = pair.first;
            const state direction = pair.second;

            if (sf::Keyboard::isKeyPressed(key)) {
                dir = direction;
                speed = 0.12;
                getplayercoordinats(x, y);
            }
        }
        if (Keyboard::isKeyPressed(Keyboard::Space) && canShoot()) {
            isShoot = true;
        }
    }
    else if (name == "Player2") {
        for (const std::pair<sf::Keyboard::Key, state>& pair : player2KeyBindings) {
            const sf::Keyboard::Key key = pair.first;
            const state direction = pair.second;

            if (sf::Keyboard::isKeyPressed(key)) {
                dir = direction;
                speed = 0.12;
                getplayercoordinats(x, y);
            }
        }
        if (Keyboard::isKeyPressed(Keyboard::F) && canShoot()) {
            isShoot = true;
        }
    }


}


void Player::interactionWithMap() {
    for (int i = 0; i < obj.size(); i++)
        if (getRect().intersects(obj[i].rect))
        {
            if (obj[i].name == "solid" || obj[i].name == "stop")
            {
                if (dy > 0) { y = obj[i].rect.top - h;  dy = 0; }
                if (dy < 0) { y = obj[i].rect.top + obj[i].rect.height;   dy = 0; }
                if (dx > 0) { x = obj[i].rect.left - w; dx = 0; }
                if (dx < 0) { x = obj[i].rect.left + obj[i].rect.width; dx = 0; }
            }
        }
}


void Player::change(const float &time) {
    control();
    shootCooldown += time*0.4;
    if (speed > 0 && life==true) {
        CurrentFrame += 0.005 * time;
        if (CurrentFrame > 7) CurrentFrame -= 7;

        switch (dir) {
        case right: {
            dx = speed;
            dy = 0;
            sprite.setTextureRect(IntRect(89 * (int(CurrentFrame)), 10, 76, 62));
            break;
        }
        case left: {
            dx = -speed;
            dy = 0;
            sprite.setTextureRect(IntRect(89 * (int(CurrentFrame)), 85, 76, 62));
            break;
        }
        case down: {
            dy = speed;
            dx = 0;
            sprite.setTextureRect(IntRect(102, 155 + 89 * (int(CurrentFrame)), 65, 76));
            break;
        }
        case up: {
            dy = -speed;
            dx = 0;
            sprite.setTextureRect(IntRect(14, 155 + 89 * (int(CurrentFrame)), 65, 76));
            break;
        }
        }
        x += dx * time;
        y += dy * time;
    }
    if (health <= 0) 
        life = false;

    speed = 0;
    sprite.setPosition(x, y);
    interactionWithMap();
}

class Bullet :public Entity {
public:
    float CurrentFrame = 0;
    Bullet(Image& image, Level& lvl, const  float &X, const  float &Y, const  int &W, const int &H, const  enum state &EntityDir, const  String &Name);
    void change(const float &time);
    bool checkCollisionWithEnemies(std::list<Entity*>& entities);
    bool checkCollisionWithPlayer(Player& player);
    Sprite CheckLife(const float time, Sprite spr);
};


Bullet::Bullet(Image& image, Level& lvl, const  float& X, const  float& Y, const  int& W, const int& H, const  enum state& EntityDir, const  String& Name) :Entity(image, X, Y, W, H, Name, lvl){
    this->dir = EntityDir;

    if (name == "Bullet")
    {
        switch (dir)
        {
        case left:
        {
            x = X - 22;
            y = Y - 10;
            break;
        }
        case right: {
            x = X + 22;
            y = Y - 10;
            break;
        }
        case up: {
            x = X - 14;
            y = Y - 22;
            break;
        }
        case down: {
            x = X - 14;
            y = Y + 22;
            break;
        }
        }
    }
    if (name == "BulletE") {
        switch (dir)
        {
        case left:
        {
            x = X - 20;
            y = Y + 30;
            break;
        }
        case right: {
            x = X + 20;
            y = Y + 30;
            break;
        }
        case up: {
            x = X + 30;
            y = Y - 20;
            break;
        }
        case down: {
            x = X + 30;
            y = Y + 20;
            break;
        }
        }
    }
    this->speed = 0.8;
    w = h = 16;
    life = true;
    sprite.setTextureRect(IntRect(0, 0, 21, 21));
}

Sprite Bullet::CheckLife(const float time, Sprite spr) {
    if (CurrentFrame < 2) {
        CurrentFrame += 0.005 * time;
        switch ((int)CurrentFrame) {
        case 0: {
            spr.setTextureRect(IntRect(6, 15, 56, 50));
            break;
        }
        case 1: {
            spr.setTextureRect(IntRect(64 * (int)CurrentFrame, 6, 62, 60));
            break;
        }
        case 2: {
            spr.setTextureRect(IntRect(64 * (int)CurrentFrame, 0, 70, 70));
            break;
        }
        }

        switch (this->dir) {
        case left: {
            spr.setPosition(x + w / 2 - 5, y - h / 2 - 10);
            break;
        }
        case right: {
            spr.setPosition(x - w / 2 - 23, y - h / 2 - 10);
            break;
        }
        case up: {
            spr.setPosition(x - w / 2 - 10, y - h / 2 - 10);
            break;
        }
        case down: {
            spr.setPosition(x - w / 2 - 10, y - h / 2 - 23);
            break;
        }
        }
    }
    return spr;
}

void Bullet::change(const float &time) {
    switch (dir) {
    case left: dx = -this->speed; dy = 0; break;
    case right: dx = this->speed; dy = 0; break;
    case up: dx = 0; dy = -this->speed; break;
    case down: dx = 0; dy = this->speed; break;
    }

    x += dx * time;
    y += dy * time;

    for (int i = 0; i < obj.size(); i++) {
        if (getRect().intersects(obj[i].rect) && obj[i].name=="solid") {
            life = false;
        }
    }
    sprite.setPosition(x, y);
}

bool Bullet::checkCollisionWithEnemies(std::list<Entity*>& entities) {
    bool flag{ false };
    for (auto& entity : entities) {
        if ((entity->GetName() == "Enemy" || entity->GetName() == "Enemypro") && getRect().intersects(entity->getRect())) {
            entity->SetHealth(entity->GetHealth()-50);
            life = false;
            flag = true;
        }
    }
    return flag;
}

bool Bullet::checkCollisionWithPlayer(Player& player) {
    if (getRect().intersects(player.getRect())) {
        player.SetHealth(player.GetHealth()-25);
        life = false;
        return true;
    }
    return false;
}

class Enemy :public Entity {
    float CurrentFrame;
    float shootCooldown;
    bool isShoot;
    float moveTimer;
    int currentState;
    //bool isSolidBlocking(float x, float y, float width, float height) const;
public:
    Enemy(const Image& image,const String &Name, Level& lvl, const float &X, const float &Y, const int &W, const int &H);
    bool checkCollisionWithMap();
    void change(const float &time);
    void SetMoveTimer(float moveTimer) { this->moveTimer = moveTimer; };
    bool canShoot();
    void shoot(std::list<Entity*>& entities, Image& BulletImage, Level& lvl);
    void checkPlayerPosition();
    void followPlayer(const Player &pl);
    void AIcreator(const float& time, const Player& pl);
};

Enemy::Enemy(const Image& image, const String& Name, Level& lvl, const float& X, const float& Y, const int& W, const int& H) :Entity(image, X, Y, W, H, Name, lvl), isShoot{ false }, shootCooldown{}, CurrentFrame{}, currentState{}{
    if (name == "Enemy" || name=="Enemypro") {
        sprite.setTextureRect(IntRect(0, 0, w, h));
        speed = 0.1;
        dx = speed;
        moveTimer = 0;
    }
}

//bool isSolidBlocking(float x, float y, float width, float height) const {
//    for (const auto& obj : obj) {
//        if (obj.name == "solid" &&
//            (y <= obj.rect.getPosition().y && y + height >= obj.rect.getPosition().y) &&
//            (x + width > obj.rect.getPosition().x && x - width < obj.rect.getPosition().x + obj.rect.getSize().x)) {
//            return true;
//        }
//    }
//    return false;
//}

void Enemy::followPlayer(const Player &pl) {
    bool check = false;
    if (this->x < pl.GetX() + pl.GetW() / 2 && this->x + this->w > pl.GetX() + pl.GetW() / 2 && this->y > pl.GetY()) {
        for (int i = 0; i < obj.size(); i++) {
            if ((obj[i].name == "solid" && (pl.GetY() - pl.GetH() / 2 <= obj[i].rect.getPosition().y && this->y + this->h / 2 >= obj[i].rect.getPosition().y) && (this->x + this->GetW() / 5 > obj[i].rect.getPosition().x && this->x - this->GetW() / 5 < obj[i].rect.getPosition().x + obj[i].rect.getSize().x))) {
                check = true;
                break;
            }
        }
        if (!check) {
            dir = up;
            speed = 0;
            dx = 0;
            dy = 0;
            moveTimer = 0;
            isShoot = true;
        }
    } else if (this->x < pl.GetX() + pl.GetW() / 2 && this->x + this->w > pl.GetX() + pl.GetW() / 2 && this->y < pl.GetY()) {
        for (int i = 0; i < obj.size(); i++) {
            if ((obj[i].name == "solid" && (pl.GetY() + pl.GetH() / 2 >= obj[i].rect.getPosition().y && this->y - this->h / 2 <= obj[i].rect.getPosition().y) && (this->x + this->GetW() / 5 > obj[i].rect.getPosition().x && this->x - this->GetW() / 5 < obj[i].rect.getPosition().x + obj[i].rect.getSize().x))) {
                check = true;
                break;
            }
        }
        if (!check) {
            dir = down;
            speed = 0;
            dx = 0;
            dy = 0;
            moveTimer = 0;
            isShoot = true;
        }
    } else if (this->y < pl.GetY() + pl.GetH() / 2 && this->y + this->h > pl.GetY() + pl.GetH() / 2 && this->x < pl.GetX()) {
        for (int i = 0; i < obj.size(); i++) {
            if ((obj[i].name == "solid" && (this->y - this->h / 5 >= obj[i].rect.getPosition().y && this->y + this->h / 5 <= obj[i].rect.getPosition().y + obj[i].rect.getSize().y) && (pl.GetX() + pl.GetW() / 2 >= obj[i].rect.getPosition().x && this->x - this->w / 2 <= obj[i].rect.getPosition().x))) {
                check = true;
                break;
            }
        }
        if (!check) {
            dir = right;
            speed = 0;
            dx = 0;
            dy = 0;
            moveTimer = 0;
            isShoot = true;
        }
    } else if (this->y < pl.GetY() + pl.GetH() / 2 && this->y + this->h > pl.GetY() + pl.GetH() / 2 && this->x > pl.GetX()) {
        for (size_t i = 0; i < obj.size(); ++i) {
            if ((obj[i].name == "solid" && (this->y - this->h / 5 >= obj[i].rect.getPosition().y && this->y + this->h / 5 <= obj[i].rect.getPosition().y + obj[i].rect.getSize().y) && (pl.GetX() - pl.GetW() / 2 <= obj[i].rect.getPosition().x && this->x + this->w / 2 >= obj[i].rect.getPosition().x))) {
                check = true;
                break;
            }
        }
        if (!check) {
            dir = left;
            speed = 0;
            dx = 0;
            dy = 0;
            moveTimer = 0;
            isShoot = true;
        }
    } else {
        speed = 0.1;
        isShoot = false;
    }
} 


bool Enemy::checkCollisionWithMap() 
{
    bool flag=false;
    for (int i = 0; i < obj.size(); i++) 
        if (getRect().intersects(obj[i].rect)) 
        {
            if (obj[i].name == "solid" || obj[i].name == "stop")
            {
                flag = true;
                if (dy > 0) {
                     y = obj[i].rect.top - h; 
                     if (name == "Enemy") {
                         dir = up; 
                         dy = -this->speed;  
                     }
                break; }
                if (dy < 0) { y = obj[i].rect.top + obj[i].rect.height; 
                if (name == "Enemy") 
                {
                    dir = down;
                    dy = this->speed;
                }
                break; }
                if (dx > 0) {
                   // dir = left;
                    x = obj[i].rect.left - w;
                    if (name == "Enemy") {
                        dir = left;
                        dx = -this->speed;
                    }
                  //  dx = -this->speed;
                    break;
                }
                if (dx < 0) {
                    if (name == "Enemy")
                    {
                        dir = right;
                        dx = this->speed;
                    }
                    x = obj[i].rect.left + obj[i].rect.width;
                   // dx = this->speed;
                    break;
                }
            }
        }
    return flag;
}

bool Enemy::canShoot() {
    if (shootCooldown >= 2000 && isShoot) {
        shootCooldown = 0; 
        return true;
    }
    return false;
}

void Enemy::AIcreator(const float& time, const Player& pl) {
    std::srand(std::time(NULL));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 9);
    float randomChange = dist(gen) * 0.001;
    const int stateThreshold = 5;

    if (moveTimer <= 10) {
        moveTimer += randomChange * time;
    }
    else if (speed > 0) {
        int differenceInX = pl.GetX() - this->x;
        int differenceInY = pl.GetY() - this->y;
        std::uniform_int_distribution<int> dirDist(0, 1);
        int randomDir = dirDist(gen);

        if (differenceInX > 0 && differenceInY > 0) {
            dir = randomDir == 0 ? right : down;
            if (currentState >= stateThreshold) {
                dir = randomDir == 0 ? left : up;
                currentState = 0;
            }
        }
        else if (differenceInX < 0 && differenceInY > 0) {
            dir = randomDir == 0 ? left : down;
            if (currentState >= stateThreshold) {
                dir = randomDir == 0 ? right : up;
                currentState = 0;
            }
        }
        else if (differenceInX > 0 && differenceInY < 0) {
            dir = randomDir == 0 ? right : up;
            if (currentState >= stateThreshold) {
                dir = randomDir == 0 ? left : down;
                currentState = 0;
            }
        }
        else if (differenceInX < 0 && differenceInY < 0) {
            dir = randomDir == 0 ? left : up;
            if (currentState >= stateThreshold) {
                dir = randomDir == 0 ? right : down;
                currentState = 0;
            }
        }

        moveTimer = 0;
    }
}

void Enemy::change(const float &time) {
    if (speed>0) {
        CurrentFrame += 0.003 * time;
    }
    if (CurrentFrame >= 7) CurrentFrame = 0;
    //else {
    //    CurrentFrame = 0;
    //}
        if (name == "Enemy" || name=="Enemypro") {
            switch (dir) {
            case right: {
                sprite.setTextureRect(IntRect(89 * (int)CurrentFrame + 4, 3, 83, 72));
                dx = speed;
                dy = 0;
                break;
            }
            case left: {
                sprite.setTextureRect(IntRect(89 * (int)CurrentFrame + 1, 78, 83, 72));
                dx = -speed;
                dy = 0;
                break;
            }
            case down: {
                sprite.setTextureRect(IntRect(90, 154+90 * (int)CurrentFrame, 72, 83));
                dy = speed;
                dx = 0;
                break;
            }
            case up: {
                sprite.setTextureRect(IntRect(0, 154 + 90 * (int)CurrentFrame, 72, 83));
                dy = -speed;
                dx = 0;
                break;
            }
            }
            x += dx * time;
            y += dy * time;
            sprite.setPosition(x, y);
            if (health <= 0) {
                life = false;
            }
            if (shootCooldown <= 2000)
                shootCooldown += time * 0.3;
            if (checkCollisionWithMap()) {
                if (moveTimer < 2)
                    currentState++;
                moveTimer = 11;
            }
        }
    }

void handleCollisionsWithEnemies(Entity* enemy, const std::list<Entity*>& entities) {
    for (auto it2 = entities.begin(); it2 != entities.end(); ++it2) {
        Entity* c = *it2;
        if (c != enemy && (c->GetName() == "Enemy" || c->GetName()=="Enemypro")) {
            Enemy* otherEnemy = static_cast<Enemy*>(c);
            enemy->handleCollision(*otherEnemy);
        }
    }
}


void Enemy::shoot(std::list<Entity*>& entities, Image& BulletImage, Level& lvl) {
    if (canShoot()) {
        entities.push_back(new Bullet(BulletImage, lvl, x, y, 21, 21, dir, "BulletE"));
    }
}

Level loadLevel(const std::string& filename) {
    Level lvl;
    lvl.LoadFromFile(filename);
    return lvl;
}

bool duoGame(RenderWindow& window) {

    Level lvl{ loadLevel("mappy.tmx") };
    lvl.LoadFromFile("mappy.tmx");

    Image BulletImage = loadImage("images/map.png", Color::White);

    SoundBuffer shootBuffer, shootBuffer1;
    Sound shoot{ loadShoot("shoot1.ogg", shootBuffer) };
    Sound shoot1 {loadShoot("pipka.ogg", shootBuffer1)};

    std::list<Entity*>  entities;
    std::list<Entity*>::iterator it;
    std::list<Entity*>  entitiesdie;
    std::list<Entity*>::iterator itd;

    Image img {loadImage("images/bulletdie.png", Color::White)};
    Texture texture;
    texture.loadFromImage(img);
    Sprite spr;
    spr.setTexture(texture);

    LifeBar lifeBarPlayer1;
    LifeBar lifeBarPlayer2;

    String namePl1{ "Player1" };
    String namePl2 { "Player2"};

    //Music music;
    //music.openFromFile("music.ogg");
    //music.play();

    Image img1 = loadImage("images/tanks.png", Color::White);
    Object player = lvl.GetObject("player");
    Player player1(img1, lvl, player.rect.left, player.rect.top, 76, 62, namePl1);

    View view;
    view.setCenter(970, 970);
    view.setSize(window.getSize().x, window.getSize().y*1.8 );
    window.setView(view);

    Image img2= loadImage("images/tanks.png", Color::White);
    Object playerenemy= lvl.GetObject("player2");
    Player player2(img2, lvl, playerenemy.rect.left, playerenemy.rect.top, player.rect.width, player.rect.height, namePl2);

    static int score1{}, score2{};
    player1.SetScore(score1);
    player2.SetScore(score2);

    Texts text1, text2, Score1, Score2;
    text1.SetText("Score: ", Color(50,45,77), Vector2f(1.7, 1.7), Vector2f(100, 140));
    text2.SetText("Score: ", Color(50, 45, 77), Vector2f(1.7, 1.7), Vector2f(1200, 140));
    std::vector<std::thread> thr;

    Clock clock;
    while (window.isOpen()) {
        float time = clock.getElapsedTime().asMicroseconds();
        clock.restart();
        time = time / 800;

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        for (auto& it : entities) {
            (it)->change(time);
        }

        for (it = entities.begin(); it != entities.end();) {
            Entity* b = *it;
            b->change(time);
            if (b->GetName() == "Bullet") {
                if (static_cast<Bullet*>(b)->checkCollisionWithPlayer(player1))player2.SetScore(player2.GetScore() + 25);
                if(static_cast<Bullet*>(b)->checkCollisionWithPlayer(player2))player1.SetScore(player1.GetScore() + 25);
            }
            if (b->GetLife() == false) {
                entitiesdie.push_back(b);
                it = entities.erase(it);
            }
            else it++;
        }

        for (itd = entitiesdie.begin(); itd != entitiesdie.end();) {
            if ((*itd)->GetName() == "Bullet") {
                if(static_cast<Bullet*>(*itd)->CurrentFrame==0)
                    shoot1.play();
                if (static_cast<Bullet*>(*itd)->CurrentFrame >= 2) {
                    delete (*itd); 
                    itd = entitiesdie.erase(itd); 
                }
                else {
                    ++itd; 
                }
            }
            else {
                delete (*itd); 
                itd = entitiesdie.erase(itd); 
            }
        }



        player1.change(time);
        player2.change(time);
        player1.handleCollision(player2);

        if (player1.GetIsShoot() == true) {
            player1.SetIsShoot(false);
            entities.push_back(new Bullet(BulletImage, lvl, player1.GetX() + player1.GetW() / 2.0f, player1.GetY() + player1.GetH() / 2.0f, 21, 21, player1.GetState(), "Bullet"));
            shoot.play();
        }
        if (player2.GetIsShoot() == true) {
            player2.SetIsShoot(false);
            entities.push_back(new Bullet(BulletImage, lvl, player2.GetX() + player1.GetW() / 2.0f, player2.GetY() + player1.GetH() / 2.0f, 21, 21, player2.GetState(), "Bullet"));
            shoot.play();
        }

        lifeBarPlayer1.update(player1.GetHealth());
        lifeBarPlayer2.update(player2.GetHealth());
        Score1.SetText(std::to_string(player1.GetScore()), Color(20, 18, 50), Vector2f(1.7, 1.7), Vector2f(350, 140)); score1 = player1.GetScore();
        Score2.SetText(std::to_string(player2.GetScore()), Color(20, 18, 50), Vector2f(1.7, 1.7), Vector2f(1450, 140)); score2 = player2.GetScore();
        window.clear();
        lvl.Draw(window);
        lifeBarPlayer1.draw(window, namePl1);
        lifeBarPlayer2.draw(window, namePl2);
        window.draw(player1.GetSprite());
        for (auto& it : entities) {
            window.draw((it)->GetSprite());
        }
        window.draw(player2.GetSprite());
        for (auto& itd : entitiesdie) {
            window.draw(static_cast<Bullet*>(itd)->CheckLife(time, spr));
        }
        window.draw(text1.GetText());
        window.draw(text2.GetText());
        window.draw(Score2.GetText());
        window.draw(Score1.GetText());
        window.display();
        if (Keyboard::isKeyPressed(Keyboard::Tab))
            return false;
        if (player1.GetLife() == false) {
            score2+= 250;
            return true;
        }
            if (player2.GetLife() == false) {
                score1 += 250;
                return true;
        }
    }
}


bool startGame(RenderWindow& window) {
    view.reset(FloatRect(1000.f, 1000.f, 300.f, 200.f));
    view.setSize(window.getSize().x, window.getSize().y * 1.8);
    Level lvl;
    lvl.LoadFromFile("topmappy.tmx");
   // window.setFramerateLimit(60);
    String namePl1{ "Player1" };
    String nameOfEn{ "Enemy" };

    Image img1= loadImage("images/tanks.png", Color::White);
    Object player = lvl.GetObject("player");
    Player player1(img1, lvl, player.rect.left, player.rect.top, player.rect.width, player.rect.height, namePl1);

    Image img = loadImage("images/bulletdie.png", Color::White);
    Texture texture;
    texture.loadFromImage(img);
    Sprite spr;
    spr.setTexture(texture);

    Image easyEnemyImage= loadImage("images/enemy.png", Color::White);
    Image proEnemyImage = loadImage("images/enemypro.png", Color::White);

    LifeBar lifeBarPlayer1;

    Texts text1,Score1;
    Score1.SetText(std::to_string(player1.GetScore()), Color(20, 18, 50), Vector2f(1.5, 1.5), Vector2f(window.getView().getCenter().x + 350, window.getView().getCenter().y - 400));
    text1.SetText("Score: ", Color(50, 45, 77), Vector2f(1.5, 1.5), Vector2f(window.getView().getCenter().x + 120, window.getView().getCenter().y - 400));

    Image BulletImage = loadImage("images/map.png", Color::White);

    std::list<Entity*>  entities;
    auto it=entities.begin();
    std::list<Entity*>  entitiesdie;
    std::list<Entity*>::iterator itd;
    std::list<Entity*>::iterator it2;
    std::vector<PickUP> healthlist;

    std::vector<Object> easy = lvl.GetObjects(nameOfEn);
    for (size_t i = 0; i < easy.size(); ++i)
        entities.push_back(new Enemy(easyEnemyImage, nameOfEn, lvl, easy[i].rect.left, easy[i].rect.top, easy[i].rect.width, easy[i].rect.height));
    std::vector<Object> pro = lvl.GetObjects(nameOfEn+"pro");
    for (size_t i = 0; i < pro.size(); ++i)
        entities.push_back(new Enemy(proEnemyImage, nameOfEn+ "pro", lvl, pro[i].rect.left, pro[i].rect.top, pro[i].rect.width, pro[i].rect.height));

    Music music;
    music.openFromFile("music.ogg");
    music.play();

    SoundBuffer shootBuffer, shootBuffer1;
    Sound shoot = loadShoot("shoot1.ogg", shootBuffer);
    Sound shoot1 = loadShoot("pipka.ogg", shootBuffer1);
    ThreadPool pool(50);
    std::queue<std::future<void>> que;

    Clock clock;
    while (window.isOpen()) {
        float time = clock.getElapsedTime().asMicroseconds();
        clock.restart();
        time = time / 500;
        sf::Event event;
       // KdOfEnemyTime += 30;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
            for (it = entities.begin(); it != entities.end();) {
                Entity* b = *it;
                b->change(time);
                //    });
                if (b->GetName() == nameOfEn || b->GetName() == "Enemypro") {
                    Enemy* enemy = static_cast<Enemy*>(b);
                    if (b->GetName() == "Enemypro")
                        static_cast<Enemy*>(enemy)->AIcreator(time, player1);
                        handleCollisionsWithEnemies(enemy, entities);
                     player1.handleCollision(*enemy);
                     pool.enqueue([&enemy, &player1] {
                    enemy->followPlayer(player1);
                       });
                    enemy->shoot(entities, BulletImage, lvl);
                    while (!que.empty()) {
                        auto& future = que.front();
                        if (future.valid()) {
                            future.get();
                        }
                        que.pop();
                    }
                }

                if (b->GetName() == "Bullet") {
                    //  static_cast<Bullet*>(b)->checkCollisionWithEnemies(entities);
                    if (static_cast<Bullet*>(b)->checkCollisionWithEnemies(entities))
                        player1.SetScore(player1.GetScore() + 25);
                }
                if (b->GetName() == "BulletE") {
                    static_cast<Bullet*>(b)->checkCollisionWithPlayer(player1);
                }
                if (b->GetLife() == false) {
                    entitiesdie.push_back(b);
                    it = entities.erase(it);
                }
                else it++;

            }
            //   });

            for (itd = entitiesdie.begin(); itd != entitiesdie.end();) {
                if ((*itd)->GetName() == "Bullet" || (*itd)->GetName() == "BulletE") {
                    if (static_cast<Bullet*>(*itd)->CurrentFrame == 0 && (*itd)->GetName() == "Bullet")
                        shoot1.play();
                    if (static_cast<Bullet*>(*itd)->CurrentFrame >= 2) {
                        delete (*itd);
                        itd = entitiesdie.erase(itd);
                    }
                    else {
                        ++itd;
                    }
                }
                else {
                    if ((*itd)->GetName() == nameOfEn || (*itd)->GetName() == nameOfEn + "pro") {
                        std::random_device rd;
                        std::mt19937 gen(rd());
                        std::uniform_int_distribution<int> dist(0, 2);
                        int randomNum = dist(gen);
                        if (randomNum == 1) {
                            healthlist.push_back(PickUP("images/coin.png", Vector2f((*itd)->GetX(), (*itd)->GetY()), 1));
                            std::uniform_int_distribution<int> coin(10, 40);
                            healthlist[healthlist.size() - 1].SetValue(coin(gen));
                        }
                        if (randomNum == 0) {
                            healthlist.push_back(PickUP("images/healthbuf.png", Vector2f((*itd)->GetX(), (*itd)->GetY()), 0));
                            std::uniform_int_distribution<int> health(10, 40);
                            healthlist[healthlist.size() - 1].SetValue(health(gen));
                        }
                    }
                    delete (*itd);
                    itd = entitiesdie.erase(itd);
                }
            }
                player1.CheckCollisionWithBuffs(healthlist);
                lifeBarPlayer1.update(player1.GetHealth());
                que.push(std::async(std::launch::async, [&] {
                    for (auto it = healthlist.begin(); it != healthlist.end(); ) {
                        it->update(time);
                        if (!it->isSpawned()) {
                            it = healthlist.erase(it);
                        }
                        else {
                            ++it;
                        }
                    }
                    }));                
        while (!que.empty()) { que.pop(); }

        if (player1.GetIsShoot() == true) {
            player1.SetIsShoot(false);
            entities.push_back(new Bullet(BulletImage, lvl, player1.GetX() + player1.GetW()/2.0f, player1.GetY() + player1.GetH() / 2.0f, 21, 21, player1.GetState(), "Bullet"));
            shoot.play();
        }
        player1.change(time);

        window.clear();
        window.setView(view);
        lvl.Draw(window);
        window.draw(player1.GetSprite());
            for (auto it : entities) {
                window.draw((it)->GetSprite());
            }
            for (auto& itd : entitiesdie) {
                window.draw(static_cast<Bullet*>(itd)->CheckLife(time, spr));
            }
            for (auto ith : healthlist) {
                window.draw(ith.getSprite());
            }
        lifeBarPlayer1.draw(window, namePl1);
        Score1.draw(window, "Score",std::to_string(player1.GetScore()));
        text1.draw(window, "Score name");
        std::cout << "time: " << time;
        window.display();
        if (player1.GetLife() == false) { return true; }
        if (Keyboard::isKeyPressed(Keyboard::Tab))
            return false;
    }

}

inline void gameRunning(RenderWindow& window) {
    if (startGame(window)) { gameRunning(window); }
}

inline void start(RenderWindow& window) {
    if (duoGame(window)) { start(window); }
}

inline void setFullScreen(sf::RenderWindow& window) {
    window.create(sf::VideoMode::getDesktopMode(), "First version of World of Tanks!", sf::Style::Fullscreen);
}


int main()
{
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "First version of World of Tanks!", sf::Style::Fullscreen);
    Image icon;
    icon.loadFromFile("images/bsticon.png");
    icon.createMaskFromColor(Color(255,255,255));
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    MenuChoice choose;
    choose=menu(window);
    while (1) {
        if (choose == SOLO)
            gameRunning(window);
        else if (choose == MULTI) {
            start(window);
        }
        else if (choose == EXIT)
            exit(-1);
        setFullScreen(window);
        choose=menu(window);
    }
}
