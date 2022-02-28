#include "Lighting.h"
#include "QuadMap.h"
#include <chrono>
#include <iostream>


class worldObject
{
    virtual sf::FloatRect getBounds() const = 0;
    virtual void move(const sf::Vector2f delta) = 0;

public:

    bool tryMove(const sf::Vector2f delta, const quadMap& map)
    {
        auto currentBounds = getBounds();
        currentBounds.left += delta.x;
        currentBounds.top += delta.y;
        if (map.sample(currentBounds))
        {
            return false;
        }
        move(delta);
        return true;
    }
};

class player : public worldObject
{
    sf::RectangleShape body;
    std::reference_wrapper<light> view;

    sf::FloatRect getBounds() const final
    {
        return body.getGlobalBounds();
    }

    void move(const sf::Vector2f delta) final
    {
        body.move(delta);
    }

public:

    player(lightManager& lights) : view(lights.addLight())
    {
        body.setSize({ 10, 10 });
        body.setPosition(200, 200);
        body.setOrigin(5, 5);

        view.get().radius = 350;
        view.get().width = 0.8;
        view.get().colour = sf::Color::White;
    }

    void update(sf::RenderWindow& window)
    {
        const auto mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        view.get().direction = std::atan2f(
            mousePos.y - body.getPosition().y,
            mousePos.x - body.getPosition().x
        );
        view.get().pos = body.getPosition();
    }

    void draw(sf::RenderWindow& window) const
    {
        window.draw(body);
    }
};

void game()
{
    sf::RenderWindow window(sf::VideoMode(512, 512), "");
    window.setFramerateLimit(60);
    quadMap world;
    lightManager lights;
    lights.create({ 512,512 });

    player PC(lights);

    sf::Texture worldTex;
    worldTex.loadFromFile("../Resources/exCast.png");
    sf::Sprite worldObj;
    worldObj.setTexture(worldTex);

    world.build(worldTex.copyToImage());
    sf::Vector2f motion = { 0,0 };

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case(sf::Event::Closed):
                window.close();
                break;

            case(sf::Event::KeyPressed):
                if (event.key.code == sf::Keyboard::W)
                    motion.y = -5;
                if (event.key.code == sf::Keyboard::S)
                    motion.y = 5;
                if (event.key.code == sf::Keyboard::A)
                    motion.x = -5;
                if (event.key.code == sf::Keyboard::D)
                    motion.x = 5;
                break;
            case(sf::Event::KeyReleased):
                if (motion.y == -5)
                    motion.y = 0;
                if (motion.y == 5)
                    motion.y = 0;
                if (motion.x == -5)
                    motion.x = 0;
                if (motion.x == 5)
                    motion.x = 0;
                break;
            }
        }

        window.clear();

        PC.update(window);

        PC.tryMove(motion, world);

        window.draw(worldObj);

        lights.clear();
        lights.drawToShadowMap(worldObj);
        lights.render();

        window.draw(sf::Sprite(lights.get()), sf::BlendAdd);

        PC.draw(window);

        window.display();
    }

}

