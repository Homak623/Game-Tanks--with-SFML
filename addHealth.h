#include <SFML/Graphics.hpp>

using namespace sf;

enum type{HILL,COIN};

class PickUP {
	int type;
	int value=25;
	bool Spawned=true;
	float SecondsSinceSpawn=0;
	float SecondsToLive = 10;
	Texture texture;
	Image img;
	Sprite sprite;
	Vector2f pos;
public:
	PickUP(String WayToImg,Vector2f pos,int type){
		img.loadFromFile(WayToImg);
		img.createMaskFromColor(Color::White);
		texture.loadFromImage(img);
		sprite.setTexture(texture);
		this->pos = pos;
		this->type = type;
		sprite.setPosition(pos.x, pos.y);
	}
	void Spawn(Vector2f pos);
	FloatRect getPosition() { return sprite.getGlobalBounds(); }
	Sprite& getSprite() { 
		sprite.setTexture(texture);
		sprite.setPosition(pos.x, pos.y);
		return sprite; }
	//enum type& GetBuff() { return buffs; }
	//void SetBuf(enum type buff) { this->buffs = buff; 
	void SetType(int type) { this->type = type; }
	int GetType() { return type; }
	void update(float elapsedTime);
	bool& isSpawned() { return Spawned; }
	void SetValue(int value) { this->value=value; }
	int GetValue(){ return value; }
};

//void PickUP::Spawn(Vector2f pos) {
//	img.loadFromFile("images/exit.png");
////	img.createMaskFromColor(Color::White);
//	texture.loadFromImage(img);
//	this->sprite.setTexture(texture); // Установите текстуру для спрайта
//	//sprite.setTextureRect(IntRect(0, 0, 60, 60));
//	this->sprite.setPosition(pos.x, pos.y);
//}

void PickUP::update(float elapsedTime) {
	SecondsSinceSpawn += elapsedTime/800;
	if (SecondsSinceSpawn >= SecondsToLive)
		Spawned = false;
}
//bool SpawnHP() {
//    Image img = loadImage("images/healthbuf.png", Color::White);
//    Texture texture;
//    texture.loadFromImage(img);
//    Sprite spr;
//    spr.setTexture(texture);
//    window
//    return true;
//}