/**
 * \file
 * \brief  Base class for a song
 */

#ifndef INCLUDE_MODEL_SONG_H_
#define INCLUDE_MODEL_SONG_H_

#include <cstdio>
#include <fstream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "error_table.h"

//! Song data information
struct AudioData {
  std::string_view file_format;  //!< Audio file format (currently supports only "WAV")
  uint16_t num_channels;         //!< Number of channels (1=Mono 2=Stereo)
  uint32_t sample_rate;  //!< Number of samples (of signal amplitude or “sound”) per second
  uint32_t bit_rate;     //!< Bits process per second
  uint32_t bit_depth;    //!< Number of bits per sample
};

/**
 * @brief Interface class for a Song
 */
class Song {
 public:
  /**
   * @brief Construct a new Song object
   */
  Song() = default;

  /**
   * @brief Destroy the Song object
   */
  virtual ~Song() = default;

  // Remove these constructors/operators
  Song(const Song& other) = delete;             // copy constructor
  Song(Song&& other) = delete;                  // move constructor
  Song& operator=(const Song& other) = delete;  // copy assignment
  Song& operator=(Song&& other) = delete;       // move assignment

  /* ******************************************************************************************** */

  /**
   * @brief Parse only the header metadata from a given sound file
   *
   * @param full_path Path where song is located
   * @return Value Error code from operation
   */
  virtual error::Value ParseHeaderInfo(const std::string& full_path) = 0;

  /**
   * @brief Parse raw data from a given sound file (this is only possible after parsing header info)
   * @return Value Error code from operation
   */
  virtual error::Value ParseData() = 0;

  /* ******************************************************************************************** */

  /**
   * @brief Get the Audio Information object
   * @return AudioData Audio information
   */
  AudioData GetAudioInformation() { return info_; }

  /* ******************************************************************************************** */
 protected:
  std::string filename_;  //!< Path to sound file
  std::ifstream file_;    //!< File-based streambuffer pointing to sound file
  AudioData info_;        //!< Sound data information
};

/* ------------------------------------ Overloaded Operators ------------------------------------ */

std::ostream& operator<<(std::ostream& os, const AudioData& arg);
std::string to_string(const AudioData& arg);

#endif  // INCLUDE_MODEL_SONG_H_