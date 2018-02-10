#include "FileSorts.h"

#include "utils/StringUtil.h"

#define gettext_noop(A) A

namespace FileSorts
{
	const FileData::SortType typesArr[] = {
		FileData::SortType(&compareName, true, gettext_noop("FILENAME, ASCENDING")),
		FileData::SortType(&compareName, false, gettext_noop("FILENAME, DESCENDING")),

		FileData::SortType(&compareRating, true, gettext_noop("RATING, ASCENDING")),
		FileData::SortType(&compareRating, false, gettext_noop("RATING, DESCENDING")),

		FileData::SortType(&compareTimesPlayed, true, gettext_noop("TIMES PLAYED, ASCENDING")),
		FileData::SortType(&compareTimesPlayed, false, gettext_noop("TIMES PLAYED, DESCENDING")),

		FileData::SortType(&compareLastPlayed, true, gettext_noop("LAST PLAYED, ASCENDING")),
		FileData::SortType(&compareLastPlayed, false, gettext_noop("LAST PLAYED, DESCENDING")),

		FileData::SortType(&compareNumPlayers, true, gettext_noop("NUMBER PLAYERS, ASCENDING")),
		FileData::SortType(&compareNumPlayers, false, gettext_noop("NUMBER PLAYERS, DESCENDING")),

		FileData::SortType(&compareReleaseDate, true, gettext_noop("RELEASE DATE, ASCENDING")),
		FileData::SortType(&compareReleaseDate, false, gettext_noop("RELEASE DATE, DESCENDING")),

		FileData::SortType(&compareGenre, true, gettext_noop("GENRE, ASCENDING")),
		FileData::SortType(&compareGenre, false, gettext_noop("GENRE, DESCENDING")),

		FileData::SortType(&compareDeveloper, true, gettext_noop("DEVELOPER, ASCENDING")),
		FileData::SortType(&compareDeveloper, false, gettext_noop("DEVELOPER, DESCENDING")),

		FileData::SortType(&comparePublisher, true, gettext_noop("PUBLISHER, ASCENDING")),
		FileData::SortType(&comparePublisher, false, gettext_noop("PUBLISHER, DESCENDING")),

		FileData::SortType(&compareSystem, true, gettext_noop("SYSTEM, ASCENDING")),
		FileData::SortType(&compareSystem, false, gettext_noop("SYSTEM, DESCENDING"))
	};

	const std::vector<FileData::SortType> SortTypes(typesArr, typesArr + sizeof(typesArr)/sizeof(typesArr[0]));

	//returns if file1 should come before file2
	bool compareName(const FileData* file1, const FileData* file2)
	{
		// we compare the actual metadata name, as collection files have the system appended which messes up the order
		std::string name1 = Utils::String::toUpper(file1->metadata.get("name"));
		std::string name2 = Utils::String::toUpper(file2->metadata.get("name"));
		return name1.compare(name2) < 0;
	}

	bool compareRating(const FileData* file1, const FileData* file2)
	{
		return file1->metadata.getFloat("rating") < file2->metadata.getFloat("rating");
	}

	bool compareTimesPlayed(const FileData* file1, const FileData* file2)
	{
		//only games have playcount metadata
		if(file1->metadata.getType() == GAME_METADATA && file2->metadata.getType() == GAME_METADATA)
		{
			return (file1)->metadata.getInt("playcount") < (file2)->metadata.getInt("playcount");
		}

		return false;
	}

	bool compareLastPlayed(const FileData* file1, const FileData* file2)
	{
		// since it's stored as an ISO string (YYYYMMDDTHHMMSS), we can compare as a string
		// as it's a lot faster than the time casts and then time comparisons
		return (file1)->metadata.get("lastplayed") < (file2)->metadata.get("lastplayed");
	}

	bool compareNumPlayers(const FileData* file1, const FileData* file2)
	{
		return (file1)->metadata.getInt("players") < (file2)->metadata.getInt("players");
	}

	bool compareReleaseDate(const FileData* file1, const FileData* file2)
	{
		// since it's stored as an ISO string (YYYYMMDDTHHMMSS), we can compare as a string
		// as it's a lot faster than the time casts and then time comparisons
		return (file1)->metadata.get("releasedate") < (file2)->metadata.get("releasedate");
	}

	bool compareGenre(const FileData* file1, const FileData* file2)
	{
		std::string genre1 = Utils::String::toUpper(file1->metadata.get("genre"));
		std::string genre2 = Utils::String::toUpper(file2->metadata.get("genre"));
		return genre1.compare(genre2) < 0;
	}

	bool compareDeveloper(const FileData* file1, const FileData* file2)
	{
		std::string developer1 = Utils::String::toUpper(file1->metadata.get("developer"));
		std::string developer2 = Utils::String::toUpper(file2->metadata.get("developer"));
		return developer1.compare(developer2) < 0;
	}

	bool comparePublisher(const FileData* file1, const FileData* file2)
	{
		std::string publisher1 = Utils::String::toUpper(file1->metadata.get("publisher"));
		std::string publisher2 = Utils::String::toUpper(file2->metadata.get("publisher"));
		return publisher1.compare(publisher2) < 0;
	}

	bool compareSystem(const FileData* file1, const FileData* file2)
	{
		std::string system1 = Utils::String::toUpper(file1->getSystemName());
		std::string system2 = Utils::String::toUpper(file2->getSystemName());
		return system1.compare(system2) < 0;
	}
};
