#include "World.h"
#include <tinyxml2.h>
#include <filesystem>
#include <sstream>
#include <string>
#include <cstdint>
#include <algorithm>
#include <cctype>
#include <iostream>

namespace
{
	std::string trim(const std::string& s)
	{
		const auto begin = std::find_if_not(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c) != 0; });
		const auto end = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c) { return std::isspace(c) != 0; }).base();

		if (begin >= end)
			return {};

		return std::string(begin, end);
	}

	std::vector<std::uint32_t> parseCsvGids(const std::string& csv)
	{
		std::vector<std::uint32_t> gids;
		std::stringstream ss(csv);
		std::string token;

		while (std::getline(ss, token, ','))
		{
			const std::string t = trim(token);
			if (t.empty())
				continue;

			gids.push_back(static_cast<std::uint32_t>(std::stoul(t)));
		}

		return gids;
	}

	void appendTileQuad(sf::VertexArray& va, float x, float y, float w, float h, float u, float v, float tw, float th)
	{
		sf::Vertex a;
		a.position = { x, y };
		a.texCoords = { u, v };

		sf::Vertex b;
		b.position = { x + w, y };
		b.texCoords = { u + tw, v };

		sf::Vertex c;
		c.position = { x + w, y + h };
		c.texCoords = { u + tw, v + th };

		sf::Vertex d;
		d.position = { x, y + h };
		d.texCoords = { u, v + th };

		va.append(a);
		va.append(b);
		va.append(c);

		va.append(a);
		va.append(c);
		va.append(d);
	}
}

void World::clear()
{
	m_layers.clear();
	m_colliders.clear();
	m_mapTileCount = { 0, 0 };
	m_tileSize = { 0, 0 };
	m_pixelSize = { 0, 0 };
	m_firstGid = 1;
	m_tilesetColumns = 0;
}

