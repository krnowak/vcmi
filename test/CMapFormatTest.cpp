
/*
 * CMapFormatTest.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"

#include <boost/test/unit_test.hpp>

#include "../lib/filesystem/CMemoryBuffer.h"
#include "../lib/filesystem/Filesystem.h"

#include "../lib/mapping/CMap.h"
#include "../lib/rmg/CMapGenOptions.h"
#include "../lib/rmg/CMapGenerator.h"
#include "../lib/mapping/MapFormatJson.h"

#include "../lib/VCMIDirs.h"

#include "MapComparer.h"

static const int TEST_RANDOM_SEED = 1337;

BOOST_AUTO_TEST_CASE(CMapFormatVCMI_RandomMap)
{
	logGlobal->info("CMapFormatVCMI_RandomMap start");
	BOOST_TEST_CHECKPOINT("CMapFormatVCMI_RandomMap start");
	std::unique_ptr<CMap> initialMap;

	CMapGenOptions opt;

	opt.setHeight(CMapHeader::MAP_SIZE_MIDDLE);
	opt.setWidth(CMapHeader::MAP_SIZE_MIDDLE);
	opt.setHasTwoLevels(true);
	opt.setPlayerCount(4);

	opt.setPlayerTypeForStandardPlayer(PlayerColor(0), EPlayerType::HUMAN);
	opt.setPlayerTypeForStandardPlayer(PlayerColor(1), EPlayerType::AI);
	opt.setPlayerTypeForStandardPlayer(PlayerColor(2), EPlayerType::AI);
	opt.setPlayerTypeForStandardPlayer(PlayerColor(3), EPlayerType::AI);

	CMapGenerator gen;

	initialMap = gen.generate(&opt, TEST_RANDOM_SEED);
	initialMap->name = "Test";
	BOOST_TEST_CHECKPOINT("CMapFormatVCMI_RandomMap generated");

	CMemoryBuffer serializeBuffer;
	{
		CMapSaverJson saver(&serializeBuffer);
		saver.saveMap(initialMap);
	}
	BOOST_TEST_CHECKPOINT("CMapFormatVCMI_RandomMap serialized");
	#if 1
	{
		auto path = VCMIDirs::get().userDataPath()/"test_random.vmap";
		boost::filesystem::remove(path);
		boost::filesystem::ofstream tmp(path, boost::filesystem::ofstream::binary);

		tmp.write((const char *)serializeBuffer.getBuffer().data(),serializeBuffer.getSize());
		tmp.flush();
		tmp.close();

		logGlobal->infoStream() << "Test map has been saved to " << path;
	}
	BOOST_TEST_CHECKPOINT("CMapFormatVCMI_RandomMap saved");

	#endif // 1

	serializeBuffer.seek(0);
	{
		CMapLoaderJson loader(&serializeBuffer);
		std::unique_ptr<CMap> serialized = loader.loadMap();

		MapComparer c;
		c(serialized, initialMap);
	}

	logGlobal->info("CMapFormatVCMI_RandomMap finish");
}

static JsonNode getFromArchive(CZipLoader & archive, const std::string & archiveFilename)
{
	ResourceID resource(archiveFilename, EResType::TEXT);

	if(!archive.existsResource(resource))
		throw new std::runtime_error(archiveFilename+" not found");

	auto data = archive.load(resource)->readAll();

	JsonNode res(reinterpret_cast<char*>(data.first.get()), data.second);

	return std::move(res);
}


static void addToArchive(CZipSaver & saver, const JsonNode & data, const std::string & filename)
{
	std::ostringstream out;
	out << data;
	out.flush();

	{
		auto s = out.str();
		std::unique_ptr<COutputStream> stream = saver.addFile(filename);

		if(stream->write((const ui8*)s.c_str(), s.size()) != s.size())
			throw new std::runtime_error("CMapSaverJson::saveHeader() zip compression failed.");
	}
}

BOOST_AUTO_TEST_CASE(CMapFormatVCMI_ObjectPropertyTestMap)
{
	logGlobal->info("CMapFormatVCMI_ObjectPropertyTestMap start");

	static const std::string MAP_DATA_PATH = "test/ObjectPropertyTest/";

	const JsonNode expectedHeader(ResourceID(MAP_DATA_PATH+"header.json"));
	const JsonNode expectedObjects(ResourceID(MAP_DATA_PATH+"objects.json"));
	const JsonNode expectedSurface(ResourceID(MAP_DATA_PATH+"surface_terrain.json"));
	const JsonNode expectedUnderground(ResourceID(MAP_DATA_PATH+"underground_terrain.json"));

	std::unique_ptr<CMap> originalMap;
	{
		CMemoryBuffer initialBuffer;

		std::shared_ptr<CIOApi> originalDataIO(new CProxyIOApi(&initialBuffer));

		{
			CZipSaver initialSaver(originalDataIO, "_");

			addToArchive(initialSaver, expectedHeader, "header.json");
			addToArchive(initialSaver, expectedObjects, "objects.json");
			addToArchive(initialSaver, expectedSurface, "surface_terrain.json");
			addToArchive(initialSaver, expectedUnderground, "underground_terrain.json");
		}

		initialBuffer.seek(0);

		{
			CMapLoaderJson initialLoader(&initialBuffer);

			originalMap = initialLoader.loadMap();
		}
	}

	CMemoryBuffer serializeBuffer;
	{
		CMapSaverJson saver(&serializeBuffer);
		saver.saveMap(originalMap);
	}

	std::shared_ptr<CIOApi> actualDataIO(new CProxyROIOApi(&serializeBuffer));
	CZipLoader actualDataLoader("", "_", actualDataIO);

	const JsonNode actualHeader = getFromArchive(actualDataLoader, "header.json");
	const JsonNode actualObjects = getFromArchive(actualDataLoader, "objects.json");
	const JsonNode actualSurface = getFromArchive(actualDataLoader, "surface_terrain.json");

	{
		auto path = VCMIDirs::get().userDataPath()/"test_object_property.vmap";
		boost::filesystem::remove(path);
		boost::filesystem::ofstream tmp(path, boost::filesystem::ofstream::binary);

		tmp.write((const char *)serializeBuffer.getBuffer().data(),serializeBuffer.getSize());
		tmp.flush();
		tmp.close();

		logGlobal->infoStream() << "Test map has been saved to " << path;
	}

    for(const auto & p : expectedHeader.Struct())
	{
		if(!vstd::contains(actualHeader.Struct(), p.first))
			BOOST_ERROR("Header field missing "+p.first);
		else if(p.second != actualHeader[p.first])
			BOOST_ERROR("Header field differ "+p.first);
	}

    for(const auto & p : expectedObjects.Struct())
	{
		if(!vstd::contains(actualObjects.Struct(), p.first))
			BOOST_ERROR("Map object missing "+p.first);

		else if(p.second != actualObjects[p.first])
			BOOST_ERROR("Map object differ "+p.first);
	}

	logGlobal->info("CMapFormatVCMI_ObjectPropertyTestMap finish");
}
