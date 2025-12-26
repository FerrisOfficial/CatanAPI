#include "displayActive.hpp"

#include <iostream>

#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>
#include <thread>
#include <chrono>

namespace {
	// Basic color mapping for resources
	sf::Color resourceColor(Resource r) {
		switch (r) {
			case Resource::Brick:  return sf::Color(178, 34, 34);   // firebrick
			case Resource::Lumber: return sf::Color(34, 139, 34);   // forest green
			case Resource::Wool:   return sf::Color(144, 238, 144); // light green
			case Resource::Grain:  return sf::Color(218, 165, 32);  // goldenrod
			case Resource::Ore:    return sf::Color(112, 128, 144); // slate gray
			default:               return sf::Color(222, 184, 135); // burlywood (desert)
		}
	}

	// Create a hexagon shape of given radius centered at (cx, cy)
	sf::ConvexShape makeHex(float cx, float cy, float radius, sf::Color fill, sf::Color outline) {
		sf::ConvexShape hex;
		hex.setPointCount(6);
		for (int i = 0; i < 6; ++i) {
			constexpr float PI = 3.14159265358979323846f;
			float angle = static_cast<float>(PI / 3.0f * i - PI / 6.0f); // flat-top
			float x = cx + radius * std::cos(angle);
			float y = cy + radius * std::sin(angle);
			hex.setPoint(i, sf::Vector2f(x, y));
		}
		hex.setFillColor(fill);
		hex.setOutlineThickness(2.f);
		hex.setOutlineColor(outline);
		return hex;
	}

	// Layout helpers for Catan 3-4-5-4-3 pattern
	struct RowLayout { int count; float y; };
	std::array<RowLayout,5> computeLayout(float startY, float rowGap, float centerX, float /*radius*/) {
		// counts per row: 3,4,5,4,3
		std::array<RowLayout,5> rows{};
		int counts[5] = {3,4,5,4,3};
		for (int i=0;i<5;++i) {
			rows[i].count = counts[i];
			rows[i].y = startY + i * rowGap;
		}
		return rows;
	}

	std::vector<sf::Vector2f> computeHexCenters(float width, float height, float radius) {
		// Add gentle gaps so tiles don't visually touch/overlap
		constexpr float HORIZ_GAP = 1.3f;   // >1 widens horizontal spacing slightly
		constexpr float VERT_GAP  = 1.05f;   // >1 widens vertical spacing slightly

		float hexHoriz = (3.f * radius / 2.f) * HORIZ_GAP;     // horizontal distance between centers
		float hexVert  = (std::sqrt(3.f) * radius) * VERT_GAP; // vertical distance between rows

		float centerX = width / 2.f;
		// Place rows centered vertically with a small top margin
		float totalHeight = 4.f * hexVert; // distance from row0 to row4
		float startY = (height - totalHeight) / 2.f; // top row y
		auto rows = computeLayout(startY, hexVert, centerX, radius);

		std::vector<sf::Vector2f> centers;
		centers.reserve(19);
		for (int i=0;i<5;++i) {
			int count = rows[i].count;
			// compute starting x such that row is centered
			float rowWidth = (count - 1) * hexHoriz;
			float startX = centerX - rowWidth / 2.f;
			for (int j=0;j<count;++j) {
				centers.emplace_back(startX + j * hexHoriz, rows[i].y);
			}
		}
		return centers;
	}
}

void Display::renderBoard(const Board::BoardState& boardState) {
	using namespace Board;
	const float tileRadius = 48.f;
	const unsigned winW = 900, winH = 750;

	sf::RenderTexture offscreen;
	if (!offscreen.create(winW, winH)) {
		std::cout << "Board State!!!! (failed to create render texture)" << std::endl;
		return;
	}
	offscreen.clear(sf::Color(30, 30, 30));

	auto centers = computeHexCenters(static_cast<float>(winW), static_cast<float>(winH), tileRadius);

	// Draw hex tiles
	for (size_t h = 0; h < HEX_COUNT && h < centers.size(); ++h) {
		auto hexPacked = boardState.hexes[h];
		auto res = Hex::unpackResource(hexPacked);
		auto num = Hex::unpackCatanNumber(hexPacked);
		auto [cx, cy] = centers[h];

		auto tile = makeHex(cx, cy, tileRadius, resourceColor(res), sf::Color::Black);
		offscreen.draw(tile);

		// robber marker
		if (boardState.robberPosition == h) {
			sf::CircleShape robber(tileRadius * 0.25f);
			robber.setFillColor(sf::Color::Black);
			robber.setOrigin(robber.getRadius(), robber.getRadius());
			robber.setPosition(cx, cy);
			offscreen.draw(robber);
		}

		// number token
		static sf::Font font;
		static bool fontLoaded = false;
		if (!fontLoaded) {
			// Try to load a common system font; if fails, skip text
			fontLoaded = font.loadFromFile("C:/Windows/Fonts/arial.ttf");
		}
		if (fontLoaded && num != 0) {
			sf::Text text;
			text.setFont(font);
			text.setString(std::to_string(num));
			text.setCharacterSize(20);
			text.setFillColor(sf::Color::Black);
			auto bounds = text.getLocalBounds();
			text.setOrigin(bounds.width/2.f, bounds.height);
			text.setPosition(cx, cy + 6.f);
			offscreen.draw(text);
		}
	}

	offscreen.display();

	// Save to file and also show a short-lived window so the user sees it.
	sf::Texture texture = offscreen.getTexture();
	sf::Image image = texture.copyToImage();
	image.saveToFile("board_view.png");

	// Display window briefly
	sf::RenderWindow window(sf::VideoMode(winW, winH), "Catan Board", sf::Style::Titlebar | sf::Style::Close);
	window.clear(sf::Color(30, 30, 30));
	sf::Sprite sprite(texture);
	window.draw(sprite);
	window.display();
	std::this_thread::sleep_for(std::chrono::milliseconds(750));
	window.close();
}