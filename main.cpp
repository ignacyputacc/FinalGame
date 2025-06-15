#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>


//!!! Please change the path in line 83 before compiling
class Arrow {
public:
    Arrow(const sf::Texture& tex, const sf::Vector2f& pos, float rot) {
        sprite.setTexture(tex);
        sprite.setOrigin(tex.getSize().x/2, tex.getSize().y/2);
        sprite.setPosition(pos);
        sprite.setRotation(rot);
        float s = 70 / float(tex.getSize().y);
        sprite.setScale(s, s);
        float angleinRad = (rot - 90) * 3.14159f / 180;
        velocity.x = std::cos(angleinRad)*500;
        velocity.y = std::sin(angleinRad)*500;
    }

    void update(float dt) {
        sprite.move(velocity * dt);
    }



    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    const sf::FloatRect getBounds() {
        return sprite.getGlobalBounds();
    }

private:
    sf::Sprite    sprite;
    sf::Vector2f  velocity;
};

class Coin {
public:
    Coin(const sf::Texture& tex) {
        sprite.setTexture(tex);
        float s = 60 / float(tex.getSize().x);
        sprite.setScale(s, s);
        int x = std::rand() % (800 - 100) + 50;
        int y = std::rand() % (600 - 150) + 100;
        sprite.setPosition(float(x), float(y));
        lifeClock.restart();
    }

    bool expired() const {
        return lifeClock.getElapsedTime().asSeconds() > 5;
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(sprite);
    }

    const sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }

private:
    sf::Sprite sprite;
    sf::Clock  lifeClock;
};

int main() {
    std::srand(unsigned(std::time(NULL)));

    const int W = 800, H = 600;
    const int GAME_TIME   = 20;
    const int ALERT_TIME  = 10;
    const int rotateSpeed = 360; // deg/sec

    sf::RenderWindow window(sf::VideoMode(W,H), "Coin sniper");

    const std::string path = "C:/Users/szafr/Desktop/IE-Assignment/GameFinal/assets/";

    sf::Texture bowTex, arrowTex, appleTex, grassTextures[3];
    bowTex.loadFromFile(path + "bow.png");
    arrowTex.loadFromFile(path + "arrow1.png");
    appleTex.loadFromFile(path + "coin.png");
    grassTextures[0].loadFromFile(path + "grass1.png");
    grassTextures[1].loadFromFile(path + "grass2.png");
    grassTextures[2].loadFromFile(path + "grass3.png");

    sf::Font font;
    font.loadFromFile(path + "font2.otf");

    sf::Music bgMusic; bgMusic.openFromFile(path + "music.wav");
    bgMusic.setLoop(true);
    bgMusic.play();

    sf::SoundBuffer hitS, timeS, overS;
    hitS.loadFromFile(path + "hit.wav");
    timeS.loadFromFile(path + "time.wav");
    overS.loadFromFile(path + "gameover.wav");
    sf::Sound hitSound(hitS), timeSound(timeS), overSound(overS);

    sf::Sprite bow(bowTex);
    bow.setOrigin(bowTex.getSize().x/2, bowTex.getSize().y/2);
    bow.setPosition(W-50, 50);
    bow.setScale(80/float(bowTex.getSize().x), 80/float(bowTex.getSize().x));

    sf::Sprite background(grassTextures[0]);
    {
        auto gs = grassTextures[0].getSize();
        background.setScale(float(W)/gs.x, float(H)/gs.y);
    }

    sf::Text timerTxt("Time: 20", font, 34);
    timerTxt.setPosition(10,10);
    sf::Text scoreTxt("Score: 0", font, 34);
    scoreTxt.setPosition(10,50);

    std::vector<Arrow> arrows;
    std::vector<Coin> coins;
    int score = 0;
    bool timeAlerted = false;
    int bgIndex = 0;

    sf::Clock frameClock, gameClock, spawnClock, shootClock, blinkClock, bgClock;

    while (window.isOpen()) {
        float dt = frameClock.restart().asSeconds();


        //dynamic background (new frame every 0.6sec)
        if (bgClock.getElapsedTime().asSeconds() > 0.6f) {
            bgClock.restart();
            bgIndex = (bgIndex+1) % 3;
            background.setTexture(grassTextures[bgIndex]);
        }

        sf::Event eClose;
        while (window.pollEvent(eClose))
            if (eClose.type == sf::Event::Closed)
                window.close();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            bow.rotate(- rotateSpeed * dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            bow.rotate( rotateSpeed * dt);

        //no-spray
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)
            && shootClock.getElapsedTime().asSeconds() >= 0.3f)
        {
            shootClock.restart();
            arrows.emplace_back(arrowTex, bow.getPosition(), bow.getRotation());
        }

        if (spawnClock.getElapsedTime().asSeconds() > 1) {
            spawnClock.restart();
            coins.emplace_back(appleTex);
        }

        //update every arrow position
        for (int i=int(arrows.size())-1; i>=0; --i) {
            arrows[i].update(dt);

        }

        //check if coins present more than 5s || hit
        for (int i=int(coins.size())-1; i>=0; i--) {
            if (coins[i].expired()) {
                coins.erase(coins.begin()+i);
                continue;
            }
            bool hit = false;
            for (int j=0; j<int(arrows.size()); ++j) {
                if (coins[i].getBounds().intersects(arrows[j].getBounds())) {
                    hitSound.play();
                    score+=1;
                    coins.erase(coins.begin()+i);
                    arrows.erase(arrows.begin()+j);
                    hit = true;
                    break;
                }
            }
            if (hit) {
                continue;
            }
        }

        int timeLeft = GAME_TIME - int(gameClock.getElapsedTime().asSeconds());





        //blinker time alert
        if (timeLeft <= ALERT_TIME) {
            if(timeAlerted!= true) {
                timeSound.play();
                timeAlerted = true;
            }

            float elapsed = blinkClock.getElapsedTime().asSeconds();

            if (elapsed < 0.6f) {
                timerTxt.setFillColor(sf::Color::Red);
            } else if (elapsed < 1.2f) {
                timerTxt.setFillColor(sf::Color::Black);
            } else {
                blinkClock.restart();//for repeating the blink
            }
        } else {
            timerTxt.setFillColor(sf::Color::Black);
        }

        window.clear();
        window.draw(background);
        window.draw(bow);
        for (auto& arrow : arrows) {
            arrow.draw(window);
        }
        for (auto& coin : coins) {
            coin.draw(window);
        }
        timerTxt.setString("Time: " + std::to_string(timeLeft));
        scoreTxt.setString("Score: " + std::to_string(score));
        window.draw(timerTxt);
        window.draw(scoreTxt);
        window.display();


        if (gameClock.getElapsedTime().asSeconds() >= float(GAME_TIME)) {
            break;
        }

    }

    bgMusic.stop();
    overSound.play();

    sf::Text finalScore("Final Score: " + std::to_string(score), font, 48);
    finalScore.setFillColor(sf::Color::Black);
    {
        auto fsBounds = finalScore.getLocalBounds();
        finalScore.setOrigin(fsBounds.left+fsBounds.width/2, fsBounds.top+fsBounds.height/2);
        finalScore.setPosition(W/2, H/2);
    }
    while (window.isOpen()) {
        sf::Event eClose;
        while (window.pollEvent(eClose))
            if (eClose.type == sf::Event::Closed)
                window.close();
        window.clear(sf::Color::White);
        window.draw(finalScore);
        window.display();
    }

    return 0;
}
