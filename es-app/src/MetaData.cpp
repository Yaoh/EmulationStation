#include "MetaData.h"

#include "utils/FileSystemUtil.h"
#include "Log.h"
#include <pugixml/src/pugixml.hpp>
#include "Locale.h"

MetaDataDecl gameDecls[] = {
  // key,         type,                   default,            statistic,  name in GuiMetaDataEd,      prompt in GuiMetaDataEd
  {"name",        MD_STRING,              "",                 false,      N_("NAME"),                 N_("ENTER GAME NAME")},
  {"sortname",    MD_STRING,              "",                 false,      N_("SORTNAME"),             N_("ENTER GAME SORT NAME")},
  {"desc",        MD_MULTILINE_STRING,    "",                 false,      N_("DESCRIPTION"),          N_("ENTER DESCRIPTION")},
  {"image",       MD_PATH,                "",                 false,      N_("IMAGE"),                N_("ENTER PATH TO IMAGE")},
  {"video",       MD_PATH     ,           "",                 false,      N_("VIDEO"),                N_("ENTER PATH TO VIDEO")},
  {"marquee",     MD_PATH,                "",                 false,      N_("MARQUEE"),              N_("ENTER PATH TO MARQUEE")},
  {"thumbnail",   MD_PATH,                "",                 false,      N_("THUMBNAIL"),            N_("ENTER PATH TO THUMBNAIL")},
  {"rating",      MD_RATING,              "0.000000",         false,      N_("RATING"),               N_("ENTER RATING")},
  {"releasedate", MD_DATE,                "not-a-date-time",  false,      N_("RELEASE DATE"),         N_("ENTER RELEASE DATE")},
  {"developer",   MD_STRING,              N_("unknown"),      false,      N_("DEVELOPER"),            N_("ENTER GAME DEVELOPER")},
  {"publisher",   MD_STRING,              N_("unknown"),      false,      N_("PUBLISHER"),            N_("ENTER GAME PUBLISHER")},
  {"genre",       MD_STRING,              N_("unknown"),      false,      N_("GENRE"),                N_("ENTER GAME GENRE")},
  {"players",     MD_INT,                 "1",                false,      N_("PLAYERS"),              N_("ENTER NUMBER OF PLAYERS")},
  {"favorite",    MD_BOOL,                "false",            false,      N_("FAVORITE"),             N_("ENTER FAVORITE OFF/ON")},
  {"hidden",      MD_BOOL,                "false",            false,      N_("HIDDEN"),               N_("ENTER HIDDEN OFF/ON" )},
  {"kidgame",     MD_BOOL,                "false",            false,      N_("KIDGAME"),              N_("ENTER KIDGAME OFF/ON" )},
  {"playcount",   MD_INT,                 "0",                true,       N_("PLAY COUNT"),           N_("ENTER NUMBER OF TIMES PLAYED")},
  {"lastplayed",  MD_TIME,                "0",                true,       N_("LAST PLAYED"),          N_("ENTER LAST PLAYED DATE")}
};
const std::vector<MetaDataDecl> gameMDD(gameDecls, gameDecls + sizeof(gameDecls) / sizeof(gameDecls[0]));

MetaDataDecl folderDecls[] = {
  {"name",        MD_STRING,              "",                 false,      N_("NAME"),                 N_("ENTER GAME NAME")},
  {"sortname",    MD_STRING,              "",                 false,      N_("SORTNAME"),             N_("ENTER GAME SORT NAME")},
  {"desc",        MD_MULTILINE_STRING,    "",                 false,      N_("DESCRIPTION"),          N_("ENTER DESCRIPTION")},
  {"image",       MD_PATH,                "",                 false,      N_("IMAGE"),                N_("ENTER PATH TO IMAGE")},
  {"thumbnail",   MD_PATH,                "",                 false,      N_("THUMBNAIL"),            N_("ENTER PATH TO THUMBNAIL")},
  {"video",       MD_PATH,                "",                 false,      N_("VIDEO"),                N_("ENTER PATH TO VIDEO")},
  {"marquee",     MD_PATH,                "",                 false,      N_("MARQUEE"),              N_("ENTER PATH TO MARQUEE")},
  {"rating",      MD_RATING,              "0.000000",         false,      N_("RATING"),               N_("ENTER RATING")},
  {"releasedate", MD_DATE,                "not-a-date-time",  false,      N_("RELEASE DATE"),         N_("ENTER RELEASE DATE")},
  {"developer",   MD_STRING,              N_("unknown"),      false,      N_("DEVELOPER"),            N_("ENTER GAME DEVELOPER")},
  {"publisher",   MD_STRING,              N_("unknown"),      false,      N_("PUBLISHER"),            N_("ENTER GAME PUBLISHER")},
  {"genre",       MD_STRING,              N_("unknown"),      false,      N_("GENRE"),                N_("ENTER GAME GENRE")},
  {"players",     MD_INT,                 "1",                false,      N_("PLAYERS"),              N_("ENTER NUMBER OF PLAYERS")}
};
const std::vector<MetaDataDecl> folderMDD(folderDecls, folderDecls + sizeof(folderDecls) / sizeof(folderDecls[0]));

