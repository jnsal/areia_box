#include <SFML/Graphics.hpp>
#include <math.h>
#include <iostream>
#include <vector>
#include <math.h>


#define TILES 40
#define TILE_SIZE 8
#define WIDTH 800
#define HEIGHT 600

enum tile_type
{
    None = -1,
    water,
    sand,
    dirt,
    wood,
};

sf::Color getColor(tile_type type)
{
    sf::Color color;
    switch (type)
    {
        case None:
            color = sf::Color::White;
            break;
        case water:
            color = sf::Color::Blue;
            break;
        case sand:
            color = sf::Color(194, 178, 128);
            break;
        case dirt:
            color = sf::Color::Cyan;
            break;
        case wood:
            color = sf::Color(200, 50, 100, 255);
            break;
        default:
            color = sf::Color::Blue;
            break;
    }
    return color;
}


enum elements
{
    nil = -1,
    solid,
    liquid,
    gas
};

struct v2 
{
    v2():
        x(0), y(0)
    {}

    v2(int _x, int _y):
        x(_x), y(_y)
    {}
    v2(sf::Vector2f v):
        x(v.x), y(v.y)
    {}

    int x, y;
    
    v2& operator=(const sf::Vector2f v)
    {
        this->x = v.x;
        this->y = v.y;
        return *this;
    }
};

struct Tile
{
    Tile()
    {
        moved = false;
    }
    Tile(int x, int y, tile_type _type = tile_type::None, elements _element = elements::nil)
    {
        pos.x = x;
        pos.y = y;
        type =_type;
        element = _element;
        moved = false;
    }
    void Update(const Tile& t)
    {
       pos = t.pos;
       type = t.type;
       element = t.element;
    }

    v2 pos;
    tile_type type;
    elements element;
    bool moved;
};

v2 getMousePosition(sf::RenderWindow* window)
{
    return v2(window->mapPixelToCoords(sf::Mouse::getPosition(*window)));
}

// HERE

// TODO: Make the relation between the colors and the tiles position, and add a mouse recognition
// NOTE: Not update the color don't know why
class Grid
{
    private:
    void change_tiles(int t1, int t2)
    {
        Tile temp;
        temp = tile[t1];
        tile[t1] = tile[t2];
        tile[t2] = temp;
        setColor(t1, tile[t1].type);
        setColor(t2, tile[t2].type);
    }
    void update_physics()
    {
        for (size_t i = 0; i < grid_range; ++i)
        {
            if (tile[i].type == tile_type::sand)
            {
                if (i <= grid_range - grid_size)
                {
                    Tile temp(tile[i]);
                    tile[i].Update(tile[i + grid_size]);
                    tile[i + grid_size].Update(temp);
                    setColor(i, tile[i].type);
                    setColor(i + grid_size, tile[i+grid_size].type);
                    // setColor(i + grid_size, tile[i].type);
                }
            }
        }
    }
    void update_colors()
    {
        for (size_t i = 0; i < grid_range; ++i)
        {
            if (image.getPixel(tile[i].pos.x * TILE_SIZE, tile[i].pos.y * TILE_SIZE) != getColor(tile[i].type))
            {
               setColor(i, tile[i].type);
            }
        }
    }
    public:
    Grid() = delete;
    Grid(uint8_t _grid_size):
        grid_size(_grid_size)
    {
        grid_range = grid_size * grid_size;
        tile = { new Tile[grid_range] };
        { // Set tile posisition default
            // int x = (WIDTH / 2) - ((grid_size * TILE_SIZE) / 2);
            // int y = HEIGHT - (grid_size * TILE_SIZE);
            int column = 0, row = 0;
            for (int i = 0; i < grid_size * grid_size; ++i)
            {
                tile[i].pos.x = column;
                tile[i].pos.y = row;

                if (column == grid_size - 1) {
                    column = 0;
                    row++;
                } else {
                    column++;
                }
            }
        }
        image.create(grid_size * TILE_SIZE, grid_size * TILE_SIZE, sf::Color::Blue);
        texture.loadFromImage(image);
        grid_spr.setTexture(texture);
        { // Center the grid.
            int x = (WIDTH / 2) - ((grid_size * TILE_SIZE) / 2);
            int y = HEIGHT - (grid_size * TILE_SIZE);
            grid_spr.setPosition(x, y);
        }
    }

    void setColor(unsigned int n, tile_type color)
    {
        int x = tile[n].pos.x * TILE_SIZE;
        int y = tile[n].pos.y * TILE_SIZE;
        for (int col = 0; col < TILE_SIZE; ++col)
        {
            for (int row = 0; row < TILE_SIZE; ++row)
            {
                image.setPixel(x + row, y + col, getColor(color));
            }
        }
        tile[n].type = color;
        texture.loadFromImage(image);
        grid_spr.setTexture(texture);
    }

    void Update(sf::RenderWindow* window)
    {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            v2 mousepos = getMousePosition(window);
            v2 grid_pos = grid_spr.getPosition();
            if ( (mousepos.x >= grid_pos.x && mousepos.x <= grid_pos.x + (grid_size * TILE_SIZE)) &&
                 (mousepos.y >= grid_pos.y && mousepos.y <= HEIGHT)) {
                int X = mousepos.x - grid_pos.x;
                int Y = mousepos.y - grid_pos.y;
                int n_x = X / TILE_SIZE;
                int n_y = Y / TILE_SIZE;
                
                unsigned int i = (unsigned int)floor((double)n_y * (double)grid_size + (double)n_x);
                if (i >= 0 && i < (unsigned int)grid_range) {
                    setColor(i, tile_type::sand);
                }
            }
        }

        update_physics();
        window->draw(grid_spr);
    }

    private:
    uint8_t grid_size; // size in tiles // TODO: Make it a v2, to not be a perfect square.
    size_t grid_range;
    sf::Texture texture;
    sf::Image image;
    sf::Sprite grid_spr;

    Tile *tile;
    // Use same logic of the sandbox.
};



int main(void)
{
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "AreiaBox");
    Grid grid(TILES);

    while (window.isOpen())
    {
        window.clear();

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        {
            grid.setColor(1585, tile_type::sand);
        }

        grid.Update(&window);
        window.display();
    }

    return 0;
}
