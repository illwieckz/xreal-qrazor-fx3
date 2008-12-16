#ifndef SOUNDFILE_H_
#define SOUNDFILE_H_

#include "isound.h"

#include <boost/shared_ptr.hpp>

namespace sound
{

/**
 * Representation of a single sound file.
 */
class SoundFile
: public ISoundFile
{
	// Name of the shader
	std::string _name;
public:

	/**
	 * Constructor.
	 */
	SoundFile(const std::string& name)
	: _name(name)
	{
	}

	/**
	 * Return the name of the file.
	 */
	std::string getName() const {
		return _name;
	}
};

/**
 * Shared pointer type.
 */
typedef boost::shared_ptr<SoundFile> SoundFilePtr;

}

#endif /*SOUNDSHADER_H_*/