const std::vector<MetaDataDecl>& getMDDByType(MetaDataListType type)
{
	switch(type)
	{
	case GAME_METADATA:
		return gameMDD;
	case FOLDER_METADATA:
		return folderMDD;
	}

	LOG(LogError) << "Invalid MDD type";
	return gameMDD;
}



MetaDataList::MetaDataList(MetaDataListType type)
	: mType(type), mWasChanged(false)
{
	const std::vector<MetaDataDecl>& mdd = getMDD();
	for(auto iter = mdd.cbegin(); iter != mdd.cend(); iter++)
		set(iter->key, iter->defaultValue);
}


MetaDataList MetaDataList::createFromXML(MetaDataListType type, pugi::xml_node& node, const std::string& relativeTo)
{
	MetaDataList mdl(type);

	const std::vector<MetaDataDecl>& mdd = mdl.getMDD();

	for(auto iter = mdd.cbegin(); iter != mdd.cend(); iter++)
	{
		pugi::xml_node md = node.child(iter->key.c_str());
		if(md)
		{
			// if it's a path, resolve relative paths
			std::string value = md.text().get();
			if (iter->type == MD_PATH)
			{
				value = Utils::FileSystem::resolveRelativePath(value, relativeTo, true);
			}
			mdl.set(iter->key, value);
		}else{
			mdl.set(iter->key, iter->defaultValue);
		}
	}

	return mdl;
}

void MetaDataList::appendToXML(pugi::xml_node& parent, bool ignoreDefaults, const std::string& relativeTo) const
{
	const std::vector<MetaDataDecl>& mdd = getMDD();

	for(auto mddIter = mdd.cbegin(); mddIter != mdd.cend(); mddIter++)
	{
		auto mapIter = mMap.find(mddIter->key);
		if(mapIter != mMap.cend())
		{
			// we have this value!
			// if it's just the default (and we ignore defaults), don't write it
			if(ignoreDefaults && mapIter->second == mddIter->defaultValue)
				continue;
			
			// try and make paths relative if we can
			std::string value = mapIter->second;
			if (mddIter->type == MD_PATH)
				value = Utils::FileSystem::createRelativePath(value, relativeTo, true);

			parent.append_child(mapIter->first.c_str()).text().set(value.c_str());
		}
	}
}

void MetaDataList::set(const std::string& key, const std::string& value)
{
	mMap[key] = value;
	mWasChanged = true;
}

const std::string& MetaDataList::get(const std::string& key) const
{
	return mMap.at(key);
}

int MetaDataList::getInt(const std::string& key) const
{
	return atoi(get(key).c_str());
}

float MetaDataList::getFloat(const std::string& key) const
{
	return (float)atof(get(key).c_str());
}

bool MetaDataList::isDefault()
{
	const std::vector<MetaDataDecl>& mdd = getMDD();

	for (unsigned int i = 1; i < mMap.size(); i++) {
		if (mMap.at(mdd[i].key) != mdd[i].defaultValue) return false;
	}

	return true;
}

bool MetaDataList::wasChanged() const
{
	return mWasChanged;
}

void MetaDataList::resetChangedFlag()
{
	mWasChanged = false;
}
