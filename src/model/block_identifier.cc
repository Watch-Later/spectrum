#include "model/block_identifier.h"

namespace model {

//! BlockIdentifier pretty print
std::ostream& operator<<(std::ostream& out, const BlockIdentifier& id) {
  switch (id) {
    case BlockIdentifier::ListDirectory:
      out << "ListDirectory";
      break;
    case BlockIdentifier::FileInfo:
      out << "FileInfo";
      break;
    case BlockIdentifier::TabViewer:
      out << "TabViewer";
      break;
    case BlockIdentifier::MediaPlayer:
      out << "MediaPlayer";
      break;
  }

  return out;
}

}  // namespace model