#include "model/song.h"

#include <iomanip>
#include <string>

namespace model {

std::string to_string(const Song& arg) {
  //   // TODO: improve this
  //   std::string bit_rate;
  //   if (arg.bit_rate > 1000) {
  //     bit_rate = std::to_string(arg.bit_rate / 1000) + " kbps";
  //   } else {
  //     bit_rate = std::to_string(arg.bit_rate) + " bps";
  //   }
  bool is_empty = arg.filepath.empty() ? true : false;

  //   std::string artist = arg.artist.empty() ? "<Unknown>" : arg.artist;

  std::string artist = is_empty ? "<Empty>" : arg.artist.empty() ? "<Unknown>" : arg.artist;
  std::string title = is_empty ? "<Empty>" : arg.title.empty() ? "<Unknown>" : arg.title;

  std::string channels = is_empty ? "<Empty>" : std::to_string(arg.num_channels);
  std::string sample_rate = is_empty ? "<Empty>" : std::to_string(arg.sample_rate);
  std::string bit_rate = is_empty ? "<Empty>" : std::to_string(arg.bit_rate);
  std::string bit_depth = is_empty ? "<Empty>" : std::to_string(arg.bit_depth);
  std::string duration = is_empty ? "<Empty>" : std::to_string(arg.duration);

  std::ostringstream ss;

  //   oss << "Filepath: " << arg.filepath << std::endl;
  ss << "Artist: " << artist << std::endl;
  ss << "Title: " << title << std::endl;
  ss << "Channels: " << channels << std::endl;
  ss << "Sample rate: " << sample_rate << std::endl;
  ss << "Bit rate: " << bit_rate << std::endl;
  ss << "Bits per sample: " << bit_depth << std::endl;
  ss << "Duration (seconds): " << duration << std::endl;

  return std::move(ss).str();
}

/* ********************************************************************************************** */

std::string time_to_string(const uint32_t& arg) {
  const int hours = arg / 3600;
  const int minutes = (arg - (hours * 3600)) / 60;
  const int seconds = arg - (hours * 3600) - (minutes * 60);

  std::ostringstream ss;

  if (hours > 0) ss << std::setw(2) << std::setfill('0') << hours << ":";
  ss << std::setw(2) << std::setfill('0') << minutes << ":";
  ss << std::setw(2) << std::setfill('0') << seconds;

  return std::move(ss).str();
}

}  // namespace model