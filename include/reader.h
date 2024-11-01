/**
 * reader.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <filesystem>

namespace cfg
{
  namespace reader
  {
    class unknown_format : public std::runtime_error
    {
    public:
      unknown_format(const std::string &msg) : std::runtime_error{msg} {};
    };

    class reader
    {
    public:
      explicit reader(const std::string &meshfile) : meshfile(meshfile)
      {
	// Confirm existence of meshfile, allowing it to throw on failure.
	std::error_code ec;
	if(!std::filesystem::exists(this->meshfile, ec))
	{
	  throw std::filesystem::filesystem_error{"Could not find mesh file " + meshfile, ec};
	}

	// Attempt to determine the format of the file
	if (std::filesystem::is_regular_file(this->meshfile))
	{
	  throw cfg::reader::unknown_format{this->meshfile.string() +
					    " treating all meshes an unrecognised format"};
	}
	else
	{
	  throw cfg::reader::unknown_format{this->meshfile.string() + " is an unrecognised format"};
	}
      }
    private:
      std::filesystem::path meshfile;
    };
  }  // namespace reader
}  // namespace cfg
  
