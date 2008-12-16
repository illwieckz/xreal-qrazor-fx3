#ifndef SOUNDFILELOADER_H_
#define SOUNDFILELOADER_H_

#include "SoundManager.h"

#include "ifilesystem.h"
#include "iarchive.h"

#include <iostream>
#include <boost/algorithm/string/predicate.hpp>

namespace sound
{

/**
 * Sound directory name.
 */
const char* SOUND_FOLDER = "sound/";
const char* WAV_EXTENSION = ".wav";
const char* OGG_EXTENSION = ".ogg";


/**
 * Loader class passed to the GlobalFileSystem to load sound files
 */
class SoundFileLoader
{
	// SoundManager to populate
	SoundManager& _manager;

public:

	// Required type
	typedef const std::string& first_argument_type;

	/**
	 * Constructor. Set the sound manager reference.
	 */
	SoundFileLoader(SoundManager& manager)
	: _manager(manager)
	{ }

	/**
	 * Functor operator.
	 */
	void operator() (const std::string& fileName) {

		/*
		// Open the .sndshd file and get its contents as a std::string
		ArchiveTextFilePtr file =
			GlobalFileSystem().openTextFile(SOUND_FOLDER + fileName);

		// Parse contents of file if it was opened successfully
		if (file) {
			std::istream is(&(file->getInputStream()));

			try {
				// Pass the contents back to the SoundModule for parsing
				_manager.parseShadersFrom(is);
			}
			catch (parser::ParseException ex) {
				globalErrorStream() << "[sound]: Error while parsing " << fileName <<
					": " << ex.what() << std::endl;
			}
		}
		else {
			std::cerr << "[sound] Warning: unable to open \""
					  << fileName << "\"" << std::endl;
		}
		*/

		// Test the extension. If it is not matching any of the known extensions,
		// not interested
		if (boost::algorithm::iends_with(fileName, WAV_EXTENSION) ||
			boost::algorithm::iends_with(fileName, OGG_EXTENSION))
		{
			_manager.addSoundFile(fileName);
		}
	}
};

}

#endif /*SOUNDFILELOADER_H_*/