bool World::loadFromTmx(const std::string& tmxPath)
{
	clear();

	tinyxml2::XMLDocument doc;
	if (doc.LoadFile(tmxPath.c_str()) != tinyxml2::XML_SUCCESS)
		return false;

	tinyxml2::XMLElement* mapElem = doc.FirstChildElement("map");
	if (!mapElem)
		return false;

	unsigned int mapWidth = 0;
	unsigned int mapHeight = 0;
	unsigned int tileWidth = 0;
	unsigned int tileHeight = 0;

	mapElem->QueryUnsignedAttribute("width", &mapWidth);
	mapElem->QueryUnsignedAttribute("height", &mapHeight);
	mapElem->QueryUnsignedAttribute("tilewidth", &tileWidth);
	mapElem->QueryUnsignedAttribute("tileheight", &tileHeight);

	if (mapWidth == 0 || mapHeight == 0 || tileWidth == 0 || tileHeight == 0)
		return false;

	m_mapTileCount = { mapWidth, mapHeight };
	m_tileSize = { tileWidth, tileHeight };
	m_pixelSize = { mapWidth * tileWidth, mapHeight * tileHeight };

	tinyxml2::XMLElement* tilesetElem = mapElem->FirstChildElement("tileset");
	if (!tilesetElem)
		return false;

	tilesetElem->QueryUnsignedAttribute("firstgid", &m_firstGid);

	const std::filesystem::path mapPath = std::filesystem::path(tmxPath).parent_path();

	// Support both embedded tilesets and external TSX tilesets
	tinyxml2::XMLDocument tsxDoc;
	tinyxml2::XMLElement* tilesetRoot = tilesetElem;
	std::filesystem::path imageBasePath = mapPath;

	if (const char* tsxSource = tilesetElem->Attribute("source"))
	{
		const std::filesystem::path tsxPath = mapPath / tsxSource;
		if (tsxDoc.LoadFile(tsxPath.string().c_str()) != tinyxml2::XML_SUCCESS)
			return false;

		tilesetRoot = tsxDoc.FirstChildElement("tileset");
		if (!tilesetRoot)
			return false;

		imageBasePath = tsxPath.parent_path();
	}

	tinyxml2::XMLElement* imageElem = tilesetRoot->FirstChildElement("image");
	if (!imageElem)
		return false;

	const char* sourceAttr = imageElem->Attribute("source");
	if (!sourceAttr)
		return false;

	const std::filesystem::path imagePath = imageBasePath / sourceAttr;

	if (!m_tilesetTexture.loadFromFile(imagePath.string()))
		return false;

	m_tilesetTexture.setSmooth(false);

	const sf::Vector2u texSize = m_tilesetTexture.getSize();
	m_tilesetColumns = texSize.x / tileWidth;
	if (m_tilesetColumns == 0)
		return false;

	for (tinyxml2::XMLElement* layerElem = mapElem->FirstChildElement("layer");
		layerElem != nullptr;
		layerElem = layerElem->NextSiblingElement("layer"))
	{
		TileLayer layer;
		if (const char* name = layerElem->Attribute("name"))
			layer.name = name;

		tinyxml2::XMLElement* dataElem = layerElem->FirstChildElement("data");
		if (!dataElem)
			continue;

		const char* encoding = dataElem->Attribute("encoding");
		if (!encoding || std::string(encoding) != "csv")
			continue;

		const char* csvText = dataElem->GetText();
		if (!csvText)
			continue;

		std::vector<std::uint32_t> gids = parseCsvGids(csvText);
		const std::size_t expectedCount = static_cast<std::size_t>(mapWidth) * static_cast<std::size_t>(mapHeight);
		if (gids.size() < expectedCount)
			gids.resize(expectedCount, 0);

		constexpr std::uint32_t flipMask = 0xE0000000u;

		for (std::size_t i = 0; i < expectedCount; ++i)
		{
			const std::uint32_t rawGid = gids[i];
			const std::uint32_t gid = rawGid & ~flipMask;
			if (gid == 0 || gid < m_firstGid)
				continue;

			const std::uint32_t localId = gid - m_firstGid;
			const std::uint32_t tu = localId % m_tilesetColumns;
			const std::uint32_t tv = localId / m_tilesetColumns;

			const std::uint32_t x = static_cast<std::uint32_t>(i % mapWidth);
			const std::uint32_t y = static_cast<std::uint32_t>(i / mapWidth);

			const float worldX = static_cast<float>(x * tileWidth);
			const float worldY = static_cast<float>(y * tileHeight);

			const float texX = static_cast<float>(tu * tileWidth);
			const float texY = static_cast<float>(tv * tileHeight);

			appendTileQuad(
				layer.vertices,
				worldX, worldY,
				static_cast<float>(tileWidth), static_cast<float>(tileHeight),
				texX, texY,
				static_cast<float>(tileWidth), static_cast<float>(tileHeight));
		}

		m_layers.push_back(std::move(layer));
	}

	for (tinyxml2::XMLElement* groupElem = mapElem->FirstChildElement("objectgroup");
		groupElem != nullptr;
		groupElem = groupElem->NextSiblingElement("objectgroup"))
	{
		const char* groupName = groupElem->Attribute("name");
		if (!groupName)
			continue;

		const std::string name = groupName;
		if (name != "Collisions" && name != "Collision")
			continue;

		for (tinyxml2::XMLElement* objectElem = groupElem->FirstChildElement("object");
			objectElem != nullptr;
			objectElem = objectElem->NextSiblingElement("object"))
		{
			const float x = objectElem->FloatAttribute("x");
			const float y = objectElem->FloatAttribute("y");
			const float w = objectElem->FloatAttribute("width");
			const float h = objectElem->FloatAttribute("height");

			if (w <= 0.f || h <= 0.f)
				continue;

			m_colliders.emplace_back(sf::Vector2f{ x, y }, sf::Vector2f{ w, h });
		}
	}

	return true;
}

void World::drawBack(sf::RenderTarget& target) const
{
	sf::RenderStates states;
	states.texture = &m_tilesetTexture;

	for (const auto& layer : m_layers)
	{
		if (layer.name == "front")
			continue;

		target.draw(layer.vertices, states);
	}
}

void World::drawFront(sf::RenderTarget& target) const
{
	sf::RenderStates states;
	states.texture = &m_tilesetTexture;

	for (const auto& layer : m_layers)
	{
		if (layer.name != "front")
			continue;

		target.draw(layer.vertices, states);
	}
}

const std::vector<sf::FloatRect>& World::getColliders() const
{
	return m_colliders;
}

sf::Vector2u World::getPixelSize() const
{
	return m_pixelSize;
}