bool imageTest()
{
    sf::RenderWindow window(sf::VideoMode(512, 512), "");
    window.setFramerateLimit(60);
    quadMap map;

    sf::Sprite obj;
    sf::Texture objTex;
    objTex.loadFromFile("../Resources/exCast.png");
    obj.setTexture(objTex);

    map.build(objTex.copyToImage());

    sf::VertexArray line;
    line.setPrimitiveType(sf::PrimitiveType::Lines);
    line.append(sf::Vector2f{ 300, 300 });
    line.append(sf::Vector2f{ 300, 300 });

    unsigned int frameCounter = 0;
    const auto frameSplit = std::chrono::seconds(1);

    auto lastFrameCount = std::chrono::high_resolution_clock::now();

    sf::CircleShape areaCheck;
    areaCheck.setRadius(25);
    areaCheck.setOrigin(25, 25);
    areaCheck.setOutlineThickness(1);
    areaCheck.setFillColor(sf::Color::Transparent);

    uint16_t pathLim = 5000;

    while (window.isOpen())
    {
        const auto now = std::chrono::high_resolution_clock::now();
        if (now - lastFrameCount > frameSplit)
        {
            std::cout << frameCounter << '\n';
            frameCounter = 0;
            lastFrameCount = now + (now - lastFrameCount - frameSplit);
        }
        frameCounter++;

        const auto mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case(sf::Event::MouseButtonPressed):
                line[0].position = mousePos;
                break;
            case(sf::Event::Closed):
                window.close();
                return true;
                break;
            case(sf::Event::KeyPressed):
                if (event.key.code == sf::Keyboard::W)
                    pathLim++;
                if (event.key.code == sf::Keyboard::S)
                    pathLim--;
                std::cout << "::" << pathLim << '\n';
                break;
            }
        }

        window.clear();
        window.draw(obj);
        static const auto mapRender = map.cellRender();
        window.draw(mapRender);

        line[1].position = mousePos;
        if (!map.sample(line[0].position, line[1].position))
        {
            line[0].color = sf::Color::Green;
            line[1].color = sf::Color::Green;
        }
        else
        {
            line[0].color = sf::Color::Red;
            line[1].color = sf::Color::Red;
        }
        
        if (map.getRayIntersect(line[0].position, line[1].position))
            line[1].position = map.getRayIntersect(line[0].position, line[1].position).value();

        //window.draw(line);

        //nav.selectAdjacent(map, mousePos, window);

        areaCheck.setPosition(mousePos);
        if (map.sample(areaCheck.getPosition(), areaCheck.getRadius()))
        {
            areaCheck.setOutlineColor(sf::Color::Red);
        }
        else
        {
            areaCheck.setOutlineColor(sf::Color::Green);
        }
        //window.draw(areaCheck);

        window.display();
    }
    return false;
}

int main()
{
    game();
    if (imageTest())
        return 0;


    sf::RenderWindow window(sf::VideoMode(512, 512), "");
    window.setFramerateLimit(60);
    lightManager lights;

    light* userLight = nullptr;

    {
        light l;
        l.radius = 56;
        l.pos = { 256, 256 };
        l.colour = sf::Color{ 255, 0, 0, 180 };
        userLight = &lights.addLight(l);
    }
    lights.create({ 512, 512 });


    sf::Sprite obj;
    sf::Texture objTex;
    objTex.loadFromFile("../Resources/exCast.png");
    obj.setTexture(objTex);
    //obj.setOrigin(obj.getLocalBounds().width / 2, obj.getLocalBounds().height / 2);

    sf::RectangleShape rect;
    rect.setSize({ 50, 50 });
    rect.setPosition(400, 400);
    rect.setFillColor(sf::Color::Green);

    auto& current = obj;

    sf::Vector2f pos = { 300, 300 };
    sf::Vector2f motion = { 0, 0 };

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
			switch (event.type)
			{
			case(sf::Event::Closed):
				window.close();
				break;

            case(sf::Event::KeyPressed):
                if (event.key.code == sf::Keyboard::W)
                    motion.y = -5;
                if (event.key.code == sf::Keyboard::S)
                    motion.y = 5;
                if (event.key.code == sf::Keyboard::A)
                    motion.x = -5;
                if (event.key.code == sf::Keyboard::D)
                    motion.x = 5;
                break;
            case(sf::Event::KeyReleased):
                if (motion.y == -5)
                    motion.y = 0;
                if (motion.y == 5)
                    motion.y = 0;
                if (motion.x == -5)
                    motion.x = 0;
                if (motion.x == 5)
                    motion.x = 0;
                break;

            case(sf::Event::MouseWheelScrolled):
                userLight->radius += 5 * event.mouseWheelScroll.delta;
                break;

            case(sf::Event::MouseMoved):
                //userLight->pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                break;
			}
        }

        const sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        pos += motion;

        lights.clear();

        lights.drawToShadowMap(current);
        lights.render();

        userLight->pos = pos;
        userLight->direction = std::atan2f(mousePos.y - pos.y, mousePos.x - pos.x);

        window.clear(sf::Color{ 100, 100, 100, 255 });
        window.draw(current);
        sf::Sprite light(lights.get());
        window.draw(light, sf::BlendAdd);
        window.display();
    }

    return 0;
}